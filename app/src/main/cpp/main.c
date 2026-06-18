#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

// Arquivos gerados pelo CMake na pasta build
#include "generated_lua/my_lib.h" 

void carregar_modulo_interno(lua_State* L) {
    // Carrega o buffer embutido direto na memória do interpretador
    if (luaL_loadbuffer(L, (const char*)my_lib, my_lib_len, "my_lib") == 0) {
        lua_pcall(L, 0, 0, 0); // Executa o script embutido para registrá-lo
    }
}
