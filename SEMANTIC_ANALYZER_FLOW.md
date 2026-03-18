# Analizador Semantico: Flujo Completo y Guia de Uso

## 1. Objetivo
Este documento explica en detalle:

1. Que cambios se hicieron para separar responsabilidades entre lexico, sintactico y semantico.
2. Como fluye la informacion desde el codigo fuente hasta los errores semanticos.
3. Como usar el analizador semantico en el proyecto.
4. Que reglas valida hoy y cuales son sus limites actuales.

## 2. Cambios de arquitectura que se hicieron

### 2.1 Separacion de fases
Antes habia logica que pertenecia a semantica mezclada en parsing de expresiones/declaraciones.

Se reorganizo asi:

1. Lexer:
- Solo tokeniza entrada.
- No valida tipos ni simbolos.

2. Parser (Manual LR):
- Solo valida estructura gramatical.
- Construye AST con nodos de control de flujo y expresiones crudas en texto.
- No decide si una variable esta declarada o si tipos son compatibles.

3. SemanticAnalyzer:
- Recorre el AST construido por el parser.
- Mantiene tabla de simbolos por scopes.
- Reporta errores de semantica (declaracion, uso, tipos).

## 3. Archivos importantes

1. Lexer y tokens:
- Lexer.h
- Token.h

2. Parser manual LR y AST:
- ManualLRParser.h
- ControlFlowAST.h
- ProgramParser.h (adaptador ManualProgramParser)

3. Nueva fase semantica:
- SemanticAnalyzer.h

4. Demo integrada del flujo:
- manual_lr_demo.cpp

## 4. Flujo de ejecucion completo

## 4.1 Entrada de programa
En manual_lr_demo.cpp se define un string con codigo C-like:

- int main() { ... }
- declaraciones
- if/else
- while

## 4.2 Fase lexico
Se instancia Lexer con el codigo fuente.

El lexer produce tokens para:

- keywords (int, if, while...)
- identificadores
- literales (entero, float, char, string)
- operadores y delimitadores

## 4.3 Fase sintactica (ManualProgramParser + ManualLRParser)
ManualProgramParser convierte la salida del lexer a una lista enlazada manual de tokens (namespace manual_lr) y llama a parseProgram().

ManualLRParser:

1. Reconoce la forma global del programa: int main() { ... }
2. Dentro del bloque, parsea sentencias:
- DeclarationNode
- AssignNode
- IfNode
- WhileNode
- ForNode
- DoWhileNode
3. Guarda expresiones como texto crudo (conditionRaw, expressionRaw, initExpr, etc).
4. En errores sintacticos aplica modo panico para continuar y recolectar mas errores.

Nota importante:
- En collectUntilDelimiter() ahora se preservan comillas de literales char/string para no confundir semantica despues.

## 4.4 Fase semantica (SemanticAnalyzer)
Si parseo devuelve ProgramNode, se ejecuta semantic.analyze(program).

Internamente hace:

1. Limpia estado previo (errors y scopeStack).
2. Verifica que ProgramNode y mainBlock existan.
3. Recorre bloques y nodos recursivamente.
4. En cada scope crea/descarta tabla local (pushScope/popScope).
5. Acumula errores en vector<string> errors.

Al final:

- sin errores -> retorna true
- con errores -> retorna false y getErrors() contiene diagnosticos

## 5. Reglas semanticas actuales

### 5.1 Tabla de simbolos y scopes
Se maneja un stack de mapas:

- vector<map<string, string>> scopeStack

Cada entrada guarda:

- nombre variable -> tipo declarado

Lookup de variables:

- busca desde scope actual hacia arriba.
- permite sombreado entre bloques distintos.
- prohibe redeclaracion en el mismo scope.

### 5.2 Declaraciones
Para DeclarationNode:

1. Si tipo es void -> error.
2. Si nombre ya existe en scope actual -> error de redeclaracion.
3. Si hay inicializacion:
- valida identificadores usados en la expresion (deben estar declarados)
- valida compatibilidad de tipos (si expresion es simple inferible)

