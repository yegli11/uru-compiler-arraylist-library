#include <iostream>
#include "lexer.h"

int main() {
    std::string code =
        "int main() {\n"
        "    i1nt x = 10;\n"
        "    float y = 3.14;\n"
        "    if (x > 5) {\n"
        "        return x + y;\n"
        "    }\n"
        "    return 0;\n"
        "}";

    Lexer lexer(code);
    LinkedList<Token>& tokens = lexer.tokenize();

    std::cout << "Tokens encontrados:\n";
    for (int i = 0; i < tokens.size(); i++) {
        std::cout << tokens.get(i).toString() << std::endl;
    }

    return 0;
}
