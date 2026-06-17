-- Carrega os bindings da Raylib e utilitários que estão protegidos no .so
local ray = require("raylib_ffi")
local utils = require("utils")

-- Inicializa a Raylib diretamente pelo Lua!
ray.InitWindow(0, 0, "Meu Jogo em Lua Puro")
ray.SetTargetFPS(60)

-- Loop principal controlado pelo Lua
while not ray.WindowShouldClose() do
    
    ray.BeginDrawing()
    ray.ClearBackground(ray.BLACK)

    -- Desenha usando a nossa lib embutida
    utils.desenharJanelaTexto("Ciclo controlado pelo Lua!", 40, 40, 22)

    ray.EndDrawing()
end

-- Finaliza a Raylib ao sair do loop
ray.CloseWindow()
