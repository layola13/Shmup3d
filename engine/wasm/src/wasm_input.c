#include "wasm_input.h"
#include <SDL2/SDL.h>
#include <string.h>

#define MAX_TOUCH_POINTS 10

// 输入状态
static int key_states[512] = {0};
static int mouse_x = 0;
static int mouse_y = 0;
static int mouse_button_states[8] = {0};

typedef struct {
    int id;
    float x;
    float y;
    int active;
} touch_point_t;

static touch_point_t touch_points[MAX_TOUCH_POINTS];
static int touch_count = 0;
static wasm_input_callback_t input_callback = NULL;

int wasm_input_init(void) {
    memset(key_states, 0, sizeof(key_states));
    memset(mouse_button_states, 0, sizeof(mouse_button_states));
    memset(touch_points, 0, sizeof(touch_points));
    mouse_x = 0;
    mouse_y = 0;
    touch_count = 0;
    input_callback = NULL;
    
    return 0;
}

void wasm_input_shutdown(void) {
    // 清理输入系统
    input_callback = NULL;
}

void wasm_input_handle_event(void* event_ptr) {
    SDL_Event* event = (SDL_Event*)event_ptr;
    wasm_input_event_t wasm_event = {0};
    
    switch (event->type) {
        case SDL_KEYDOWN:
            if (event->key.keysym.sym < 512) {
                key_states[event->key.keysym.sym] = 1;
            }
            wasm_event.type = WASM_INPUT_KEY_DOWN;
            wasm_event.key_code = event->key.keysym.sym;
            break;
            
        case SDL_KEYUP:
            if (event->key.keysym.sym < 512) {
                key_states[event->key.keysym.sym] = 0;
            }
            wasm_event.type = WASM_INPUT_KEY_UP;
            wasm_event.key_code = event->key.keysym.sym;
            break;
            
        case SDL_MOUSEMOTION:
            mouse_x = event->motion.x;
            mouse_y = event->motion.y;
            wasm_event.type = WASM_INPUT_MOUSE_MOVE;
            wasm_event.mouse_x = event->motion.x;
            wasm_event.mouse_y = event->motion.y;
            break;
            
        case SDL_MOUSEBUTTONDOWN:
            if (event->button.button < 8) {
                mouse_button_states[event->button.button] = 1;
            }
            wasm_event.type = WASM_INPUT_MOUSE_BUTTON_DOWN;
            wasm_event.mouse_x = event->button.x;
            wasm_event.mouse_y = event->button.y;
            wasm_event.mouse_button = event->button.button;
            break;
            
        case SDL_MOUSEBUTTONUP:
            if (event->button.button < 8) {
                mouse_button_states[event->button.button] = 0;
            }
            wasm_event.type = WASM_INPUT_MOUSE_BUTTON_UP;
            wasm_event.mouse_x = event->button.x;
            wasm_event.mouse_y = event->button.y;
            wasm_event.mouse_button = event->button.button;
            break;
            
        case SDL_FINGERDOWN:
            if (touch_count < MAX_TOUCH_POINTS) {
                // 查找空闲的触摸点
                for (int i = 0; i < MAX_TOUCH_POINTS; i++) {
                    if (!touch_points[i].active) {
                        touch_points[i].id = event->tfinger.fingerId;
                        touch_points[i].x = event->tfinger.x;
                        touch_points[i].y = event->tfinger.y;
                        touch_points[i].active = 1;
                        touch_count++;
                        break;
                    }
                }
            }
            wasm_event.type = WASM_INPUT_TOUCH_START;
            wasm_event.touch_id = event->tfinger.fingerId;
            wasm_event.touch_x = event->tfinger.x;
            wasm_event.touch_y = event->tfinger.y;
            break;
            
        case SDL_FINGERMOTION:
            // 更新触摸点位置
            for (int i = 0; i < MAX_TOUCH_POINTS; i++) {
                if (touch_points[i].active && touch_points[i].id == event->tfinger.fingerId) {
                    touch_points[i].x = event->tfinger.x;
                    touch_points[i].y = event->tfinger.y;
                    break;
                }
            }
            wasm_event.type = WASM_INPUT_TOUCH_MOVE;
            wasm_event.touch_id = event->tfinger.fingerId;
            wasm_event.touch_x = event->tfinger.x;
            wasm_event.touch_y = event->tfinger.y;
            break;
            
        case SDL_FINGERUP:
            // 移除触摸点
            for (int i = 0; i < MAX_TOUCH_POINTS; i++) {
                if (touch_points[i].active && touch_points[i].id == event->tfinger.fingerId) {
                    touch_points[i].active = 0;
                    touch_count--;
                    break;
                }
            }
            wasm_event.type = WASM_INPUT_TOUCH_END;
            wasm_event.touch_id = event->tfinger.fingerId;
            wasm_event.touch_x = event->tfinger.x;
            wasm_event.touch_y = event->tfinger.y;
            break;
    }
    
    // 调用回调函数
    if (input_callback) {
        input_callback(&wasm_event);
    }
}

int wasm_input_is_key_down(int key_code) {
    if (key_code < 0 || key_code >= 512) {
        return 0;
    }
    return key_states[key_code];
}

void wasm_input_get_mouse_position(int* x, int* y) {
    if (x) *x = mouse_x;
    if (y) *y = mouse_y;
}

int wasm_input_is_mouse_button_down(int button) {
    if (button < 0 || button >= 8) {
        return 0;
    }
    return mouse_button_states[button];
}

int wasm_input_get_touch_count(void) {
    return touch_count;
}

int wasm_input_get_touch(int index, int* id, float* x, float* y) {
    if (index < 0 || index >= MAX_TOUCH_POINTS) {
        return 0;
    }
    
    if (!touch_points[index].active) {
        return 0;
    }
    
    if (id) *id = touch_points[index].id;
    if (x) *x = touch_points[index].x;
    if (y) *y = touch_points[index].y;
    
    return 1;
}

void wasm_input_set_callback(wasm_input_callback_t callback) {
    input_callback = callback;
}