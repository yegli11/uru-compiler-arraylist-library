#include <iostream>
#include <fstream>
#include "Lexer.h"
#include "ProgramParser.h"
#include "SemanticAnalyzer.h"
#include "Transpiler.h"

int main() {
    std::string code =
        "int main() {\n"
        "  char c = 'a';\n"
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

        if (errors.empty() && !hasSemanticErrors) {
            TypeScriptTranspiler transpiler;
            std::string tsCode = transpiler.transpile(program);

            std::cout << "\n===== CODIGO TYPESCRIPT TRANSPILADO =====\n";
            std::cout << tsCode;

            std::ofstream outFile("transpiled_output.ts");
            if (outFile) {
                outFile << tsCode;
                outFile.close();
                std::cout << "\nArchivo generado: transpiled_output.ts\n";
            } else {
                std::cout << "\nNo se pudo escribir transpiled_output.ts\n";
            }
        } else {
            std::cout << "\nTranspilacion omitida por errores sintacticos/semanticos.\n";
        }

        delete program;
        return (errors.empty() && !hasSemanticErrors) ? 0 : 1;
    }

    return 1;
}
