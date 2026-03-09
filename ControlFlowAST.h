#ifndef CONTROL_FLOW_AST_H
#define CONTROL_FLOW_AST_H

#include "ASTNode.h"
#include <string>
#include <vector>
#include <iostream>

// Nodo base para sentencias del subconjunto de C.
class StatementNode : public ASTNode {
public:
    double evaluate(const Environment&) override {
        return 0.0;
    }
};

// Guarda terminales cuando se hace shift de simbolos en la pila LR.
class TerminalNode : public ASTNode {
public:
    int type;
    std::string lexeme;

    TerminalNode(int tokenType, const std::string& tokenLexeme)
        : type(tokenType), lexeme(tokenLexeme) {}

    void print(int indent = 0) override {
        std::cout << std::string(indent, ' ') << "<" << type << ":" << lexeme << ">\n";
    }

    double evaluate(const Environment&) override {
        return 0.0;
    }
};

class BlockNode;

class ProgramNode : public ASTNode {
public:
    std::string functionName;
    BlockNode* mainBlock;

    ProgramNode(const std::string& name, BlockNode* block)
        : functionName(name), mainBlock(block) {}

    ~ProgramNode() override;

    void print(int indent = 0) override;

    double evaluate(const Environment&) override {
        return 0.0;
    }
};

class BlockNode : public StatementNode {
public:
    std::vector<ASTNode*> statements;

    ~BlockNode() override {
        for (size_t i = 0; i < statements.size(); ++i) {
            delete statements[i];
        }
    }

    void addStatement(ASTNode* stmt) {
        statements.push_back(stmt);
    }

    void print(int indent = 0) override {
        std::cout << std::string(indent, ' ') << "Block\n";
        for (size_t i = 0; i < statements.size(); ++i) {
            if (statements[i]) {
                statements[i]->print(indent + 2);
            }
        }
    }
};

class AssignNode : public StatementNode {
public:
    std::string identifier;
    std::string expressionRaw;

    AssignNode(const std::string& id, const std::string& expr)
        : identifier(id), expressionRaw(expr) {}

    void print(int indent = 0) override {
        std::cout << std::string(indent, ' ') << "Assign " << identifier
                  << " = " << expressionRaw << "\n";
    }
};

class WhileNode : public StatementNode {
public:
    std::string conditionRaw;
    BlockNode* body;

    WhileNode(const std::string& cond, BlockNode* block)
        : conditionRaw(cond), body(block) {}

    ~WhileNode() override {
        delete body;
    }

    void print(int indent = 0) override {
        std::cout << std::string(indent, ' ') << "While (" << conditionRaw << ")\n";
        if (body) {
            body->print(indent + 2);
        }
    }
};

class ForNode : public StatementNode {
public:
    std::string initRaw;
    std::string conditionRaw;
    std::string updateRaw;
    BlockNode* body;

    ForNode(const std::string& initExpr,
            const std::string& condExpr,
            const std::string& updateExpr,
            BlockNode* block)
        : initRaw(initExpr), conditionRaw(condExpr), updateRaw(updateExpr), body(block) {}

    ~ForNode() override {
        delete body;
    }

    void print(int indent = 0) override {
        std::cout << std::string(indent, ' ') << "For (" << initRaw << "; "
                  << conditionRaw << "; " << updateRaw << ")\n";
        if (body) {
            body->print(indent + 2);
        }
    }
};

class DoWhileNode : public StatementNode {
public:
    BlockNode* body;
    std::string conditionRaw;

    DoWhileNode(BlockNode* block, const std::string& cond)
        : body(block), conditionRaw(cond) {}

    ~DoWhileNode() override {
        delete body;
    }

    void print(int indent = 0) override {
        std::cout << std::string(indent, ' ') << "DoWhile\n";
        if (body) {
            body->print(indent + 2);
        }
        std::cout << std::string(indent + 2, ' ') << "while (" << conditionRaw << ")\n";
    }
};

class IfNode : public StatementNode {
public:
    struct ElseIfBranch {
        std::string conditionRaw;
        BlockNode* block;

        ElseIfBranch(const std::string& cond, BlockNode* blk)
            : conditionRaw(cond), block(blk) {}
    };

    std::string conditionRaw;
    BlockNode* thenBlock;
    std::vector<ElseIfBranch> elseIfBranches;
    BlockNode* elseBlock;

    IfNode(const std::string& cond, BlockNode* thenBlk)
        : conditionRaw(cond), thenBlock(thenBlk), elseBlock(nullptr) {}

    ~IfNode() override {
        delete thenBlock;
        for (size_t i = 0; i < elseIfBranches.size(); ++i) {
            delete elseIfBranches[i].block;
        }
        delete elseBlock;
    }

    void addElseIf(const std::string& cond, BlockNode* block) {
        elseIfBranches.push_back(ElseIfBranch(cond, block));
    }

    void setElseBlock(BlockNode* block) {
        elseBlock = block;
    }

    void print(int indent = 0) override {
        std::cout << std::string(indent, ' ') << "If (" << conditionRaw << ")\n";
        if (thenBlock) {
            thenBlock->print(indent + 2);
        }
        for (size_t i = 0; i < elseIfBranches.size(); ++i) {
            std::cout << std::string(indent, ' ') << "ElseIf ("
                      << elseIfBranches[i].conditionRaw << ")\n";
            if (elseIfBranches[i].block) {
                elseIfBranches[i].block->print(indent + 2);
            }
        }
        if (elseBlock) {
            std::cout << std::string(indent, ' ') << "Else\n";
            elseBlock->print(indent + 2);
        }
    }
};

inline ProgramNode::~ProgramNode() {
    delete mainBlock;
}

inline void ProgramNode::print(int indent) {
    std::cout << std::string(indent, ' ') << "Program " << functionName << "\n";
    if (mainBlock) {
        mainBlock->print(indent + 2);
    }
}

#endif
