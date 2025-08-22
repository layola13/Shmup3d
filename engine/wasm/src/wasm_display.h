#ifndef WASM_DISPLAY_H
#define WASM_DISPLAY_H

#include <stdint.h>
#include "renderer.h" // For renderer_t

// 初始化WebGL显示
int wasm_display_init(int width, int height);

// 绑定渲染器方法
void wasm_display_bind_renderer_methods(renderer_t* renderer);

// 清理显示资源
void wasm_display_shutdown(void);

// 调整显示大小
void wasm_display_resize(int width, int height);

// 获取当前显示尺寸
void wasm_display_get_size(int* width, int* height);

// 交换缓冲区
void wasm_display_swap_buffers(void);

// 设置全屏模式
void wasm_display_set_fullscreen(int fullscreen);

// 检查是否支持WebGL2
int wasm_display_check_webgl2_support(void);

#endif // WASM_DISPLAY_H