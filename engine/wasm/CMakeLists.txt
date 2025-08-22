cmake_minimum_required(VERSION 3.22)
project(shmup_wasm)

# 设置emscripten工具链
if(NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake" CACHE PATH "Emscripten toolchain file")
endif()

# 设置WASM编译选项
set(CMAKE_EXECUTABLE_SUFFIX ".html")

# 使用target_link_options来设置emscripten链接选项
set(EMSCRIPTEN_LINK_OPTIONS
    -sALLOW_MEMORY_GROWTH=1
    -sEXPORTED_FUNCTIONS=_main,_malloc,_free,_TEX_UnloadTexture,_TEX_LoadFromDiskAndUploadToGPU
    -sEXPORTED_RUNTIME_METHODS=ccall,cwrap,GL
    -sMODULARIZE=0
    -sUSE_WEBGL2=1
    -sFULL_ES3=1
    -sUSE_SDL=2
    -sSDL2_IMAGE_FORMATS=["png"]
    -sINITIAL_MEMORY=134217728
    -sMAXIMUM_MEMORY=268435456
    -sSTACK_SIZE=16777216
    -sASSERTIONS=2
    -sSAFE_HEAP=1
    -sSTACK_OVERFLOW_CHECK=2
    -sGL_DEBUG=1
    -sGL_TRACK_ERRORS=1
    -g3
    -O0
    --preload-file ${CMAKE_SOURCE_DIR}/data/data@/data
    --use-preload-plugins
    -sNO_EXIT_RUNTIME=1
)

# 编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-implicit-function-declaration")

# 包含路径
set(BASE_DIR "${CMAKE_SOURCE_DIR}/../src")
set(INCLUDES
    ${BASE_DIR}
    ${BASE_DIR}/libpng
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/../src/filesystem
)

# 源文件
file(GLOB SOURCES ${BASE_DIR}/*.c)
file(GLOB WASM_SOURCES ${CMAKE_SOURCE_DIR}/src/*.c)
list(REMOVE_ITEM WASM_SOURCES ${CMAKE_SOURCE_DIR}/src/wasm_stub.c)
list(APPEND WASM_SOURCES ${CMAKE_SOURCE_DIR}/src/wasm_net_stub.c)
list(APPEND WASM_SOURCES ${CMAKE_SOURCE_DIR}/src/wasm_platform_stub.c)
file(GLOB FILESYSTEM_SOURCES ${CMAKE_SOURCE_DIR}/../src/filesystem/*.c)

# 排除不需要的文件
list(REMOVE_ITEM SOURCES
    ${BASE_DIR}/android/android_main.c
    ${BASE_DIR}/android/android_display.c
    ${BASE_DIR}/android/android_filesystem.c
    ${BASE_DIR}/android/android_music.c
    ${BASE_DIR}/android/android_native.c
    ${BASE_DIR}/android/android_URL.c
    ${BASE_DIR}/android/native_app_glue.c
    ${BASE_DIR}/netchannel.c
    ${BASE_DIR}/renderer_progr.c
    ${BASE_DIR}/renderer_fixed.c
)

# PNG库文件
set(PNG_SOURCES
    ${BASE_DIR}/libpng/png.c
    ${BASE_DIR}/libpng/pngerror.c
    ${BASE_DIR}/libpng/pngget.c
    ${BASE_DIR}/libpng/pngmem.c
    ${BASE_DIR}/libpng/pngpread.c
    ${BASE_DIR}/libpng/pngread.c
    ${BASE_DIR}/libpng/pngrio.c
    ${BASE_DIR}/libpng/pngrtran.c
    ${BASE_DIR}/libpng/pngrutil.c
    ${BASE_DIR}/libpng/pngset.c
    ${BASE_DIR}/libpng/pngtrans.c
    ${BASE_DIR}/libpng/pngwio.c
    ${BASE_DIR}/libpng/pngwrite.c
    ${BASE_DIR}/libpng/pngwtran.c
    ${BASE_DIR}/libpng/pngwutil.c
)

# 创建可执行文件
add_executable(shmup_wasm ${SOURCES} ${WASM_SOURCES} ${PNG_SOURCES})

# 设置包含目录
target_include_directories(shmup_wasm PRIVATE ${INCLUDES})

# 设置链接选项
target_link_options(shmup_wasm PRIVATE ${EMSCRIPTEN_LINK_OPTIONS})