-- Carrega o módulo FFI do LuaJIT
local ffi = require("ffi")

-- 1. Declaramos para o LuaJIT as funções e estruturas da Raylib que queremos usar.
-- Isso é idêntico ao que está no 'raylib.h', mas em formato string para o Lua.
ffi.cdef([[
    // Estrutura de cor da Raylib
    typedef struct Color {
        unsigned char r;
        unsigned char g;
        unsigned char b;
        unsigned char a;
    } Color;

    // Funções da Raylib que vamos usar no teste
    void DrawText(const char *text, int posX, int posY, int fontSize, Color color);
    void DrawRectangle(int posX, int posY, int width, int height, Color color);
]])

-- 2. Criamos uma tabela para guardar nossas funções utilitárias
local utils = {}

-- 3. Criamos uma função no Lua que desenha uma caixa de texto estilizada usando a Raylib
function utils.desenharJanelaTexto(mensagem, x, y, tamanhoFonte)
    -- Definição de cores usando a estrutura que declaramos acima
    local corFundo = ffi.new("Color", 30, 30, 30, 220)  -- Cinza escuro translúcido
    local corTexto = ffi.new("Color", 255, 255, 255, 255) -- Branco

    -- Calcula uma largura aproximada para o fundo baseado no tamanho do texto
    local larguraFundo = #mensagem * (tamanhoFonte * 0.6) + 20
    local alturaFundo = tamanhoFonte + 20

    -- Chama as funções nativas da Raylib diretamente do Lua!
    ffi.C.DrawRectangle(x - 10, y - 10, larguraFundo, alturaFundo, corFundo)
    ffi.C.DrawText(mensagem, x, y, tamanhoFonte, corTexto)
end

-- Retorna a tabela para que quem use o 'require("utils")' tenha acesso a ela
return utils
