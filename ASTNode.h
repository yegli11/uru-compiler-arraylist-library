#ifndef ASTNODE_H
#define ASTNODE_H

#include <iostream>
#include "Token.h"

class ASTNode {
public:
    virtual void print(int indent = 0) = 0;
    virtual ~ASTNode() {}
};

#endif
