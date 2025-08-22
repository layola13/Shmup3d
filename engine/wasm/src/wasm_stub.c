// wasm_stub.c - 提供缺失的符号定义
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 文件系统相关
// FS_GetExtensionAddress and FS_GetFilenameOnly are now implemented in wasm_filesystem.c
// to match the function signatures required by the engine's filesystem.h

// FS_Write, FS_GetLoadedFilePointer, and FS_Gamedir are now implemented in wasm_filesystem.c

// 网络相关
void NET_Setup(void) {}
void NET_Receive(void) {}
void NET_Send(void) {}
void NET_Free(void) {}
int NET_IsRunning(void) { return 0; }
void NET_OnNextLevelLoad(void) {}
int NET_Init(void) { return 0; }
void Net_SendDie(int player) {}

// 声音相关
void SND_InitSoundTrack(int frequency, int channels) {}
void SND_StartSoundTrack(void) {}
void SND_StopSoundTrack(void) {}
void SND_BACKEND_Upload(void* data, int size) {}
void SND_BACKEND_Init(void) {}
void SND_BACKEND_Play(int id) {}

// 其他
void Native_UploadScore(int score) {}
// FS_InitFilesystem is now implemented in wasm_filesystem.c
// Match signature from dEngine.h which is likely int(...)
int dEngine_resize(int width, int height) {
    // Stub implementation
    return 0;
}

// Match signature from texture.c void loadNativePNG(texture_t*);
// We use void* to avoid including texture.h in this stub file.
void loadNativePNG(void* texture) {
    // Stub implementation
}

// Match signature from texture.c void loadNativePVRT(texture_t*);
// We use void* to avoid including texture.h in this stub file.
void loadNativePVRT(void* texture) {
    // Stub implementation
}


// 网络统计
int NET_GetDropedPackets(void) { return 0; }
int net = 0;