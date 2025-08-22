# Shmup3D WebAssembly版本

## 概述
这是Shmup3D的WebAssembly版本，使用emscripten工具链将C语言游戏引擎编译为WebAssembly，支持在现代浏览器中运行。

## 技术栈
- **语言**: C99
- **编译工具**: emscripten (emsdk)
- **图形API**: WebGL 2.0 (OpenGL ES 3.0)
- **音频API**: Web Audio API (通过SDL2_mixer)
- **输入**: 键盘、鼠标、触摸
- **构建系统**: CMake

## 文件结构
```
engine/wasm/
├── CMakeLists.txt          # CMake构建配置
├── build.sh               # 构建脚本
├── src/                   # WASM特定源代码
│   ├── wasm_main.c        # 主入口点
│   ├── wasm_display.c     # WebGL显示适配
│   ├── wasm_filesystem.c  # HTTP文件系统
│   ├── wasm_audio.c       # Web Audio后端
│   └── wasm_input.c       # 输入处理
├── web/                   # Web前端文件
│   ├── index.html         # 主页面
│   └── styles.css         # 样式文件
└── README.md              # 本文档
```

## 构建要求
- **emscripten SDK**: 已安装并配置
- **CMake**: 3.22或更高版本
- **Python3**: 用于本地测试服务器

## 构建步骤

### 1. 设置emscripten环境
```bash
source /path/to/emsdk/emsdk_env.sh
```

### 2. 构建项目
```bash
cd engine/wasm
./build.sh
```

### 3. 本地测试
```bash
cd web
python3 -m http.server 8000
# 访问 http://localhost:8000
```

## 功能特性
- ✅ 完整的3D图形渲染
- ✅ 音效和背景音乐
- ✅ 键盘/鼠标/触摸输入
- ✅ 响应式设计
- ✅ 全屏支持
- ✅ 资源预加载

## 浏览器兼容性
- **Chrome**: 57+ (推荐)
- **Firefox**: 52+
- **Safari**: 11+
- **Edge**: 16+

## 性能优化
- 使用WebGL 2.0加速
- 资源预加载和缓存
- 内存动态增长
- 代码压缩和优化

## 已知限制
- 需要WebGL 2.0支持
- 首次加载需要下载资源包
- 移动设备性能可能受限
- 需要HTTPS才能使用全部功能

## 调试
使用浏览器开发者工具查看控制台输出和性能分析。

## 部署
将`web/`目录下的所有文件部署到支持静态文件的服务器即可。