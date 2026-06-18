cmake_minimum_required(VERSION 3.22.1)

# Set standards for C and C++
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

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
# 3. EMBUTIR ARQUIVOS LUA (JIT + LUA-LIBS) NO CÓDIGO FONTE
# ==============================================================================
# Função auxiliar para transformar arquivos .lua em headers C (.h) com hex arrays
function(embed_lua_file LUA_FILE OUTPUT_H_FILE VARIABLE_NAME)
    add_custom_command(
        OUTPUT ${OUTPUT_H_FILE}
        COMMAND ${CMAKE_COMMAND} -DFILE_IN=${LUA_FILE} -DFILE_OUT=${OUTPUT_H_FILE} -DVAR_NAME=${VARIABLE_NAME} -P ${CMAKE_SOURCE_DIR}/cmake/InlineLua.cmake
        DEPENDS ${LUA_FILE}
        COMMENT "Embutindo script Lua: ${LUA_FILE}"
    )
endfunction()

# Coleta todos os arquivos .lua das pastas de bibliotecas
file(GLOB_RECURSE JIT_SCRIPTS "${CMAKE_SOURCE_DIR}/deps/LuaJIT/android/${ANDROID_ABI}/jit/*.lua")
file(GLOB_RECURSE LUA_LIBS_SCRIPTS "${CMAKE_SOURCE_DIR}/deps/lua-libs/*.lua")

# Loop para gerar os arquivos gerados (Exemplo conceitual: você pode incluí-los no main.c)
set(EMBEDDED_LUA_HEADERS "")
# Nota: Para automação completa de arquivos dinâmicos, o ideal é mapeá-los no main.c via tabela nativa.

# ==============================================================================
# 4. COLETA DE FONTES DO PROJETO (SEU CÓDIGO)
# ==============================================================================
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/*.c" "${CMAKE_SOURCE_DIR}/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/deps/.*")

# Adiciona o glue do Android Native App
include_directories(${ANDROID_NDK}/sources/android/native_app_glue/)
list(APPEND SOURCES ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

# ==============================================================================
# 5. CRIAÇÃO E CONFIGURAÇÃO DO BINÁRIO FINAL (libgame.so)
# ==============================================================================
add_library(${APP_LIB_NAME} SHARED ${SOURCES})

target_compile_definitions(${APP_LIB_NAME} PRIVATE PLATFORM_ANDROID)
if(CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_definitions(${APP_LIB_NAME} PRIVATE _DEBUG DEBUG)
endif()

# Include directories para compilação do seu main.c
target_include_directories(${APP_LIB_NAME} PRIVATE 
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/deps/raylib/src"
    "${CMAKE_SOURCE_DIR}/deps/raymob"
    "${CMAKE_SOURCE_DIR}/deps/LuaJIT/src"
    "${CMAKE_SOURCE_DIR}/deps/lua-libs"
    ${CMAKE_BINARY_DIR}/generated_lua # Onde os headers de scripts embutidos vão ficar
)

# Linkagem estática/dinâmica das dependências no binário final
target_link_libraries(${APP_LIB_NAME} 
    raylib 
    raymoblib 
    luajit         # Linka o libluajit.so pré-compilado da arquitetura correta
    android 
    log 
    EGL 
    GLESv2
)
