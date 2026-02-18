#ifndef EXPRESSIONPARSER_H
#define EXPRESSIONPARSER_H

#include <stdexcept>
#include <string>
#include "Lexer.h"
#include "BinaryNode.h"
#include "LeafNode.h"

class ExpressionParser {
private:
    Lexer& lexer;
    Token currentToken;

    void advance() {
        currentToken = lexer.getNextToken();
    }

    void error(const std::string& message) {
        throw std::runtime_error(
            "Error sintactico en linea " +
            std::to_string(currentToken.getLine()) +
            ", columna " +
            std::to_string(currentToken.getColumn()) +
            ": " + message +
            " (Token: '" + currentToken.getLexeme() + "')"
        );
    }

public:
    ExpressionParser(Lexer& lex) : lexer(lex) {
        advance();
    }

    ASTNode* parseExpression() {
        ASTNode* node = parseTerm();

        while (currentToken.getType() == TOKEN_PLUS ||
            currentToken.getType() == TOKEN_MINUS) {

            Token op = currentToken;
            advance();

            if (currentToken.getType() == TOKEN_EOF)
                error("Expresion incompleta despues del operador");

            ASTNode* right = parseTerm();
            node = new BinaryNode(op, node, right);
        }

        return node;
    }

private:
    ASTNode* parseTerm() {
        ASTNode* node = parseFactor();

        while (currentToken.getType() == TOKEN_MULTIPLY ||
            currentToken.getType() == TOKEN_DIVIDE) {

            Token op = currentToken;
            advance();

            if (currentToken.getType() == TOKEN_EOF)
                error("Expresion incompleta despues del operador");

            ASTNode* right = parseFactor();
            node = new BinaryNode(op, node, right);
        }

        return node;
    }

    ASTNode* parseFactor() {
        Token token = currentToken;

        // Número o identificador
        if (token.getType() == TOKEN_INTEGER ||
            token.getType() == TOKEN_FLOAT_NUM ||
            token.getType() == TOKEN_IDENTIFIER) {

            advance();
            return new LeafNode(token);
        }

        // Paréntesis
        if (token.getType() == TOKEN_LPAREN) {
            advance();
            ASTNode* node = parseExpression();

            if (currentToken.getType() != TOKEN_RPAREN)
                error("Se esperaba ')'");

            advance();
            return node;
        }

        error("Token inesperado");
        return nullptr;
    }
};

#endif
