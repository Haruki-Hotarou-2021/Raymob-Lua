cmake_minimum_required(VERSION 3.22.1)

# Define padrões para C e C++
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

# Nome do projeto baseado no gradle.properties
project("${APP_LIB_NAME}")

# ==============================================================================
# 1. CONFIGURAÇÃO DO LUAJIT PRÉ-COMPILADO
# ==============================================================================
add_library(luajit SHARED IMPORTED)
set_target_properties(luajit PROPERTIES
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/deps/LuaJIT/android/${ANDROID_ABI}/libluajit.so"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/deps/LuaJIT/src"
)

# ==============================================================================
# 2. SUBDIRETÓRIOS (RAYLIB E RAYMOB)
# ==============================================================================
set(PLATFORM "Android" CACHE STRING "Platform to build for")
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/raylib)
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/raymob)

# ==============================================================================
# 3. AUTOMAÇÃO: EMBUTIR ARQUIVOS LUA NO BINÁRIO
# ==============================================================================
function(embed_lua_file LUA_FILE OUTPUT_H_FILE VARIABLE_NAME)
    add_custom_command(
        OUTPUT ${OUTPUT_H_FILE}
        COMMAND ${CMAKE_COMMAND} -DFILE_IN=${LUA_FILE} -DFILE_OUT=${OUTPUT_H_FILE} -DVAR_NAME=${VARIABLE_NAME} -P ${CMAKE_SOURCE_DIR}/InlineLua.cmake
        DEPENDS ${LUA_FILE}
        COMMENT "Embutindo script Lua: ${LUA_FILE}"
    )
endfunction()

set(GENERATED_LUA_DIR "${CMAKE_BINARY_DIR}/generated_lua")
file(MAKE_DIRECTORY ${GENERATED_LUA_DIR})

set(LUA_GENERATED_HEADERS "")

# Coleta e processa os arquivos internos do JIT da arquitetura correspondente
file(GLOB_RECURSE JIT_SCRIPTS "${CMAKE_SOURCE_DIR}/deps/LuaJIT/android/${ANDROID_ABI}/jit/*.lua")
foreach(SCRIPT ${JIT_SCRIPTS})
    get_filename_component(NAME_WE ${SCRIPT} NAME_WE)
    set(OUTPUT_H "${GENERATED_LUA_DIR}/jit_${NAME_WE}.h")
    embed_lua_file(${SCRIPT} ${OUTPUT_H} "lua_jit_${NAME_WE}")
    list(APPEND LUA_GENERATED_HEADERS ${OUTPUT_H})
endforeach()

# Coleta e processa os arquivos de lua-libs
file(GLOB_RECURSE LUA_LIBS_SCRIPTS "${CMAKE_SOURCE_DIR}/deps/lua-libs/*.lua")
foreach(SCRIPT ${LUA_LIBS_SCRIPTS})
    get_filename_component(NAME_WE ${SCRIPT} NAME_WE)
    set(OUTPUT_H "${GENERATED_LUA_DIR}/lib_${NAME_WE}.h")
    embed_lua_file(${SCRIPT} ${OUTPUT_H} "lua_lib_${NAME_WE}")
    list(APPEND LUA_GENERATED_HEADERS ${OUTPUT_H})
endforeach()

# ==============================================================================
# 4. COLETA DE FONTES DO SEU PROJETO
# ==============================================================================
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/*.c" "${CMAKE_SOURCE_DIR}/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/deps/.*")

include_directories(${ANDROID_NDK}/sources/android/native_app_glue/)
list(APPEND SOURCES ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

# ==============================================================================
# 5. CRIAÇÃO DO BINÁRIO ÚNICO (libgame.so)
# ==============================================================================
add_library(${APP_LIB_NAME} SHARED ${SOURCES} ${LUA_GENERATED_HEADERS})

target_compile_definitions(${APP_LIB_NAME} PRIVATE PLATFORM_ANDROID)
if(CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_definitions(${APP_LIB_NAME} PRIVATE _DEBUG DEBUG)
endif()

target_include_directories(${APP_LIB_NAME} PRIVATE 
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/deps/raylib/src"
    "${CMAKE_SOURCE_DIR}/deps/raymob"
    "${CMAKE_SOURCE_DIR}/deps/LuaJIT/src"
    "${CMAKE_SOURCE_DIR}/deps/lua-libs"
    "${GENERATED_LUA_DIR}"
)

target_link_libraries(${APP_LIB_NAME} 
    raylib 
    raymoblib 
    luajit 
    android 
    log 
    EGL 
    GLESv2
)
