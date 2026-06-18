/*******************************************************************************************
*
*   Raymob + LuaJIT + lua-libs + Jogo Embutido
*
*   Carrega lua-libs (bytecode embutido) + jogo (bytecode embutido OU assets)
*
********************************************************************************************/

#include "raymob.h"
#include "luajit.h"
#include "lua.h"
#include "lauxlib.h"

// ===========================================================================
// IMPORTAR CABEÇALHOS
// ===========================================================================
#include "lua-libs.h"         // ← lua-libs (utils.lua, game.lua, scene.lua)

// ===========================================================================
// ADICIONAR: Só incluir game-embedded.h quando EMBED_GAME=1
// ===========================================================================
#ifdef EMBED_GAME
#include "game-embedded.h"    // ← jogo embutido (só existe em Release)
#endif

#include <android/log.h>

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "Raymob", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "Raymob", __VA_ARGS__)

// ===========================================================================
// ESTADO LUA
// ===========================================================================
static lua_State *L = NULL;

// ===========================================================================
// CARREGAR LUA-LIBS (usar variáveis do cabeçalho lua-libs.h)
// ===========================================================================

static void load_lua_libs(lua_State *L) {
    LOGD("Carregando lua-libs...");
    
    // utils.lua (variável: lib_utils)
    #ifdef lib_utils
    if (luaL_loadbuffer(L, lib_utils, lib_utils_len, "utils") == 0) {
        if (lua_pcall(L, 0, 0, 0) == 0) {
            LOGD("✓ utils.loaded");
        } else {
            LOGE("✗ utils error: %s", lua_tostring(L, -1));
        }
    }
    #endif
    
    // game.lua (variável: lib_game)
    #ifdef lib_game
    if (luaL_loadbuffer(L, lib_game, lib_game_len, "game") == 0) {
        if (lua_pcall(L, 0, 0, 0) == 0) {
            LOGD("✓ game.loaded");
        } else {
            LOGE("✗ game error: %s", lua_tostring(L, -1));
        }
    }
    #endif
    
    // scene.lua (variável: lib_scene)
    #ifdef lib_scene
    if (luaL_loadbuffer(L, lib_scene, lib_scene_len, "scene") == 0) {
        if (lua_pcall(L, 0, 0, 0) == 0) {
            LOGD("✓ scene.loaded");
        } else {
            LOGE("✗ scene error: %s", lua_tostring(L, -1));
        }
    }
    #endif
}

// ===========================================================================
// CARREGAR JOGO (EMBED_GAME=1 → bytecode embutido, EMBED_GAME=0 → assets)
// ===========================================================================

static void load_game(lua_State *L) {
    #ifdef EMBED_GAME
    LOGD("Carregando jogo EMBUTIDO (bytecode)...");
    
    // assets/jogo/main.lua (variável: assets_jogo_main)
    #ifdef assets_jogo_main
    if (luaL_loadbuffer(L, assets_jogo_main, assets_jogo_main_len, "main") == 0) {
        if (lua_pcall(L, 0, 0, 0) == 0) {
            LOGD("✓ main.loaded (embedded)");
        } else {
            LOGE("✗ main error: %s", lua_tostring(L, -1));
        }
    }
    #endif
    
    // assets/jogo/level.lua (variável: assets_jogo_level)
    #ifdef assets_jogo_level
    if (luaL_loadbuffer(L, assets_jogo_level, assets_jogo_level_len, "level") == 0) {
        if (lua_pcall(L, 0, 0, 0) == 0) {
            LOGD("✓ level.loaded (embedded)");
        } else {
            LOGE("✗ level error: %s", lua_tostring(L, -1));
        }
    }
    #endif
    
    // assets/jogo/enemy.lua (variável: assets_jogo_enemy)
    #ifdef assets_jogo_enemy
    if (luaL_loadbuffer(L, assets_jogo_enemy, assets_jogo_enemy_len, "enemy") == 0) {
        if (lua_pcall(L, 0, 0, 0) == 0) {
            LOGD("✓ enemy.loaded (embedded)");
        } else {
            LOGE("✗ enemy error: %s", lua_tostring(L, -1));
        }
    }
    #endif
    
    #else
    LOGD("Carregando jogo dos ASSETS...");
    
    // assets/jogo/main.lua (assets)
    if (luaL_loadfile(L, "assets/jogo/main.lua") == 0) {
        if (lua_pcall(L, 0, 0, 0) == 0) {
            LOGD("✓ main.loaded (assets)");
        } else {
            LOGE("✗ main error: %s", lua_tostring(L, -1));
        }
    } else {
        LOGE("✗ assets/jogo/main.lua não encontrado!");
    }
    #endif
}

// ===========================================================================
// CALLBACKS DO RAYMOB (Init, Update, Render, Shutdown)
// ===========================================================================

void GameInit(void) {
    LOGD("=== Game Init ===");
    
    // Criar estado Lua
    L = luaL_newstate();
    if (!L) {
        LOGE("✗ Failed to create Lua state!");
        return;
    }
    
    // Open Lua libs
    luaopen_base(L);
    luaopen_package(L);
    luaopen_io(L);
    luaopen_os(L);
    luaopen_string(L);
    luaopen_math(L);
    luaopen_table(L);
    
    // Load lua-libs
    load_lua_libs(L);
    
    // Load game
    load_game(L);
    
    // Init Raylib
    LOGD("InitWindow...");
    InitWindow(0, 0, "Raymob Game");
    SetTargetFPS(60);
}

void GameUpdate(void) {
    if (!L) return;
    
    LOGD("Game Update");
    
    // Call update() from game (se existe)
    lua_getglobal(L, "update");
    if (lua_isfunction(L, -1)) {
        lua_pcall(L, 0, 0, 0);
    }
}

void GameRender(void) {
    if (!L) return;
    
    LOGD("Game Render");
    
    // Call render() from game (se existe)
    lua_getglobal(L, "render");
    if (lua_isfunction(L, -1)) {
        lua_pcall(L, 0, 0, 0);
    }
    
    // Draw FPS
    DrawFPS(10, 10);
}

void GameShutdown(void) {
    LOGD("=== Game Shutdown ===");
    
    if (L) {
        lua_close(L);
        L = NULL;
    }
    
    CloseWindow();
}

// ===========================================================================
// NATIVE ACTIVITY CALLBACKS (Raymob)
// ===========================================================================

void ANativeActivity_onCreate(ANativeActivity *activity, void *args, size_t len) {
    LOGD("ANativeActivity_onCreate");
    
    activity->callbacks->OnInit = GameInit;
    activity->callbacks->OnUpdate = GameUpdate;
    activity->callbacks->OnRender = GameRender;
    activity->callbacks->OnShutdown = GameShutdown;
}