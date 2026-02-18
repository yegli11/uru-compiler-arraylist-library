#ifndef LEAFNODE_H
#define LEAFNODE_H

#include "ASTNode.h"

class LeafNode : public ASTNode {
public:
    Token token;

    LeafNode(Token t) : token(t) {}

    void print(int indent = 0) override {
        std::cout << std::string(indent, ' ')
                << token.getLexeme() << "\n";
    }

    double evaluate(const Environment& env) override {
        if (token.getType() == TOKEN_INTEGER || token.getType() == TOKEN_FLOAT_NUM) {
            return std::stod(token.getLexeme());
        } else if (token.getType() == TOKEN_IDENTIFIER) {
            return env.get(token.getLexeme());
        } else {
            throw std::runtime_error("Token inesperado en evaluate: " + token.getLexeme());
        }
    }
};

#endif
