cmake_minimum_required(VERSION 3.22.1)

# Define padrões para C e C++
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

# Nome do projeto baseado no gradle.properties
project("${APP_LIB_NAME}")

# ===========================================================================
# 1. LUAJIT PRÉ-COMPILADO
# ===========================================================================
add_library(luajit SHARED IMPORTED)
set_target_properties(luajit PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/deps/LuaJIT/android/${ANDROID_ABI}/libluajit.so"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/deps/LuaJIT/src"
)

# ===========================================================================
# 2. SUBDIRETÓRIOS (RAYLIB E RAYMOB)
# ===========================================================================
set(PLATFORM "Android" CACHE STRING "Platform to build for")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/raylib)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/raymob)

# ===========================================================================
# 3. HEADERS GERADOS PELO GRADLE (usar, não gerar!)
# ===========================================================================

# Headers de lua-libs (gerado por compileLuaLibs task)
set(LUA_LIBS_GENERATED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/generated_libs")

# Headers de game (gerado por embedGameAssets task, só em Release)
set(GAME_GENERATED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/generated_game")

# ===========================================================================
# 4. COLETA DE FONTES (C/C++)
# ===========================================================================
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX "${CMAKE_CURRENT_SOURCE_DIR}/deps/.*")

list(APPEND SOURCES "${CMAKE_ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c")

# ===========================================================================
# 5. CRIAÇÃO DO BINÁRIO (libgame.so)
# ===========================================================================
add_library(${APP_LIB_NAME} SHARED ${SOURCES})

target_compile_definitions(${APP_LIB_NAME} PRIVATE PLATFORM_ANDROID)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_definitions(${APP_LIB_NAME} PRIVATE _DEBUG DEBUG)
endif()

# ===========================================================================
# ADICIONAR: Definir EMBED_GAME baseado no argumento CMake
# ===========================================================================
if(EMBED_GAME EQUAL ON)
    target_compile_definitions(${APP_LIB_NAME} PRIVATE EMBED_GAME=1 MODO_EMBUTIDO=1)
else()
    target_compile_definitions(${APP_LIB_NAME} PRIVATE MODO_ASSETS=1)
endif()

# ===========================================================================
# 6. INCLUDES
# ===========================================================================
target_include_directories(${APP_LIB_NAME} PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/deps/raylib/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/deps/raymob"
    "${CMAKE_CURRENT_SOURCE_DIR}/deps/LuaJIT/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/deps/lua-libs"
    "${LUA_LIBS_GENERATED_DIR}"  # ← Headers de lua-libs do Gradle
    "${GAME_GENERATED_DIR}"       # ← Headers de game do Gradle (só existe em Release)
    "${CMAKE_ANDROID_NDK}/sources/android/native_app_glue" 
)

# ===========================================================================
# 7. LINK LIBRARIES
# ===========================================================================
target_link_libraries(${APP_LIB_NAME} 
    raylib 
    raymoblib 
    luajit 
    android 
    log 
    EGL 
    GLESv2
)