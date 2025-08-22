#ifndef WASM_AUDIO_H
#define WASM_AUDIO_H

#include <stdint.h>

// 初始化Web Audio
int wasm_audio_init(void);

// 清理音频系统
void wasm_audio_shutdown(void);

// 加载音频文件
int wasm_audio_load_sound(const char* path);

// 播放音效
void wasm_audio_play_sound(int sound_id, float volume, float pan);

// 停止音效
void wasm_audio_stop_sound(int sound_id);

// 设置主音量
void wasm_audio_set_master_volume(float volume);

// 暂停所有音频
void wasm_audio_pause_all(void);

// 恢复所有音频
void wasm_audio_resume_all(void);

// 更新音频系统
void wasm_audio_update(void);

#endif // WASM_AUDIO_H