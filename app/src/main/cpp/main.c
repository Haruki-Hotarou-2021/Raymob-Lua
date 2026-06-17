#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include <string.h>
#include <stdlib.h>

// ------------------------------------------------------------------------
// 1. INCLUSÃO DAS LUA-LIBS FIXAS (Disponíveis tanto em Debug quanto em Release)
// ------------------------------------------------------------------------
// Seus arquivos dentro de "lua-libs" geram cabeçalhos aqui. 
// Vamos incluir o utils e o arquivo de bindings da Raylib que ficará lá.
#include "generated_libs/utils.lua.h"
#include "generated_libs/raylib_ffi.lua.h" 

// ------------------------------------------------------------------------
// 2. CONFIGURAÇÃO POR BUILD TYPE (Debug vs Release)
// ------------------------------------------------------------------------
#ifdef MODO_EMBUTIDO
    // Modo Release: Inclui a lógica do jogo inteira embutida no binário
    #include "generated_game/main.lua.h"
    #include "generated_game/player.lua.h"
#else
    // Modo Debug: Carrega o jogo dinamicamente da pasta assets do APK
    #include <android/asset_manager.h>
    #include <android/asset_manager_jni.h>
    #include <android_native_app_glue.h>
    
    AAssetManager* android_asset_manager = NULL;
#endif

// ------------------------------------------------------------------------
// 3. O BUSCADOR INTELIGENTE (Resolve require das libs e do jogo)
// ------------------------------------------------------------------------
static int custom_lua_searcher(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);

    // ====================================================================
    // CAMADA A: PROCURA NA PASTA "lua-libs" (Módulos internos do motor)
    // ====================================================================
    if (strcmp(name, "utils") == 0) {
        luaL_loadbuffer(L, (const char*)lib_utils, lib_utils_len, "lua-libs/utils.lua");
        return 1;
    }
    else if (strcmp(name, "raylib_ffi") == 0) {
        luaL_loadbuffer(L, (const char*)lib_raylib_ffi, lib_raylib_ffi_len, "lua-libs/raylib_ffi.lua");
        return 1;
    }
    // Toda biblioteca interna nova que você colocar em "lua-libs" ganha um 'else if' aqui.

    // ====================================================================
    // CAMADA B: PROCURA NA RAIZ DO JOGO (Seus scripts customizados)
    // ====================================================================
#ifdef MODO_EMBUTIDO
    // --- FLUXO RELEASE (Lendo o Jogo da Memória) ---
    if (strcmp(name, "main") == 0) {
        luaL_loadbuffer(L, (const char*)game_main, game_main_len, "main.lua");
        return 1;
    }
    else if (strcmp(name, "player") == 0) {
        luaL_loadbuffer(L, (const char*)game_player, game_player_len, "player.lua");
        return 1;
    }
#else
    // --- FLUXO DEBUG (Lendo o Jogo dos Assets do APK) ---
    char filename[256];
    snprintf(filename, sizeof(filename), "%s.lua", name);
    
    // Suporta subpastas no require do jogo (ex: require("scripts.player") -> "scripts/player.lua")
    for (int i = 0; filename[i]; i++) {
        if (filename[i] == '.') filename[i] = '/';
    }

    AAsset* asset = AAssetManager_open(android_asset_manager, filename, AASSET_MODE_BUFFER);
    if (asset) {
        size_t size = AAsset_getLength(asset);
        char* buffer = (char*)malloc(size);
        AAsset_read(asset, buffer, size);
        AAsset_close(asset);

        if (luaL_loadbuffer(L, buffer, size, filename) != 0) {
            free(buffer);
            return lua_error(L);
        }
        free(buffer);
        return 1;
    }
#endif

    // Se o require não bateu com nenhuma lib interna e nenhum arquivo do jogo:
    lua_pushfstring(L, "\n\t[Engine] Não foi possível encontrar o módulo '%s'.", name);
    return 1;
}

// ------------------------------------------------------------------------
// 4. EMBARCADOR (Inicializa a VM e passa o controle para o main.lua)
// ------------------------------------------------------------------------
void android_main(struct android_app* app) {
#ifndef MODO_EMBUTIDO
    android_asset_manager = app->activity->assetManager;
#endif

    // Inicializa o LuaJIT de forma pura (Sem criar janela gráfica no C)
    lua_State* L = luaL_newstate();
    luaL_openlibs(L); 

    // Configura e injeta o Buscador Unificado
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "searchers"); 
    lua_pushcfunction(L, custom_lua_searcher);
    
    for (int i = (int)lua_objlen(L, -2); i >= 2; i--) {
        lua_rawgeti(L, -2, i);
        lua_rawseti(L, -3, i + 1);
    }
    lua_rawseti(L, -2, 2);
    lua_pop(L, 2);

    // Passa a estrutura 'android_app' para o Lua caso seus bindings precisem dela futuramente
    // (Útil para capturar eventos nativos do Android direto no Lua se necessário)
    lua_pushlightuserdata(L, app);
    lua_setglobal(L, "__ANDROID_APP__");

    // --- DA O SINAL DE PARTIDA EXECUTANDO O "main.lua" ---
#ifdef MODO_EMBUTIDO
    // Em Release, roda o main.lua da memória RAM
    luaL_dobuffer(L, (const char*)game_main, game_main_len, "main.lua");
#else
    // Em Debug, abre e roda o main.lua solto da pasta Assets
    AAsset* main_asset = AAssetManager_open(android_asset_manager, "main.lua", AASSET_MODE_BUFFER);
    if (main_asset) {
        size_t size = AAsset_getLength(main_asset);
        char* buffer = (char*)malloc(size);
        AAsset_read(main_asset, buffer, size);
        AAsset_close(main_asset);
        
        luaL_dobuffer(L, buffer, size, "main.lua");
        free(buffer);
    }
#endif

    // Fecha a VM quando o loop da Raylib terminar lá no main.lua e o script encerrar
    lua_close(L);
}
