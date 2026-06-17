# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.22.1)

# Set standards for C and C++
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

# Set the project name based on the name given on the gradle.properties
project("${APP_LIB_NAME}")

# Include raylib and raymob as subdirectories
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/raylib)
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/raymob)

# Build LuaJIT from source
set(LUAJIT_DIR ${CMAKE_SOURCE_DIR}/deps/luajit)
set(LUAJIT_SRC_DIR ${LUAJIT_DIR}/src)

# LuaJIT core source files (Mantendo a estrutura precisa do Raymob + Assembly)
set(LUAJIT_CORE_SOURCES
    ${LUAJIT_SRC_DIR}/lj_gc.c
    ${LUAJIT_SRC_DIR}/lj_err.c
    ${LUAJIT_SRC_DIR}/lj_lex.c
    ${LUAJIT_SRC_DIR}/lj_parse.c
    ${LUAJIT_SRC_DIR}/lj_bcread.c
    ${LUAJIT_SRC_DIR}/lj_bcwrite.c
    ${LUAJIT_SRC_DIR}/lj_load.c
    ${LUAJIT_SRC_DIR}/lj_ir.c
    ${LUAJIT_SRC_DIR}/lj_opt_mem.c
    ${LUAJIT_SRC_DIR}/lj_opt_fold.c
    ${LUAJIT_SRC_DIR}/lj_opt_dce.c
    ${LUAJIT_SRC_DIR}/lj_opt_loop.c
    ${LUAJIT_SRC_DIR}/lj_opt_split.c
    ${LUAJIT_SRC_DIR}/lj_opt_sink.c
    ${LUAJIT_SRC_DIR}/lj_mcode.c
    ${LUAJIT_SRC_DIR}/lj_snap.c
    ${LUAJIT_SRC_DIR}/lj_record.c
    ${LUAJIT_SRC_DIR}/lj_crecord.c
    ${LUAJIT_SRC_DIR}/lj_ffrecord.c
    ${LUAJIT_SRC_DIR}/lj_asm.c
    ${LUAJIT_SRC_DIR}/lj_trace.c
    ${LUAJIT_SRC_DIR}/lj_gdebug.c
    ${LUAJIT_SRC_DIR}/lj_capi.c
    ${LUAJIT_SRC_DIR}/lj_ccallback.c
    ${LUAJIT_SRC_DIR}/lj_ccall.c
    ${LUAJIT_SRC_DIR}/lj_carith.c
    ${LUAJIT_SRC_DIR}/lj_clib.c
    ${LUAJIT_SRC_DIR}/lj_cparse.c
    ${LUAJIT_SRC_DIR}/lj_lib.c
    ${LUAJIT_SRC_DIR}/lj_alloc.c
    ${LUAJIT_SRC_DIR}/lib_aux.c
    ${LUAJIT_SRC_DIR}/lib_base.c
    ${LUAJIT_SRC_DIR}/lib_math.c
    ${LUAJIT_SRC_DIR}/lib_bit.c
    ${LUAJIT_SRC_DIR}/lib_string.c
    ${LUAJIT_SRC_DIR}/lib_table.c
    ${LUAJIT_SRC_DIR}/lib_io.c
    ${LUAJIT_SRC_DIR}/lib_os.c
    ${LUAJIT_SRC_DIR}/lib_package.c
    ${LUAJIT_SRC_DIR}/lib_debug.c
    ${LUAJIT_SRC_DIR}/lib_jit.c
    ${LUAJIT_SRC_DIR}/lib_ffi.c
    ${LUAJIT_SRC_DIR}/lj_vm.S
)

# Create LuaJIT static library
add_library(luajit STATIC ${LUAJIT_CORE_SOURCES})

# Include directories for LuaJIT
target_include_directories(luajit PUBLIC ${LUAJIT_SRC_DIR})

# Compiler flags for Android
target_compile_options(luajit PRIVATE -O2 -fPIC)

# Define flags
target_compile_definitions(luajit PRIVATE LUAJIT_ENABLE_LUA52COMPAT)

# Enable C language for assembly files
set_source_files_properties(${LUAJIT_SRC_DIR}/lj_vm.S PROPERTIES LANGUAGE C)

# Fetch all source files for your project (recursively), excluding 'deps' source files
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/*.c" "${CMAKE_SOURCE_DIR}/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/deps/.*")

# Add headers directory for android_native_app_glue.c
include_directories(${ANDROID_NDK}/sources/android/native_app_glue/)

# Add android_native_app_glue.c to the source files
list(APPEND SOURCES ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

# Create a shared library with game source files
add_library(${APP_LIB_NAME} SHARED ${SOURCES})

# Define compiler macros for the library
target_compile_definitions(${APP_LIB_NAME} PRIVATE PLATFORM_ANDROID)

# Apply flags depending on the build type
if(CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_definitions(${APP_LIB_NAME} PRIVATE _DEBUG DEBUG)
endif()

# ------------------------------------------------------------------------
# INTEGRAÇÃO: Mapeamento de diretórios gerados pelo Gradle e dependências
# ------------------------------------------------------------------------
target_include_directories(${APP_LIB_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/deps/raylib")
target_include_directories(${APP_LIB_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/deps/raymob")
target_include_directories(${APP_LIB_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/deps/luajit/src")
target_include_directories(${APP_LIB_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/generated_libs")
target_include_directories(${APP_LIB_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/generated_game")

# Link required libraries to the native application
# Adicionado as dependências do sistema Android nativo (log, android, EGL, GLESv2, m)
target_link_libraries(${APP_LIB_NAME} 
    raylib 
    raymoblib 
    luajit
    android
    log
    EGL
    GLESv2
    m
)
