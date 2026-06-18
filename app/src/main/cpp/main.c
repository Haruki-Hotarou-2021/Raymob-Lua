#include <raymob.h>
#include <android/log.h>
#include <stdlib.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <luajit.h>

// Declaração do entrypoint do Raymob para registrar as funções no Lua
// Ajuste o nome da função se ela for nomeada diferentemente no código do Raymob
extern int luaopen_raymob(lua_State* L);

// Inclusão dos headers Lua autogerados pelo pipeline do CMake
#include "lib_libs.h"  // Gerado a partir de lua-libs/libs.lua -> mapeia para lua_lib_libs

// Módulos Core do JIT gerados automaticamente
#include "jit_bc.h"
#include "jit_bcsave.h"
#include "jit_dis_arm64.h" // Modifique ou adicione dependendo de quais gerou, ex: dis_arm, dis_x86
#include "jit_dump.h"
#include "jit_p.h"
#include "jit_v.h"
#include "jit_vmdef.h"

// Função utilitária para injetar os binários embutidos diretamente no 'package.preload' do Lua
// Isso faz com que chamadas de 'require "modulo"' busquem direto na memória do binário
void preload_lua_module(lua_State* L, const char* name, const unsigned char* buffer, unsigned int size) {
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    
    if (luaL_loadbuffer(L, (const char*)buffer, size, name) == 0) {
        lua_setfield(L, -2, name);
    } else {
        TraceLog(LOG_ERROR, "Falha ao pré-carregar módulo embutido: %s", name);
    }
    
    lua_pop(L, 2); // Limpa a pilha (preload e package)
}

// Ponto de entrada nativo do Android (Gerenciado pelo native_app_glue)
void android_main(struct android_app* app) {
    // Registra a infraestrutura básica de ciclo de vida do Android exigida pela Raylib
    SetCallbackStructure(app);

    // Inicializa o Estado do LuaJIT
    lua_State* L = luaL_newstate();
    if (!L) {
        TraceLog(LOG_ERROR, "Não foi possível inicializar o motor LuaJIT.");
        return;
    }

    // Carrega as bibliotecas nativas e padrão do Lua
    luaL_openlibs(L);

    // Registra o Raymob nativamente no ecossistema Lua
    // Permite que seus scripts usem require("raymob") ou tenham acesso às globais da engine
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "loaded");
    luaopen_raymob(L);
    lua_setfield(L, -2, "raymob");
    lua_pop(L, 2);

    // Mapeia os módulos internos do JIT para que o compilador funcione perfeitamente
    preload_lua_module(L, "jit.bc", lua_jit_bc, lua_jit_bc_len);
    preload_lua_module(L, "jit.bcsave", lua_jit_bcsave, lua_jit_bcsave_len);
    preload_lua_module(L, "jit.dump", lua_jit_dump, lua_jit_dump_len);
    preload_lua_module(L, "jit.p", lua_jit_p, lua_jit_p_len);
    preload_lua_module(L, "jit.v", lua_jit_v, lua_jit_v_len);
    preload_lua_module(L, "jit.vmdef", lua_jit_vmdef, lua_jit_vmdef_len);

    // Pré-carrega o seu ponto de entrada "libs"
    preload_lua_module(L, "libs", lua_lib_libs, lua_lib_libs_len);

    // Executa o ponto de entrada principal (libs.lua)
    // O seu libs.lua cuidará de gerenciar o loop ou chamar sub-módulos via FFI
    lua_getglobal(L, "require");
    lua_pushstring(L, "libs");
    
    if (lua_pcall(L, 1, 0, 0) != 0) {
        TraceLog(LOG_ERROR, "ERRO LUA: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
    }

    // Fecha o estado de execução do Lua e limpa a memória ao encerrar o app
    lua_close(L);
}
