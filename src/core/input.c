#include "input.h"

#include "core_internal.h"
#include "utils/log.h"
#include <GLFW/glfw3.h>
#include <stddef.h>
#include <string.h>

static bool input_initialized = false;
static bool current_keys[KEY_COUNT];
static bool previous_keys[KEY_COUNT];
static bool current_mouse_buttons[MOUSE_BUTTON_COUNT];
static bool previous_mouse_buttons[MOUSE_BUTTON_COUNT];
static Vec2 mouse_position;
static Vec2 mouse_delta;
static bool has_mouse_sample = false;
static bool cursor_captured = false;

static const KeyCode tracked_keys[] = {
    KEY_SPACE, KEY_APOSTROPHE, KEY_COMMA, KEY_MINUS, KEY_PERIOD, KEY_SLASH,
    KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
    KEY_SEMICOLON, KEY_EQUAL,
    KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J,
    KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T,
    KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,
    KEY_LEFT_BRACKET, KEY_BACKSLASH, KEY_RIGHT_BRACKET, KEY_GRAVE_ACCENT,
    KEY_WORLD_1, KEY_WORLD_2,
    KEY_ESCAPE, KEY_ENTER, KEY_TAB, KEY_BACKSPACE, KEY_INSERT, KEY_DELETE,
    KEY_RIGHT, KEY_LEFT, KEY_DOWN, KEY_UP, KEY_PAGE_UP, KEY_PAGE_DOWN,
    KEY_HOME, KEY_END, KEY_CAPS_LOCK, KEY_SCROLL_LOCK, KEY_NUM_LOCK,
    KEY_PRINT_SCREEN, KEY_PAUSE,
    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9,
    KEY_F10, KEY_F11, KEY_F12, KEY_F13, KEY_F14, KEY_F15, KEY_F16, KEY_F17,
    KEY_F18, KEY_F19, KEY_F20, KEY_F21, KEY_F22, KEY_F23, KEY_F24, KEY_F25,
    KEY_KP_0, KEY_KP_1, KEY_KP_2, KEY_KP_3, KEY_KP_4, KEY_KP_5, KEY_KP_6,
    KEY_KP_7, KEY_KP_8, KEY_KP_9, KEY_KP_DECIMAL, KEY_KP_DIVIDE,
    KEY_KP_MULTIPLY, KEY_KP_SUBTRACT, KEY_KP_ADD, KEY_KP_ENTER, KEY_KP_EQUAL,
    KEY_LEFT_SHIFT, KEY_LEFT_CONTROL, KEY_LEFT_ALT, KEY_LEFT_SUPER,
    KEY_RIGHT_SHIFT, KEY_RIGHT_CONTROL, KEY_RIGHT_ALT, KEY_RIGHT_SUPER,
    KEY_MENU,
};

static bool keycode_is_valid(KeyCode key)
{
    return key >= 0 && key < KEY_COUNT;
}

static bool mouse_button_is_valid(MouseButton button)
{
    return button >= 0 && button < MOUSE_BUTTON_COUNT;
}

static GLFWwindow* require_window(void)
{
    GLFWwindow* window = core_get_window();
    if (window == NULL)
    {
        PANIC("input requires an initialized window");
    }
    return window;
}

void InitInput(void)
{
    memset(current_keys, 0, sizeof(current_keys));
    memset(previous_keys, 0, sizeof(previous_keys));
    memset(current_mouse_buttons, 0, sizeof(current_mouse_buttons));
    memset(previous_mouse_buttons, 0, sizeof(previous_mouse_buttons));
    mouse_position = vec2(0.0f, 0.0f);
    mouse_delta = vec2(0.0f, 0.0f);
    has_mouse_sample = false;
    cursor_captured = false;

    require_window();
    input_initialized = true;
}

void UpdateInput(void)
{
    size_t i;
    GLFWwindow* window;
    double mouse_x;
    double mouse_y;

    if (!input_initialized)
    {
        PANIC("input not initialized");
    }

    window = require_window();

    memcpy(previous_keys, current_keys, sizeof(current_keys));
    memcpy(previous_mouse_buttons, current_mouse_buttons, sizeof(current_mouse_buttons));

    glfwPollEvents();

    for (i = 0; i < sizeof(tracked_keys) / sizeof(tracked_keys[0]); ++i)
    {
        KeyCode key = tracked_keys[i];
        current_keys[key] = glfwGetKey(window, key) == GLFW_PRESS;
    }

    for (i = 0; i < MOUSE_BUTTON_COUNT; ++i)
    {
        current_mouse_buttons[i] = glfwGetMouseButton(window, (int)i) == GLFW_PRESS;
    }

    glfwGetCursorPos(window, &mouse_x, &mouse_y);

    if (!has_mouse_sample)
    {
        mouse_position = vec2((float)mouse_x, (float)mouse_y);
        mouse_delta = vec2(0.0f, 0.0f);
        has_mouse_sample = true;
    }
    else
    {
        Vec2 next_position = vec2((float)mouse_x, (float)mouse_y);
        mouse_delta = sub2(next_position, mouse_position);
        mouse_position = next_position;
    }
}

bool IsKeyDown(KeyCode key)
{
    if (!keycode_is_valid(key))
    {
        return false;
    }
    return current_keys[key];
}

bool IsKeyPressed(KeyCode key)
{
    if (!keycode_is_valid(key))
    {
        return false;
    }
    return current_keys[key] && !previous_keys[key];
}

bool IsKeyReleased(KeyCode key)
{
    if (!keycode_is_valid(key))
    {
        return false;
    }
    return !current_keys[key] && previous_keys[key];
}

bool IsMouseButtonDown(MouseButton button)
{
    if (!mouse_button_is_valid(button))
    {
        return false;
    }
    return current_mouse_buttons[button];
}

bool IsMouseButtonPressed(MouseButton button)
{
    if (!mouse_button_is_valid(button))
    {
        return false;
    }
    return current_mouse_buttons[button] && !previous_mouse_buttons[button];
}

bool IsMouseButtonReleased(MouseButton button)
{
    if (!mouse_button_is_valid(button))
    {
        return false;
    }
    return !current_mouse_buttons[button] && previous_mouse_buttons[button];
}

Vec2 GetMousePosition(void)
{
    return mouse_position;
}

Vec2 GetMouseDelta(void)
{
    return mouse_delta;
}

void SetCursorCaptured(bool captured)
{
    GLFWwindow* window;

    if (!input_initialized)
    {
        PANIC("input not initialized");
    }

    window = require_window();
    glfwSetInputMode(window, GLFW_CURSOR, captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    cursor_captured = captured;
    has_mouse_sample = false;
    mouse_delta = vec2(0.0f, 0.0f);
}

bool IsCursorCaptured(void)
{
    return cursor_captured;
}
