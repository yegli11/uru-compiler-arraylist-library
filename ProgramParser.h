#ifndef PROGRAM_PARSER_H
#define PROGRAM_PARSER_H

#include "Lexer.h"
#include "ExpressionParser.h"
#include "Environment.h"
#include "ASTNode.h"
#include "Token.h"
#include <stdexcept>

class ProgramParser {
public:
    ProgramParser(Lexer& lexer) : lexer(lexer) {}

    ASTNode* parse(Environment& env) {
        ASTNode* tree = nullptr;
        while (true) {
            Token t = lexer.peekNextToken();
            if (t.getType() == TOKEN_IDENTIFIER) {
                Token id = lexer.getNextToken();
                Token assign = lexer.getNextToken();
                if (assign.getType() != TOKEN_ASSIGN)
                    throw std::runtime_error("Se esperaba '=' en asignación");
                Token value = lexer.getNextToken();
                double val = 0;
                if (value.getType() == TOKEN_INTEGER || value.getType() == TOKEN_FLOAT_NUM) {
                    val = std::stod(value.getLexeme());
                } else {
                    throw std::runtime_error("Valor inválido en asignación: " + value.getLexeme());
                }
                env.set(id.getLexeme(), val);
                Token semi = lexer.getNextToken();
                if (semi.getType() != TOKEN_SEMICOLON)
                    throw std::runtime_error("Se esperaba ';' después de asignación");
            } else {
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

#endif
