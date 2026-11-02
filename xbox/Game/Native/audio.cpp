// Keep Win32's unrelated CloseWindow declaration separate from the raylib ABI.
#if defined(_WIN32)
#define CloseWindow Win32CloseWindow
#include <xaudio2.h>
#undef CloseWindow
#endif
#include "raylib.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace
{
    std::mutex audioMutex;
    struct Sample
    {
        std::vector<uint8_t> pcm;
#if defined(_WIN32)
        IXAudio2SourceVoice *voice = nullptr;
        ~Sample() { if (voice) voice->DestroyVoice(); }
#endif
    };
    std::unordered_map<rAudioBuffer *, std::unique_ptr<Sample>> sounds;
#if defined(_WIN32)
    IXAudio2 *engine = nullptr;
    IXAudio2MasteringVoice *master = nullptr;
#endif
    uint16_t u16(const uint8_t *p) { return p[0] | (uint16_t(p[1]) << 8); }
    uint32_t u32(const uint8_t *p) { return u16(p) | (uint32_t(u16(p + 2)) << 16); }
}

void InitAudioDevice()
{
    std::lock_guard lock(audioMutex);
#if defined(_WIN32)
    if (engine) return;
    if (FAILED(XAudio2Create(&engine))) { engine = nullptr; return; }
    if (FAILED(engine->CreateMasteringVoice(&master)))
    {
        engine->Release(); engine = nullptr; master = nullptr;
    }
#endif
}
bool IsAudioDeviceReady()
{
    std::lock_guard lock(audioMutex);
#if defined(_WIN32)
    return engine && master;
#else
    return false;
#endif
}
void CloseAudioDevice()
{
    std::lock_guard lock(audioMutex);
    sounds.clear();
#if defined(_WIN32)
    if (master) master->DestroyVoice();
    if (engine) engine->Release();
    master = nullptr; engine = nullptr;
#endif
}
Wave LoadWaveFromMemory(const char *type, const unsigned char *data, int size)
{
    Wave wave{};
    if (!type || std::strcmp(type, ".wav") || !data || size < 12 ||
        std::memcmp(data, "RIFF", 4) || std::memcmp(data + 8, "WAVE", 4)) return wave;
    const uint8_t *pcm = nullptr;
    uint32_t bytes = 0;
    bool validFormat = false;
    for (size_t offset = 12; offset + 8 <= static_cast<size_t>(size);)
    {
        const uint32_t length = u32(data + offset + 4);
        const size_t start = offset + 8;
        if (length > static_cast<size_t>(size) - start) return {};
        if (!std::memcmp(data + offset, "fmt ", 4))
        {
            if (length < 16 || u16(data + start) != 1) return {};
            wave.channels = u16(data + start + 2);
            wave.sampleRate = u32(data + start + 4);
            wave.sampleSize = u16(data + start + 14);
            validFormat = (wave.channels == 1 || wave.channels == 2) && wave.sampleSize == 16 &&
                          wave.sampleRate >= 1000 && wave.sampleRate <= 192000 &&
                          u16(data + start + 12) == wave.channels * 2;
        }
        if (!std::memcmp(data + offset, "data", 4)) { pcm = data + start; bytes = length; }
        offset = start + length + (length & 1u);
    }
    if (!validFormat || !pcm || !bytes || bytes % (wave.channels * 2)) return {};
    wave.data = std::malloc(bytes);
    if (!wave.data) return {};
    std::memcpy(wave.data, pcm, bytes);
    wave.frameCount = bytes / (wave.channels * 2);
    return wave;
}
void UnloadWave(Wave wave) { std::free(wave.data); }
Sound LoadSoundFromWave(Wave wave)
{
    Sound sound{};
    if (!wave.data || !wave.frameCount || wave.sampleSize != 16 ||
        (wave.channels != 1 && wave.channels != 2) || wave.frameCount > UINT32_MAX / (wave.channels * 2)) return sound;
    std::lock_guard lock(audioMutex);
#if defined(_WIN32)
    if (!engine) return sound;
    auto sample = std::make_unique<Sample>();
    WAVEFORMATEX format{};
    format.wFormatTag = WAVE_FORMAT_PCM; format.nChannels = static_cast<WORD>(wave.channels);
    format.nSamplesPerSec = wave.sampleRate; format.wBitsPerSample = 16;
    format.nBlockAlign = static_cast<WORD>(wave.channels * 2);
    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
    if (FAILED(engine->CreateSourceVoice(&sample->voice, &format, 0, 4.0f))) return sound;
    const auto *data = static_cast<const uint8_t *>(wave.data);
    sample->pcm.assign(data, data + static_cast<size_t>(wave.frameCount) * format.nBlockAlign);
    sound.stream.buffer = reinterpret_cast<rAudioBuffer *>(sample.get());
    sound.stream.sampleRate = wave.sampleRate; sound.stream.sampleSize = 16;
    sound.stream.channels = wave.channels; sound.frameCount = wave.frameCount;
    sounds.emplace(sound.stream.buffer, std::move(sample));
#endif
    return sound;
}
void UnloadSound(Sound sound)
{
    std::lock_guard lock(audioMutex);
    sounds.erase(sound.stream.buffer);
}
void PlaySound(Sound sound)
{
    std::lock_guard lock(audioMutex);
#if defined(_WIN32)
    auto it = sounds.find(sound.stream.buffer);
    if (it == sounds.end()) return;
    auto &sample = *it->second;
    sample.voice->Stop(); sample.voice->FlushSourceBuffers();
    XAUDIO2_BUFFER buffer{};
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.AudioBytes = static_cast<UINT32>(sample.pcm.size()); buffer.pAudioData = sample.pcm.data();
    if (SUCCEEDED(sample.voice->SubmitSourceBuffer(&buffer))) sample.voice->Start();
#endif
}
void StopSound(Sound sound)
{
    std::lock_guard lock(audioMutex);
#if defined(_WIN32)
    auto it = sounds.find(sound.stream.buffer);
    if (it != sounds.end()) { it->second->voice->Stop(); it->second->voice->FlushSourceBuffers(); }
#endif
}
bool IsSoundPlaying(Sound sound)
{
    std::lock_guard lock(audioMutex);
#if defined(_WIN32)
    auto it = sounds.find(sound.stream.buffer);
    if (it != sounds.end()) { XAUDIO2_VOICE_STATE state{}; it->second->voice->GetState(&state); return state.BuffersQueued != 0; }
#endif
    return false;
}
void SetSoundVolume(Sound sound, float volume)
{
    std::lock_guard lock(audioMutex);
#if defined(_WIN32)
    auto it = sounds.find(sound.stream.buffer);
    if (it != sounds.end() && std::isfinite(volume)) it->second->voice->SetVolume(std::clamp(volume, 0.0f, 1.0f));
#endif
}
void SetSoundPitch(Sound sound, float pitch)
{
    std::lock_guard lock(audioMutex);
#if defined(_WIN32)
    auto it = sounds.find(sound.stream.buffer);
    if (it != sounds.end() && std::isfinite(pitch)) it->second->voice->SetFrequencyRatio(std::clamp(pitch, 0.01f, 4.0f));
#endif
}
