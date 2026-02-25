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

//     // 1️⃣ Operación simple
// std::string code = "a + b";

// // 2️⃣ Precedencia básica
// std::string code = "a + b * c";

// // 3️⃣ Paréntesis que cambian precedencia
// std::string code = "(a + b) * c";

// // 4️⃣ Números enteros
// std::string code = "3 + 4 * 5";

// // 5️⃣ Números flotantes
// std::string code = "3.14 * x";

// // 6️⃣ Expresión larga
// std::string code = "a + b * c - d / e";

// // 7️⃣ Paréntesis anidados
// std::string code = "(a + (b * c))";

// // 8️⃣ Solo un identificador
// std::string code = "x";

// // 9️⃣ Solo un número
// std::string code = "42";

// Expresión con asignaciones
std::string code = "int a = 5; int b = 3; int c = 10; int d = 1; (a + b) * (c - d) / d";
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
