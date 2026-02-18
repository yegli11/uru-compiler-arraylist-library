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
};

#endif
