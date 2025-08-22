#!/bin/bash

# Shmup3D WebAssembly构建脚本

# 检查emscripten是否安装
if [ -z "$EMSDK" ]; then
    echo "错误: EMSDK环境变量未设置"
    echo "请运行: source /path/to/emsdk/emsdk_env.sh"
    exit 1
fi

# 设置构建目录
BUILD_DIR="build"
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# 清理之前的构建
echo "清理之前的构建..."
rm -rf *

# 配置CMake
echo "配置CMake..."
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release

# 构建项目
echo "构建项目..."
emmake make -j$(nproc)

# 检查构建结果
if [ -f "shmup_wasm.html" ]; then
    echo "构建成功!"
    echo "输出文件:"
    echo "  - shmup_wasm.html (主页面)"
    echo "  - shmup_wasm.js (JavaScript胶水代码)"
    echo "  - shmup_wasm.wasm (WASM二进制)"
    echo "  - shmup_wasm.data (预加载资源)"
    
    # 复制到web目录
    cp shmup_wasm.* ../web/
    echo "文件已复制到web目录"
else
    echo "构建失败!"
    exit 1
fi

# 启动本地服务器进行测试
echo ""
echo "要测试Web版本，请运行:"
echo "cd ../web"
echo "python3 -m http.server 8000"
echo "然后在浏览器中访问: http://localhost:8000"