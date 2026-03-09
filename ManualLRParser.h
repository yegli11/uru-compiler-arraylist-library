#ifndef MANUAL_LR_PARSER_H
#define MANUAL_LR_PARSER_H

#include "ControlFlowAST.h"
#include "Token.h"
#include "LinkedList.h"
#include <stack>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

namespace manual_lr {

// Entrada pedida: lista enlazada de tokens con next explicito.
struct Token {
    int tipo;
    std::string valor;
    Token* siguiente;
    int linea;
    int columna;

    Token(int t = TOKEN_EOF,
          const std::string& v = "",
          Token* sig = nullptr,
          int ln = 0,
          int col = 0)
        : tipo(t), valor(v), siguiente(sig), linea(ln), columna(col) {}
};

// Convierte la salida del lexer existente a la lista enlazada manual requerida.
inline Token* buildTokenStream(LinkedList<::Token>& tokenList) {
    Token* head = nullptr;
    Token* tail = nullptr;

    for (int i = 0; i < tokenList.size(); ++i) {
        ::Token tk = tokenList.get(i);
        Token* node = new Token(static_cast<int>(tk.getType()),
                                tk.getLexeme(),
                                nullptr,
                                tk.getLine(),
                                tk.getColumn());
        if (!head) {
            head = node;
            tail = node;
        } else {
            tail->siguiente = node;
            tail = node;
        }
    }

    if (!tail) {
        head = new Token(TOKEN_EOF, "", nullptr, 0, 0);
    }

    return head;
}

inline void freeTokenStream(Token* head) {
    while (head) {
        Token* next = head->siguiente;
        delete head;
        head = next;
    }
}

class ManualLRParser {
public:
    explicit ManualLRParser(Token* tokenHead)
        : head(tokenHead), current(tokenHead), hadFatalError(false) {
        stateStack.push(0);
    }

    ProgramNode* parseProgram() {
        reset();

        // LR(0) simplificado para: Main -> INT MAIN LPAREN RPAREN Bloque
        while (!hadFatalError) {
            if (stateStack.empty()) {
                reportError("Pila de estados vacia");
                return nullptr;
            }

            int state = stateStack.top();
            switch (state) {
                case 0:
                    // Shift esperado: INT
                    if (lookaheadType() == TOKEN_INT) {
                        shift(1);
                    } else {
                        reportError("Se esperaba 'int' al inicio del programa");
                        panicRecover();
                    }
                    break;

                case 1:
                    // Shift esperado: identificador main
                    if (lookaheadType() == TOKEN_IDENTIFIER && lookaheadLexeme() == "main") {
                        shift(2);
                    } else {
                        reportError("Se esperaba identificador 'main'");
                        panicRecover();
                    }
                    break;

                case 2:
                    // Shift esperado: (
                    if (lookaheadType() == TOKEN_LPAREN) {
                        shift(3);
                    } else {
                        reportError("Se esperaba '('");
                        panicRecover();
                    }
                    break;

                case 3:
                    // Shift esperado: )
                    if (lookaheadType() == TOKEN_RPAREN) {
                        shift(4);
                    } else {
                        reportError("Se esperaba ')'");
                        panicRecover();
                    }
                    break;

                case 4: {
                    // Reduce esperado: Bloque y luego Main/Program.
                    BlockNode* block = parseBlockLR();
                    if (!block) {
                        if (!hadFatalError) {
                            panicRecover();
                        }
                        break;
                    }

                    symbolStack.push(block);
                    stateStack.push(5); // goto ficticio para Main completo
                    reduceToProgram();
                    break;
                }

                case 1000: {
                    // Accept
                    if (lookaheadType() != TOKEN_EOF) {
                        reportError("Tokens extra despues de cerrar el programa");
                    }

                    if (symbolStack.empty()) {
                        reportError("No se pudo construir ProgramNode");
                        return nullptr;
                    }

                    ASTNode* root = symbolStack.top();
                    ProgramNode* program = dynamic_cast<ProgramNode*>(root);
                    if (!program) {
                        reportError("La raiz construida no es ProgramNode");
                        return nullptr;
                    }
                    return program;
                }

                default:
                    reportError("Estado invalido en parser principal: " + toString(state));
                    panicRecover();
                    break;
            }
        }

        return nullptr;
    }

