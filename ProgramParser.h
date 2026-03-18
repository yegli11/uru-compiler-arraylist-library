#ifndef PROGRAM_PARSER_H
#define PROGRAM_PARSER_H

#include "Lexer.h"
#include "ExpressionParser.h"
#include "ASTNode.h"
#include "Token.h"
#include "ManualLRParser.h"
#include <stdexcept>
#include <vector>

class ProgramParser {
public:
    ProgramParser(Lexer& lexer) : lexer(lexer) {}

    ASTNode* parse() {
        ASTNode* tree = nullptr;

        while (true) {
            Token t = lexer.peekNextToken();

            if (t.getType() == TOKEN_EOF) {
                break;
            }

            if (t.getType() == TOKEN_INT ||
                t.getType() == TOKEN_FLOAT ||
                t.getType() == TOKEN_CHAR ||
                t.getType() == TOKEN_VOID) {
                lexer.getNextToken(); 

                Token id = lexer.getNextToken();
                if (id.getType() != TOKEN_IDENTIFIER)
                    throw std::runtime_error("Se esperaba identificador después del tipo");

                Token next = lexer.peekNextToken();
                if (next.getType() == TOKEN_ASSIGN) {
                    lexer.getNextToken();

                    Token value = lexer.getNextToken();
                    if (value.getType() != TOKEN_INTEGER &&
                        value.getType() != TOKEN_FLOAT_NUM &&
                        value.getType() != TOKEN_IDENTIFIER) {
                        throw std::runtime_error("Valor inválido en declaración: " + value.getLexeme());
                    }
                }
                else if (next.getType() != TOKEN_SEMICOLON) {
                    throw std::runtime_error("Se esperaba '=' o ';' en declaración");
                }

                Token semi = lexer.getNextToken();
                if (semi.getType() != TOKEN_SEMICOLON)
                    throw std::runtime_error("Se esperaba ';' después de declaración");
            }
      
            else if (t.getType() == TOKEN_IDENTIFIER) {
                Token id = lexer.getNextToken();

                Token assign = lexer.getNextToken();
                if (assign.getType() != TOKEN_ASSIGN)
                    throw std::runtime_error("Se esperaba '=' en asignación");

                Token value = lexer.getNextToken();
                if (value.getType() != TOKEN_INTEGER &&
                    value.getType() != TOKEN_FLOAT_NUM &&
                    value.getType() != TOKEN_IDENTIFIER) {
                    throw std::runtime_error("Valor inválido en asignación: " + value.getLexeme());
                }

                Token semi = lexer.getNextToken();
                if (semi.getType() != TOKEN_SEMICOLON)
                    throw std::runtime_error("Se esperaba ';' después de asignación");
            }
                    
            else {
                ExpressionParser exprParser(lexer);
                tree = exprParser.parseExpression();
                break;
            }
        }

        return tree;
    }

private:
    Lexer& lexer;
};

// Adaptador para usar el analizador sintactico ascendente manual.
class ManualProgramParser {
public:
    explicit ManualProgramParser(Lexer& lexer)
        : lexer(lexer) {}

    ProgramNode* parse() {
        LinkedList<::Token>& list = lexer.tokenize();
        manual_lr::Token* input = manual_lr::buildTokenStream(list);

        manual_lr::ManualLRParser parser(input);
        ProgramNode* program = parser.parseProgram();

        errors = parser.getErrors();
        manual_lr::freeTokenStream(input);
        return program;
    }

    const std::vector<std::string>& getErrors() const {
        return errors;
    }

private:
    Lexer& lexer;
    std::vector<std::string> errors;
};

#endif