#include <iostream>
#include <string>
#include <stdexcept>

#include "Lexer.h"
#include "ExpressionParser.h"
#include "ASTNode.h"
#include "BinaryNode.h"
#include "LeafNode.h"
#include "LinkedList.h"
#include "Token.h"
#include "Environment.h"
#include "ProgramParser.h"

int main() {

std::string code = "int a; int b = 3; int c = 10; int d = 1; (a + b) * (c - d) / d";
    try {
        Lexer lexer(code);
        Environment env;
        ProgramParser progParser(lexer);
        ASTNode* tree = progParser.parse(env);

        std::cout << "===== AST GENERADO =====\n";
        tree->print();

        double result = tree->evaluate(env);
        std::cout << "\n===== RESULTADO =====\n";
        std::cout << result << std::endl;
    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
