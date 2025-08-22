#ifndef WASM_INPUT_H
#define WASM_INPUT_H

#include <stdint.h>

// 输入事件类型
typedef enum {
    WASM_INPUT_KEY_DOWN,
    WASM_INPUT_KEY_UP,
    WASM_INPUT_MOUSE_MOVE,
    WASM_INPUT_MOUSE_BUTTON_DOWN,
    WASM_INPUT_MOUSE_BUTTON_UP,
    WASM_INPUT_TOUCH_START,
    WASM_INPUT_TOUCH_MOVE,
    WASM_INPUT_TOUCH_END
} wasm_input_event_type_t;

// 输入事件
typedef struct {
    wasm_input_event_type_t type;
    int key_code;
    int mouse_x;
    int mouse_y;
    int mouse_button;
    int touch_id;
    float touch_x;
    float touch_y;
} wasm_input_event_t;

// 初始化输入系统
int wasm_input_init(void);

// 清理输入系统
void wasm_input_shutdown(void);

// 处理SDL事件
void wasm_input_handle_event(void* sdl_event);

// 获取按键状态
int wasm_input_is_key_down(int key_code);

// 获取鼠标位置
void wasm_input_get_mouse_position(int* x, int* y);

// 获取鼠标按钮状态
int wasm_input_is_mouse_button_down(int button);

// 获取触摸点数量
int wasm_input_get_touch_count(void);

// 获取触摸点信息
int wasm_input_get_touch(int index, int* id, float* x, float* y);

// 设置输入回调
typedef void (*wasm_input_callback_t)(const wasm_input_event_t* event);
void wasm_input_set_callback(wasm_input_callback_t callback);

#endif // WASM_INPUT_H