# Script para converter texto/lua em array hexadecimal de C
file(READ ${FILE_IN} CONTENTS HEX)
string(REGEX MATCHALL "([0-9a-f][0-9a-f])" HEX_BYTES "${CONTENTS}")
string(REGEX REPLACE ";" ", 0x" HEX_COMMA "${HEX_BYTES}")
set(HEX_FINAL "0x${HEX_COMMA}")

file(WRITE ${FILE_OUT} "const unsigned char ${VAR_NAME}[] = { ${HEX_FINAL}, 0x00 };\nconst unsigned int ${VAR_NAME}_len = sizeof(${VAR_NAME}) - 1;\n")
