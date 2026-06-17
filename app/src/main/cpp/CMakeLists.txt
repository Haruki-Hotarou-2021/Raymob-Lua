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
# CORREÇÃO: Coleta dinâmica automática e segura dos fontes do LuaJIT
# ====================================================================
# Coleta todos os arquivos do núcleo do interpretador e bibliotecas padrão
file(GLOB LUAJIT_CORE_SOURCES
    "${LUAJIT_SRC_DIR}/lj_*.c"
    "${LUAJIT_SRC_DIR}/lib_*.c"
)

# Remove arquivos geradores de host de desktop que quebram a compilação cruzada do Android
list(FILTER LUAJIT_CORE_SOURCES EXCLUDE REGEX "lj_vmfit.c|lj_ccallback.c")

# Adiciona explicitamente o motor de assembly core
list(APPEND LUAJIT_CORE_SOURCES "${LUAJIT_SRC_DIR}/lj_vm.S")
# ====================================================================

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