    const std::vector<std::string>& getErrors() const {
        return errors;
    }

private:
    Token* head;
    Token* current;
    std::stack<int> stateStack;
    std::stack<ASTNode*> symbolStack;
    std::vector<std::string> errors;
    bool hadFatalError;

    void reset() {
        current = head;
        while (!stateStack.empty()) {
            stateStack.pop();
        }
        while (!symbolStack.empty()) {
            ASTNode* n = symbolStack.top();
            symbolStack.pop();
            // Evitamos leaks de nodos terminales de shift no reducidos.
            delete n;
        }
        errors.clear();
        hadFatalError = false;
        stateStack.push(0);
    }

    static std::string toString(int value) {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }

    int lookaheadType() const {
        return current ? current->tipo : TOKEN_EOF;
    }

    std::string lookaheadLexeme() const {
        return current ? current->valor : "";
    }

    void advanceToken() {
        if (current) {
            current = current->siguiente;
        }
    }

    // Accion Shift: apila simbolo terminal y avanza entrada.
    void shift(int nextState) {
        TerminalNode* terminal = new TerminalNode(lookaheadType(), lookaheadLexeme());
        symbolStack.push(terminal);
        stateStack.push(nextState);
        advanceToken();
    }

    // Reduce final Main -> ... y Program -> Main.
    void reduceToProgram() {
        if (symbolStack.size() < 5) {
            reportError("No hay suficientes simbolos para reducir Program");
            hadFatalError = true;
            return;
        }

        ASTNode* blockSym = symbolStack.top();
        symbolStack.pop();

        ASTNode* rp = symbolStack.top();
        symbolStack.pop();
        ASTNode* lp = symbolStack.top();
        symbolStack.pop();
        ASTNode* mainId = symbolStack.top();
        symbolStack.pop();
        ASTNode* intKw = symbolStack.top();
        symbolStack.pop();

        delete rp;
        delete lp;
        delete intKw;

        TerminalNode* mainTerminal = dynamic_cast<TerminalNode*>(mainId);
        if (!mainTerminal) {
            delete mainId;
            delete blockSym;
            reportError("No se pudo recuperar nombre de funcion principal");
            hadFatalError = true;
            return;
        }

        BlockNode* block = dynamic_cast<BlockNode*>(blockSym);
        if (!block) {
            delete mainId;
            delete blockSym;
            reportError("No se pudo reducir Bloque a Program");
            hadFatalError = true;
            return;
        }

        ProgramNode* program = new ProgramNode(mainTerminal->lexeme, block);
        delete mainId;

        // Limpia estados previos y marca estado Accept.
        while (!stateStack.empty()) {
            stateStack.pop();
        }
        stateStack.push(1000);
        symbolStack.push(program);
    }

    // Bloque -> LBRACE ListaSentencias RBRACE
    BlockNode* parseBlockLR() {
        std::stack<int> localState;
        localState.push(10);

        BlockNode* block = new BlockNode();

        while (!localState.empty()) {
            int state = localState.top();

            switch (state) {
                case 10:
                    if (lookaheadType() == TOKEN_LBRACE) {
                        shift(10);
                        localState.push(11);
                    } else {
                        reportError("Se esperaba '{' para abrir bloque");
                        delete block;
                        return nullptr;
                    }
                    break;

                case 11:
                    // ListaSentencias -> vacio | Sentencia ListaSentencias
                    if (lookaheadType() == TOKEN_RBRACE) {
                        shift(11);
                        localState.push(12);
                    } else if (lookaheadType() == TOKEN_EOF) {
                        reportError("EOF dentro de bloque sin '}'");
                        delete block;
                        return nullptr;
                    } else {
                        ASTNode* stmt = parseStatementLR();
                        if (stmt) {
                            block->addStatement(stmt);
                        } else {
                            // Error no fatal: se intenta recuperar y continuar lista.
                            panicRecover();
                            if (lookaheadType() == TOKEN_EOF) {
                                reportError("No fue posible recuperar dentro del bloque");
                                delete block;
                                return nullptr;
                            }
                            if (lookaheadType() == TOKEN_RBRACE) {
                                // Permite salir del bloque despues de recuperacion.
                                continue;
                            }
                        }
                    }
                    break;

                case 12:
                    // Reduce Bloque completado.
                    // Limpia los terminales '{' y '}' que entraron por shift local.
                    pruneTerminalsFromShift();
                    localState.pop();
                    localState.pop();
                    localState.pop();
                    return block;

                default:
                    reportError("Estado invalido en parseBlockLR: " + toString(state));
                    delete block;
                    return nullptr;
            }
        }

        delete block;
        return nullptr;
    }

