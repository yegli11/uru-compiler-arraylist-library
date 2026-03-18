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

std::string code = "(3 + 3) * (10 - 1) / 1";
    try {
        Lexer lexer(code);
        Environment env;
        ProgramParser progParser(lexer);
        ASTNode* tree = progParser.parse();

        if (!tree) {
            std::cout << "No se encontró una expresión para parsear." << std::endl;
            return 1;
        }

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
