# Lê o arquivo Lua de entrada e converte seu conteúdo textual em uma string Hexadecimal pura
file(READ ${FILE_IN} CONTENTS HEX)

# Divide a string Hex em pares de bytes individuais
string(REGEX MATCHALL "([0-9a-f][0-9a-f])" HEX_BYTES "${CONTENTS}")

# Substitui os separadores por formatação de Array em C (adiciona '0x')
string(REGEX REPLACE ";" ", 0x" HEX_COMMA "${HEX_BYTES}")
set(HEX_FINAL "0x${HEX_COMMA}")

# Escreve o arquivo header .h final contendo o array de bytes e seu respectivo tamanho
file(WRITE ${FILE_OUT} "const unsigned char ${VAR_NAME}[] = { ${HEX_FINAL}, 0x00 };\nconst unsigned int ${VAR_NAME}_len = sizeof(${VAR_NAME}) - 1;\n")
