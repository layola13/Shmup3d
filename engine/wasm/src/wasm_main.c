#include <emscripten.h>
#include <emscripten/html5.h>
#include <SDL2/SDL.h>
#include <GLES3/gl3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 引擎头文件
#include "dEngine.h"
#include "wasm_display.h"
#include "wasm_filesystem.h"
#include "wasm_audio.h"
#include "wasm_input.h"

// 全局变量
static bool engine_running = false;
static int window_width = 800;
static int window_height = 600;

// 主函数
int main(int argc, char* argv[]) {
    printf("Shmup3D WebAssembly版本启动\n");
    
    // 初始化引擎
    if (!dEngine_Init()) {
        printf("引擎初始化失败\n");
        return 1;
    }
    
    engine_running = true;
    
    // 启动主循环
    emscripten_set_main_loop(dEngine_HostFrame, 0, 1);
    
    return 0;
}

// 导出的函数供JavaScript调用
EMSCRIPTEN_KEEPALIVE
void pause_game() {
    dEngine_Pause();
}

EMSCRIPTEN_KEEPALIVE
void resume_game() {
    dEngine_Resume();
}

EMSCRIPTEN_KEEPALIVE
void resize_game(int width, int height) {
    dEngine_resize(width, height);
}