#ifndef BINARYNODE_H
#define BINARYNODE_H

#include "ASTNode.h"

class BinaryNode : public ASTNode {
public:
    Token op;
    ASTNode* left;
    ASTNode* right;

    BinaryNode(Token oper, ASTNode* l, ASTNode* r)
        : op(oper), left(l), right(r) {}

    void print(int indent = 0) override {
        if (right) right->print(indent + 4);
        std::cout << std::string(indent, ' ')
                  << op.getLexeme() << "\n";
        if (left) left->print(indent + 4);
    }

    double evaluate(const Environment& env) override {
        double lval = left->evaluate(env);
        double rval = right->evaluate(env);
        switch (op.getType()) {
            case TOKEN_PLUS: return lval + rval;
            case TOKEN_MINUS: return lval - rval;
            case TOKEN_MULTIPLY: return lval * rval;
            case TOKEN_DIVIDE: return lval / rval;
            default:
                throw std::runtime_error("Operador no soportado en evaluate: " + op.getLexeme());
        }
    }
};

#endif
