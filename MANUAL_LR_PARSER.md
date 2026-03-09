# Documentacion del Analizador Sintactico Ascendente Manual (LR)

## 1. Objetivo
Este documento describe el analizador sintactico ascendente manual implementado para el proyecto, siguiendo un enfoque LR codificado a mano (sin Bison/Yacc), usando:

- pila de estados (`std::stack<int>`)
- pila de simbolos AST (`std::stack<ASTNode*>`)
- entrada como lista enlazada de tokens (`Token* siguiente`)

El parser construye un AST cuyo nodo raiz es `ProgramNode`, que sirve como entrada para fases posteriores (por ejemplo, analisis semantico).

## 2. Archivos Relacionados
- `ManualLRParser.h`: parser ascendente manual, modo panico y conversion de tokens.
- `ControlFlowAST.h`: jerarquia de nodos AST para estructuras de control y programa.
- `ProgramParser.h`: adaptador `ManualProgramParser` para reutilizar el `Lexer` existente.
- `Token.h`: tipos de token usados por lexer/parser.
- `Lexer.h`: analizador lexico existente reutilizado.
- `manual_lr_demo.cpp`: ejemplo ejecutable de uso.

## 3. Entrada del Parser
El parser manual opera sobre una lista enlazada simple en `namespace manual_lr`:

```cpp
struct Token {
    int tipo;
    std::string valor;
    Token* siguiente;
    int linea;
    int columna;
};
```

### 3.1 Adaptacion desde el lexer existente
Se reutiliza `LinkedList<::Token>` del lexer y se convierte con:

- `manual_lr::buildTokenStream(LinkedList<::Token>&)`
- `manual_lr::freeTokenStream(Token*)`

Esto permite mantener el lexer actual sin reescribirlo.

## 4. AST Implementado
En `ControlFlowAST.h` se incluye la jerarquia:

- `ASTNode` (base existente)
- `StatementNode` (base para sentencias)
- `ProgramNode` (raiz: nombre de funcion + bloque principal)
- `BlockNode` (lista de sentencias)
- `AssignNode` (`id = expr`)
- `IfNode` (incluye `else if` y `else` opcionales)
- `WhileNode`
- `ForNode`
- `DoWhileNode`
- `TerminalNode` (nodo auxiliar para simbolos terminales shift-eados)

Cada nodo conserva metadatos relevantes (por ejemplo: identificador, condicion o expresion en formato crudo).

## 5. Gramatica Soportada
Subconjunto de C soportado por el parser actual:

```txt
Main            -> INT MAIN LPAREN RPAREN Bloque
Bloque          -> LBRACE ListaSentencias RBRACE
ListaSentencias -> Sentencia ListaSentencias | vacio
Sentencia       -> IfStmt
                | WhileStmt
                | ForStmt
                | DoWhileStmt
                | Asignacion SEMICOLON
IfStmt          -> IF LPAREN Exp RPAREN Bloque (ELSE IF ...)* (ELSE Bloque)?
```

Notas:
- `MAIN` se valida como `TOKEN_IDENTIFIER` con lexema `"main"`.
- El lexer actual no define `TOKEN_DO`; por eso `do` se acepta como `TOKEN_IDENTIFIER` con lexema `"do"`.
- `Exp` se captura como texto crudo (secuencia de tokens) hasta delimitador correspondiente.

## 6. Modelo LR Manual
El parser (`ManualLRParser`) implementa acciones LR por `switch(estadoActual)`:

- `shift(nextState)`: apila `TerminalNode`, apila estado, avanza token.
- `reduce`: combina simbolos apilados para construir no terminales AST.
- `accept`: estado final (`1000`) al construir `ProgramNode`.

### 6.1 Parser principal
Produccion central:

```txt
Main -> int main ( ) Bloque
```

Estados principales:
- `0`: espera `int`
- `1`: espera `main`
- `2`: espera `(`
- `3`: espera `)`
- `4`: parsea/reduce `Bloque`
- `1000`: `accept`

### 6.2 Bloques y sentencias
`parseBlockLR()` aplica:

```txt
Bloque -> { ListaSentencias }
```

Dentro del bloque, `parseStatementLR()` despacha a:
- `parseIfStatement()`
- `parseWhileStatement()`
- `parseForStatement()`
- `parseDoWhileStatement()`
- `parseAssignmentStatement()`

## 7. Modo Panico (Recuperacion de Errores)
Si el parser detecta estado o token invalido:

1. Reporta error con linea/columna/token.
2. Avanza en la lista enlazada hasta encontrar:
   - `;` o
   - `}` o
   - `EOF`
3. Ajusta pilas para intentar continuar analisis.

Esto evita abortar al primer error y permite recolectar mas diagnosticos.

## 8. API Principal
### 8.1 Uso directo del parser manual
```cpp
manual_lr::Token* head = manual_lr::buildTokenStream(tokens);
manual_lr::ManualLRParser parser(head);
ProgramNode* root = parser.parseProgram();
const std::vector<std::string>& errors = parser.getErrors();
manual_lr::freeTokenStream(head);
```

### 8.2 Uso recomendado (adaptador)
En `ProgramParser.h`:

```cpp
Lexer lexer(code);
ManualProgramParser parser(lexer);
ProgramNode* root = parser.parse();
const std::vector<std::string>& errors = parser.getErrors();
```

## 9. Salida del Parser
- Exito: retorna `ProgramNode*` (raiz AST).
- Con errores recuperables: puede retornar AST parcial + lista de errores.
- Error fatal: retorna `nullptr`.

Siempre se recomienda revisar `getErrors()` incluso si hay AST.

## 10. Ejemplo Minimo
Codigo fuente de entrada:

```c
int main() {
  x = 1;
  if (x > 0) { y = x; } else { y = 0; }
  while (x < 10) { x = x + 1; }
}
```

Salida esperada (resumen):

```txt
Program main
  Block
    Assign x = 1
    If (x > 0)
      Block
        Assign y = x
    Else
      Block
        Assign y = 0
    While (x < 10)
      Block
        Assign x = x + 1
```

## 11. Limitaciones Actuales
- No hay tabla LR generada formalmente; es LR manual dirigido por estados codificados.
- Las expresiones (`Exp`) no se parsean con precedencia completa en esta fase; se almacenan como texto crudo.
- `do` aun no existe como `TOKEN_DO` en lexer, se reconoce por lexema.
- No incluye validacion semantica (tipos, simbolos, etc.).

## 12. Recomendaciones de Evolucion
1. Agregar `TOKEN_DO` al lexer y keywords.
2. Expandir grammar de expresiones para construir AST expresional completo.
3. Formalizar tabla ACTION/GOTO documentada para mayor mantenibilidad.
4. Integrar pruebas unitarias por produccion y por recuperacion de errores.

## 13. Resumen
El analizador sintactico implementado cumple con:
- enfoque ascendente manual LR
- pilas de estados y simbolos AST
- gramatica de control de flujo del subconjunto C solicitado
- recuperacion de errores en modo panico
- retorno de `ProgramNode*` como raiz de AST

Con esto queda lista la base para conectar el analisis semantico sobre una estructura sintactica robusta.