    ASTNode* parseStatementLR() {
        switch (lookaheadType()) {
            case TOKEN_IF:
                return parseIfStatement();
            case TOKEN_WHILE:
                return parseWhileStatement();
            case TOKEN_FOR:
                return parseForStatement();
            case TOKEN_IDENTIFIER:
                return parseAssignmentStatement();
            default:
                reportError("Sentencia no reconocida con token: " + lookaheadLexeme());
                return nullptr;
        }
    }

    AssignNode* parseAssignmentStatement() {
        // Sentencia -> Asignacion ;
        if (lookaheadType() != TOKEN_IDENTIFIER) {
            reportError("Se esperaba identificador al inicio de asignacion");
            return nullptr;
        }

        std::string id = lookaheadLexeme();
        shift(stateStack.top());

        if (lookaheadType() != TOKEN_ASSIGN) {
            reportError("Se esperaba '=' en asignacion");
            return nullptr;
        }
        shift(stateStack.top());

        std::string expression = collectUntilDelimiter(TOKEN_SEMICOLON);
        if (lookaheadType() != TOKEN_SEMICOLON) {
            reportError("Se esperaba ';' al final de asignacion");
            return nullptr;
        }
        shift(stateStack.top());

        pruneTerminalsFromShift();
        return new AssignNode(id, expression);
    }

    WhileNode* parseWhileStatement() {
        shift(stateStack.top());

        std::string condition = parseParenthesizedExpression();
        if (condition.empty() && lookaheadType() == TOKEN_EOF) {
            return nullptr;
        }

        BlockNode* body = parseBlockLR();
        if (!body) {
            return nullptr;
        }

        pruneTerminalsFromShift();
        return new WhileNode(condition, body);
    }

    ForNode* parseForStatement() {
        shift(stateStack.top());

        if (lookaheadType() != TOKEN_LPAREN) {
            reportError("Se esperaba '(' despues de for");
            return nullptr;
        }
        shift(stateStack.top());

        std::string init = collectUntilDelimiter(TOKEN_SEMICOLON);
        if (lookaheadType() != TOKEN_SEMICOLON) {
            reportError("Se esperaba ';' en for (init)");
            return nullptr;
        }
        shift(stateStack.top());

        std::string cond = collectUntilDelimiter(TOKEN_SEMICOLON);
        if (lookaheadType() != TOKEN_SEMICOLON) {
            reportError("Se esperaba ';' en for (condicion)");
            return nullptr;
        }
        shift(stateStack.top());

        std::string update = collectUntilDelimiter(TOKEN_RPAREN);
        if (lookaheadType() != TOKEN_RPAREN) {
            reportError("Se esperaba ')' en for");
            return nullptr;
        }
        shift(stateStack.top());

        BlockNode* body = parseBlockLR();
        if (!body) {
            return nullptr;
        }

        pruneTerminalsFromShift();
        return new ForNode(init, cond, update, body);
    }

    DoWhileNode* parseDoWhileStatement() {
        // Soporta TOKEN_IDENTIFIER con lexema "do" si el lexer aun no tiene TOKEN_DO.
        shift(stateStack.top());

        BlockNode* body = parseBlockLR();
        if (!body) {
            return nullptr;
        }

        if (!(lookaheadType() == TOKEN_WHILE ||
              (lookaheadType() == TOKEN_IDENTIFIER && lookaheadLexeme() == "while"))) {
            reportError("Se esperaba while despues de do { ... }");
            delete body;
            return nullptr;
        }
        shift(stateStack.top());

        std::string cond = parseParenthesizedExpression();
        if (lookaheadType() != TOKEN_SEMICOLON) {
            reportError("Se esperaba ';' al final de do-while");
            delete body;
            return nullptr;
        }
        shift(stateStack.top());

        pruneTerminalsFromShift();
        return new DoWhileNode(body, cond);
    }

