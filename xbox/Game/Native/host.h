#pragma once
#include <cstdint>

#if defined(_WIN32)
#define PS2X_EXPORT extern "C" __declspec(dllexport)
#else
#define PS2X_EXPORT extern "C" __attribute__((visibility("default")))
#endif

// Pixel storage is supplied by the caller. Frames are opaque BGRA, tightly packed.
PS2X_EXPORT uint64_t ps2x_copy_frame(uint8_t *pixels, uint32_t capacity,
                                   uint32_t *width, uint32_t *height);
// Buttons use the raylib GamepadButton indices as bit positions. Axes are [-1, 1], Y down.
PS2X_EXPORT void ps2x_set_input(uint32_t buttons, float lx, float ly, float rx, float ry);
PS2X_EXPORT int ps2x_start(const char *elfPath);
PS2X_EXPORT void ps2x_stop();
// 0 idle, 1 running, 2 finished, -1 failed. Error is UTF-8, always terminated when capacity > 0.
PS2X_EXPORT int ps2x_status(char *error, uint32_t capacity);

namespace ps2x::host
{
    void reset();
    void close();
}
