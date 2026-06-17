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

# ====================================================================
# 1. COLETA DINÂMICA DE FONTES DO LUAJIT
# ====================================================================
# Coleta todos os arquivos do núcleo do interpretador e bibliotecas padrão
file(GLOB LUAJIT_CORE_SOURCES
    "${LUAJIT_SRC_DIR}/lj_*.c"
    "${LUAJIT_SRC_DIR}/lib_*.c"
)

# Remove arquivos geradores de host de desktop (não usados em cross-compilation para Android)
list(FILTER LUAJIT_CORE_SOURCES EXCLUDE REGEX "lj_vmfit.c|lj_ccallback.c")

# Adiciona o motor em Assembly core do LuaJIT
list(APPEND LUAJIT_CORE_SOURCES "${LUAJIT_SRC_DIR}/lj_vm.S")

# Create LuaJIT static library
add_library(luajit STATIC ${LUAJIT_CORE_SOURCES})

# Include directories for LuaJIT
target_include_directories(luajit PUBLIC ${LUAJIT_SRC_DIR})

# Compiler flags for Android
target_compile_options(luajit PRIVATE -O2 -fPIC)

# Define flags globais do LuaJIT
target_compile_definitions(luajit PRIVATE LUAJIT_ENABLE_LUA52COMPAT)

# ====================================================================
# 2. DEFINIÇÃO DINÂMICA DE ABI PARA O APK UNIVERSAL
# ====================================================================
# O Gradle chamará este script para cada ABI. Mapeamos as macros internas 
# do LuaJIT de acordo com a arquitetura alvo atual (ANDROID_ABI).
if(ANDROID_ABI STREQUAL "arm64-v8a")
    target_compile_definitions(luajit PRIVATE LJVM_TARGET=LJ_TARGET_ARM64 LUAJIT_TARGET=LUAJIT_ARCH_ARM64)
elseif(ANDROID_ABI STREQUAL "armeabi-v7a")
    target_compile_definitions(luajit PRIVATE LJVM_TARGET=LJ_TARGET_ARM LUAJIT_TARGET=LUAJIT_ARCH_ARM)
elseif(ANDROID_ABI STREQUAL "x86_64")
    target_compile_definitions(luajit PRIVATE LJVM_TARGET=LJ_TARGET_X64 LUAJIT_TARGET=LUAJIT_ARCH_X64)
elseif(ANDROID_ABI STREQUAL "x86")
    target_compile_definitions(luajit PRIVATE LJVM_TARGET=LJ_TARGET_X86 LUAJIT_TARGET=LUAJIT_ARCH_X86)
else()
    message(FATAL_ERROR "Arquitetura Android nao suportada pelo LuaJIT: ${ANDROID_ABI}")
endif()

# Enable C language interpreter for assembly files (.S)
set_source_files_properties(${LUAJIT_SRC_DIR}/lj_vm.S PROPERTIES LANGUAGE C)

# ====================================================================
# 3. COMPILAÇÃO DO SEU JOGO (SHARED LIBRARY)
# ====================================================================
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
# Vincula o raylib, raymob, luajit e as bibliotecas nativas essenciais do Android NDK
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
