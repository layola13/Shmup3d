#include "wasm_audio.h"
#include <emscripten.h>
#include <stdio.h>

// Web Audio API的JavaScript接口
EM_JS(void, js_audio_init, (), {
    if (!Module.audioContext) {
        Module.audioContext = new (window.AudioContext || window.webkitAudioContext)();
    }
});

EM_JS(void, js_audio_play_sound, (int sound_id, float volume, float pan), {
    if (Module.audioContext) {
        // 这里简化处理，实际应该根据sound_id加载对应的音频文件
        console.log("Playing sound:", sound_id, "volume:", volume, "pan:", pan);
    }
});

EM_JS(void, js_audio_pause, (), {
    if (Module.audioContext) {
        Module.audioContext.suspend();
    }
});

EM_JS(void, js_audio_resume, (), {
    if (Module.audioContext) {
        Module.audioContext.resume();
    }
});

// 初始化音频系统
int wasm_audio_init(void) {
    js_audio_init();
    return 0;
}

// 播放音效
void wasm_audio_play_sound(int sound_id, float volume, float pan) {
    js_audio_play_sound(sound_id, volume, pan);
}

// 暂停音频
void wasm_audio_pause(void) {
    js_audio_pause();
}

// 恢复音频
void wasm_audio_resume(void) {
    js_audio_resume();
}

// 清理音频资源
void wasm_audio_cleanup(void) {
    // Web Audio API会自动清理
}

// 占位符函数，避免链接错误
int Mix_OpenAudio(int frequency, uint16_t format, int channels, int chunksize) {
    (void)frequency; (void)format; (void)channels; (void)chunksize;
    return 0;
}

void Mix_Init(int flags) {
    (void)flags;
}

void Mix_CloseAudio(void) {
}

void Mix_FreeChunk(void* chunk) {
    (void)chunk;
}

void* Mix_LoadWAV(const char* file) {
    (void)file;
    return NULL;
}

void Mix_PlayChannel(int channel, void* chunk, int loops) {
    (void)channel; (void)chunk; (void)loops;
}