#ifndef WASM_FILESYSTEM_H
#define WASM_FILESYSTEM_H

#include <stddef.h>

// 初始化文件系统
int wasm_filesystem_init(void);

// 清理文件系统
void wasm_filesystem_shutdown(void);

// 加载文件
int wasm_filesystem_load_file(const char* path, void** buffer, size_t* size);

// 检查文件是否存在
int wasm_filesystem_file_exists(const char* path);

// 获取文件大小
size_t wasm_filesystem_get_file_size(const char* path);

// 异步加载文件
typedef void (*wasm_filesystem_load_callback)(const char* path, void* buffer, size_t size, void* user_data);
int wasm_filesystem_load_file_async(const char* path, wasm_filesystem_load_callback callback, void* user_data);

// 设置基础路径
void wasm_filesystem_set_base_path(const char* path);

#endif // WASM_FILESYSTEM_H