### 5.3 Asignaciones
Para AssignNode:

1. Lado izquierdo debe estar declarado.
2. Identificadores del lado derecho deben estar declarados.
3. Si tipo del lado derecho es inferible de forma simple, valida compatibilidad con tipo del lado izquierdo.

### 5.4 Condiciones y expresiones en control de flujo
Para if, else-if, while, do-while, for:

- valida que identificadores usados en condiciones/expresiones existan.
- para for init con declaracion (ej: int i = 0), declara i en scope del for.
- para for init tambien revisa compatibilidad de tipo cuando hay inicializacion simple.

### 5.5 Compatibilidad de tipos implementada
Reglas actuales en isTypeCompatible:

1. void nunca acepta asignacion.
2. mismo tipo -> permitido.
3. float <- int -> permitido.
4. cualquier otro cruce incompatible -> error.

Ejemplos con reglas actuales:

- int x = 1; -> OK
- float y = 1; -> OK
- int x = 1.5; -> Error
- int x = 'a'; -> Error
- int x = "hola"; -> Error
- char c = 'a'; -> OK

## 6. Correcciones importantes que se aplicaron al semantico

### 6.1 Falso positivo con literales
Problema que ocurria:

- Expresiones con texto dentro de comillas podian ser tomadas como identificadores.

Ejemplo:

- int x = 'ana';

Sin filtro, el analizador podia detectar a como variable no declarada.

Solucion aplicada:

1. extractIdentifiers() ignora contenido entre comillas simples y dobles.
2. checkExpressionIdentifiers() deduplica identificadores para no repetir el mismo error varias veces por expresion.

### 6.2 Preservacion de literales desde parser
Problema:

- Si el parser pierde delimitadores de comillas, semantica no distingue literal vs identificador.

Solucion:

- En ManualLRParser::collectUntilDelimiter() se reinyectan comillas para TOKEN_STRING y TOKEN_CHAR_LITERAL al reconstruir expresion cruda.

## 7. Inferencia simple de tipo en expresiones

El analizador infiere tipo solo en expresiones simples:

1. Literales enteros -> int
2. Literales flotantes -> float
3. Literales char validos -> char
4. Literales string o char invalido/multicaracter -> string
5. Identificador simple -> tipo declarado en tabla
6. Expresiones complejas (x + y, llamadas, etc) -> <unknown>

Comportamiento con <unknown>:

- No dispara error de incompatibilidad por tipo.
- Si mantiene chequeo de variables no declaradas dentro de la expresion.

## 8. Como usar el analizador semantico

## 8.1 Uso minimo en codigo
Patron recomendado:

1. Parsear
2. Imprimir errores sintacticos
3. Si hay AST, correr semantico
4. Imprimir errores semanticos
5. Retornar codigo de salida segun errores

En manual_lr_demo.cpp ya esta integrado exactamente asi.

## 8.2 Compilar y ejecutar demo
En PowerShell dentro del repo:

1. g++ .\manual_lr_demo.cpp -o .\manual_lr_demo.exe
2. .\manual_lr_demo.exe

Resultado esperado:

- Si no hay errores semanticos: mensaje de exito + AST
- Si hay errores semanticos: listado [SemanticError] + AST + exit code 1

## 9. Ejemplos de diagnostico

## 9.1 Variable no declarada
Entrada:

- y = x;

Sin declarar y previamente, se reporta:

- [SemanticError] Variable no declarada 'y' en asignacion.

## 9.2 Incompatibilidad de tipos
Entrada:

- int x = "hola";

Se reporta:

- [SemanticError] Incompatibilidad de tipos en inicializacion de 'x': no se puede asignar 'string' a 'int'.

## 9.3 Redeclaracion
Entrada en mismo bloque:

- int x = 1;
- int x = 2;

Se reporta redeclaracion de x en declaracion.