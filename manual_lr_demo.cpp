#include <iostream>
#include "Lexer.h"
#include "ProgramParser.h"

int main() {
    std::string code =
        "int main() {\n"
        "  x = 1;\n"
        "  if (x > 0) {\n"
        "    y = x;\n"
        "  } else {\n"
        "    y = 0;\n"
        "  }\n"
        "  while (x < 10) {\n"
        "    x = x + 1;\n"
        "  }\n"
        "}\n";

    Lexer lexer(code);
    ManualProgramParser parser(lexer);

    ProgramNode* program = parser.parse();

    const std::vector<std::string>& errors = parser.getErrors();
    for (size_t i = 0; i < errors.size(); ++i) {
        std::cout << errors[i] << "\n";
    }

    if (program) {
        program->print();
        delete program;
        return 0;
    }

    return 1;
}
