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
};

#endif
