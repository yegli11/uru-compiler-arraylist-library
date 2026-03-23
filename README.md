# uru-compiler-arraylist-library

Proyecto de compilador educativo en C++ con estas fases:

1. Analisis lexico
2. Analisis sintactico (parser LR manual)
3. Analisis semantico
4. Transpilacion a TypeScript

## Flujo actual

El demo principal para el pipeline completo esta en `manual_lr_demo.cpp`.

Cuando no hay errores sintacticos ni semanticos, el programa:

1. Imprime el AST
2. Genera codigo TypeScript equivalente
3. Escribe la salida en `transpiled_output.ts`

## Compilacion rapida (g++)

```bash
g++ -std=c++11 manual_lr_demo.cpp -o manual_lr_demo
./manual_lr_demo
```