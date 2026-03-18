#include <iostream>
#include "Lexer.h"
#include "ProgramParser.h"
#include "SemanticAnalyzer.h"

int main() {
    std::string code =
        "int main() {\n"
        "  int x = 1;\n"
        "  int y = 0;\n"
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

    bool hasSemanticErrors = false;

    if (program) {
        SemanticAnalyzer semantic;
        semantic.analyze(program);
        const std::vector<std::string>& semanticErrors = semantic.getErrors();

        for (size_t i = 0; i < semanticErrors.size(); ++i) {
            hasSemanticErrors = true;
            std::cout << semanticErrors[i] << "\n";
        }

        if (semanticErrors.empty()) {
            std::cout << "Analisis semantico: sin errores.\n";
        }

        program->print();
        delete program;
        return (errors.empty() && !hasSemanticErrors) ? 0 : 1;
    }

    return 1;
}
