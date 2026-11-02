#include "host.h"
#include "raylib.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <thread>
#include <vector>

namespace
{
    constexpr uint32_t maxWidth = 640, maxHeight = 512;
    std::mutex frameMutex, inputMutex;
    std::vector<uint8_t> texture, frame;
    uint32_t textureWidth = 0, textureHeight = 0, frameWidth = 0, frameHeight = 0;
    uint64_t sequence = 0;
    uint32_t buttons = 0;
    std::array<float, 4> axes{};
    std::atomic<bool> closed{true};
    std::chrono::steady_clock::time_point lastPresent;

    float axis(float value)
    {
        return std::isfinite(value) ? std::clamp(value, -1.0f, 1.0f) : 0.0f;
    }
}

void ps2x::host::reset()
{
    std::scoped_lock lock(frameMutex, inputMutex);
    frame.clear(); texture.clear(); sequence = 0;
    frameWidth = frameHeight = textureWidth = textureHeight = buttons = 0;
    axes.fill(0.0f);
    lastPresent = std::chrono::steady_clock::now();
    closed.store(false);
}
void ps2x::host::close() { closed.store(true); }

uint64_t ps2x_copy_frame(uint8_t *pixels, uint32_t capacity, uint32_t *width, uint32_t *height)
{
    if (!width || !height) return 0;
    std::lock_guard lock(frameMutex);
    *width = frameWidth; *height = frameHeight;
    if (!pixels || frame.empty() || capacity < frame.size()) return 0;
    std::memcpy(pixels, frame.data(), frame.size());
    return sequence;
}
void ps2x_set_input(uint32_t value, float lx, float ly, float rx, float ry)
{
    std::lock_guard lock(inputMutex);
    buttons = value;
    axes = {axis(lx), axis(ly), axis(rx), axis(ry)};
}

void InitWindow(int, int, const char *) {}
bool IsWindowReady() { return !closed.load(); }
void CloseWindow() { ps2x::host::close(); }
bool WindowShouldClose() { return closed.load(); }
void SetConfigFlags(unsigned int) {}
void SetTargetFPS(int) {}
int GetScreenWidth() { return static_cast<int>(maxWidth); }
int GetScreenHeight() { return 448; }
void BeginDrawing() {}
void ClearBackground(Color) {}
void EndDrawing()
{
    // Host presentation must not busy-spin while the guest owns the GS lock.
    const auto next = lastPresent + std::chrono::milliseconds(16);
    std::this_thread::sleep_until(next);
    lastPresent = std::chrono::steady_clock::now();
}
bool IsKeyDown(int) { return false; }
bool IsGamepadAvailable(int gamepad) { return gamepad == 0; }
bool IsGamepadButtonDown(int gamepad, int button)
{
    if (gamepad != 0 || button <= 0 || button >= 32) return false;
    std::lock_guard lock(inputMutex);
    return (buttons & (uint32_t{1} << button)) != 0;
}
float GetGamepadAxisMovement(int gamepad, int index)
{
    if (gamepad != 0 || index < 0 || index >= static_cast<int>(axes.size())) return 0.0f;
    std::lock_guard lock(inputMutex);
    return axes[index];
}
Image GenImageColor(int width, int height, Color color)
{
    if (width <= 0 || height <= 0 || width > static_cast<int>(maxWidth) || height > static_cast<int>(maxHeight)) return {};
    auto *pixels = static_cast<Color *>(std::malloc(static_cast<size_t>(width) * height * sizeof(Color)));
    if (!pixels) return {};
    std::fill_n(pixels, static_cast<size_t>(width) * height, color);
    return {pixels, width, height, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
}
void UnloadImage(Image image) { std::free(image.data); }
Texture2D LoadTextureFromImage(Image image)
{
    if (!image.data || image.width <= 0 || image.height <= 0 || image.width > static_cast<int>(maxWidth) ||
        image.height > static_cast<int>(maxHeight) || image.format != PIXELFORMAT_UNCOMPRESSED_R8G8B8A8) return {};
    textureWidth = image.width; textureHeight = image.height;
    const auto *pixels = static_cast<const uint8_t *>(image.data);
    texture.assign(pixels, pixels + static_cast<size_t>(textureWidth) * textureHeight * 4);
    return {1, image.width, image.height, 1, image.format};
}
void UpdateTexture(Texture2D value, const void *pixels)
{
    if (value.id == 1 && pixels && !texture.empty()) std::memcpy(texture.data(), pixels, texture.size());
}
void UnloadTexture(Texture2D) { texture.clear(); }
void DrawTexturePro(Texture2D value, Rectangle source, Rectangle, Vector2, float, Color)
{
    if (value.id != 1 || texture.empty() || source.x != 0 || source.y != 0 ||
        !std::isfinite(source.width) || !std::isfinite(source.height) || source.width < 1 || source.height < 1 ||
        source.width > textureWidth || source.height > textureHeight) return;
    const uint32_t width = static_cast<uint32_t>(source.width), height = static_cast<uint32_t>(source.height);
    std::lock_guard lock(frameMutex);
    frame.resize(static_cast<size_t>(width) * height * 4);
    for (uint32_t y = 0; y < height; ++y)
        for (uint32_t x = 0; x < width; ++x)
        {
            const size_t src = (static_cast<size_t>(y) * textureWidth + x) * 4;
            const size_t dst = (static_cast<size_t>(y) * width + x) * 4;
            frame[dst] = texture[src + 2]; frame[dst + 1] = texture[src + 1];
            frame[dst + 2] = texture[src]; frame[dst + 3] = 255;
        }
    frameWidth = width; frameHeight = height;
    ++sequence;
}