    IfNode* parseIfStatement() {
        shift(stateStack.top());

        std::string cond = parseParenthesizedExpression();
        if (cond.empty() && lookaheadType() == TOKEN_EOF) {
            return nullptr;
        }

        BlockNode* thenBlock = parseBlockLR();
        if (!thenBlock) {
            return nullptr;
        }

        IfNode* ifNode = new IfNode(cond, thenBlock);

        // Soporte opcional para else-if y else.
        while (lookaheadType() == TOKEN_ELSE) {
            shift(stateStack.top());

            if (lookaheadType() == TOKEN_IF) {
                shift(stateStack.top());
                std::string elseIfCond = parseParenthesizedExpression();
                BlockNode* elseIfBlock = parseBlockLR();
                if (!elseIfBlock) {
                    delete ifNode;
                    return nullptr;
                }
                ifNode->addElseIf(elseIfCond, elseIfBlock);
                continue;
            }

            BlockNode* elseBlock = parseBlockLR();
            if (!elseBlock) {
                delete ifNode;
                return nullptr;
            }
            ifNode->setElseBlock(elseBlock);
            break;
        }

        pruneTerminalsFromShift();
        return ifNode;
    }

    std::string parseParenthesizedExpression() {
        if (lookaheadType() != TOKEN_LPAREN) {
            reportError("Se esperaba '('");
            return "";
        }
        shift(stateStack.top());

        std::string expr = collectUntilDelimiter(TOKEN_RPAREN);
        if (lookaheadType() != TOKEN_RPAREN) {
            reportError("Se esperaba ')'");
            return "";
        }
        shift(stateStack.top());
        return expr;
    }

    std::string collectUntilDelimiter(int delimiterType) {
        std::ostringstream oss;
        int nestedParens = 0;

        while (current) {
            int type = lookaheadType();
            if (type == TOKEN_LPAREN) {
                nestedParens++;
            } else if (type == TOKEN_RPAREN && nestedParens > 0) {
                nestedParens--;
            }

            if (type == delimiterType && nestedParens == 0) {
                break;
            }
            if (type == TOKEN_EOF) {
                break;
            }

            if (!lookaheadLexeme().empty()) {
                if (oss.tellp() > 0) {
                    oss << " ";
                }
                oss << lookaheadLexeme();
            }
            shift(stateStack.top());
        }

        return oss.str();
    }

    // Elimina terminales que solo se apilaron por las acciones shift internas.
    void pruneTerminalsFromShift() {
        while (!symbolStack.empty()) {
            TerminalNode* terminal = dynamic_cast<TerminalNode*>(symbolStack.top());
            if (!terminal) {
                break;
            }
            delete terminal;
            symbolStack.pop();
            if (stateStack.size() > 1) {
                stateStack.pop();
            } else {
                break;
            }
        }
    }

    void panicRecover() {
        // Modo panico: salta hasta ';' o '}' para intentar continuar.
        while (current &&
               lookaheadType() != TOKEN_SEMICOLON &&
               lookaheadType() != TOKEN_RBRACE &&
               lookaheadType() != TOKEN_EOF) {
            advanceToken();
        }

        if (lookaheadType() == TOKEN_SEMICOLON) {
            advanceToken();
        }

        // Ajuste de pila: mantener solo estado raiz y limpiar simbolos terminales.
        while (stateStack.size() > 1) {
            stateStack.pop();
        }
        pruneTerminalsFromShift();

        if (!current || lookaheadType() == TOKEN_EOF) {
            hadFatalError = true;
        }
    }

    void reportError(const std::string& message) {
        std::ostringstream oss;
        oss << "[ParserError] " << message;
        if (current) {
            oss << " en linea " << current->linea
                << ", columna " << current->columna
                << " (token='" << current->valor << "')";
        }
        errors.push_back(oss.str());
    }
};

} // namespace manual_lr

#endif
