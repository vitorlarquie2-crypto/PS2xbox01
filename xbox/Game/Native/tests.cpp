#include "host.h"
#include "raylib.h"
#include <array>
#include <cmath>
#include <cstdio>
#include <limits>

int main()
{
    int failures = 0;
    auto check = [&](bool valid, const char *message)
    {
        if (!valid) { std::fprintf(stderr, "%s\n", message); ++failures; }
    };
    ps2x::host::reset();
    Image image = GenImageColor(3, 2, Color{10, 20, 30, 128});
    auto *colors = static_cast<Color *>(image.data);
    colors[3] = Color{40, 50, 60, 0};
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);
    DrawTexturePro(texture, Rectangle{0, 0, 2, 2}, {}, {}, 0, WHITE);
    std::array<uint8_t, 20> pixels;
    pixels.fill(0xcd);
    uint32_t width = 0, height = 0;
    check(ps2x_copy_frame(pixels.data(), 15, &width, &height) == 0, "Small buffer accepted");
    check(pixels[0] == 0xcd && width == 2 && height == 2, "Small buffer was modified or size unavailable");
    check(ps2x_copy_frame(pixels.data(), 16, &width, &height) == 1, "Frame unavailable");
    check(pixels[0] == 30 && pixels[1] == 20 && pixels[2] == 10 && pixels[3] == 255, "BGRA conversion failed");
    check(pixels[8] == 60 && pixels[9] == 50 && pixels[10] == 40 && pixels[11] == 255, "Source row stride failed");
    check(pixels[16] == 0xcd, "Frame copy exceeded capacity");
    check(ps2x_copy_frame(pixels.data(), 20, nullptr, &height) == 0, "Null dimension accepted");
    DrawTexturePro(texture, Rectangle{0, 0, 4, 2}, {}, {}, 0, WHITE);
    check(ps2x_copy_frame(pixels.data(), 20, &width, &height) == 1, "Invalid crop published");
    ps2x_set_input(1u << GAMEPAD_BUTTON_RIGHT_FACE_DOWN, 2, -2, std::numeric_limits<float>::quiet_NaN(), 0.5f);
    check(IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN), "Cross button missing");
    check(!IsGamepadButtonDown(1, GAMEPAD_BUTTON_RIGHT_FACE_DOWN), "Second gamepad accepted");
    check(GetGamepadAxisMovement(0, 0) == 1 && GetGamepadAxisMovement(0, 1) == -1, "Axis clamp failed");
    check(GetGamepadAxisMovement(0, 2) == 0 && GetGamepadAxisMovement(0, 3) == 0.5f, "Axis sanitization failed");
    check(GetGamepadAxisMovement(0, -1) == 0 && !IsGamepadButtonDown(0, 32), "Invalid input index accepted");
    ps2x::host::close();
    InitWindow(640, 448, "test");
    check(WindowShouldClose(), "Late initialization lost stop request");
    ps2x::host::reset();
    check(!WindowShouldClose() && !IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN), "Reset retained input/stop state");
    check(ps2x_copy_frame(pixels.data(), 20, &width, &height) == 0 && width == 0 && height == 0, "Reset retained old frame");
    std::printf("Embedded host: %d failure(s)\n", failures);
    return failures ? 1 : 0;
}
