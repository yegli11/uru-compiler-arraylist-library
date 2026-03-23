#ifndef TRANSPILER_H
#define TRANSPILER_H

#include "ControlFlowAST.h"
#include <string>
#include <sstream>
#include <cctype>


// Clase que transpila un AST de C (subconjunto) a código TypeScript equivalente.
class TypeScriptTranspiler {
public:
    /**
     * Transpila el AST de un programa completo (ProgramNode) a código TypeScript.
     * Genera una función main y la invoca al final.
     * @param program Nodo raíz del programa (debe ser ProgramNode)
     * @return Código TypeScript generado como string
     */
    std::string transpile(const ProgramNode* program) const {
        if (!program) {
            return "";
        }

        std::ostringstream out;
        // Define la función principal en TypeScript
        out << "function " << safeFunctionName(program->functionName) << "(): void {\n";

        // Transpila el bloque principal del programa
        if (program->mainBlock) {
            transpileBlock(program->mainBlock, out, 1);
        }

        out << "}\n\n";
        // Llama a la función principal
        out << safeFunctionName(program->functionName) << "();\n";
        return out.str();
    }

private:
    /**
     * Devuelve una cadena de espacios para la indentación según el nivel dado.
     * @param level Nivel de indentación (cada nivel = 2 espacios)
     */
    std::string indent(int level) const {
        return std::string(level * 2, ' ');
    }

    /**
     * Elimina espacios en blanco al inicio y final de un string.
     * @param text Texto a recortar
     * @return Texto sin espacios al inicio ni final
     */
    std::string trim(const std::string& text) const {
        size_t start = 0;
        while (start < text.size() && std::isspace(static_cast<unsigned char>(text[start]))) {
            ++start;
        }

        size_t end = text.size();
        while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1]))) {
            --end;
        }

        return text.substr(start, end - start);
    }

    /**
     * Mapea un tipo de C a su equivalente en TypeScript.
     * int, float -> number; char -> string; void -> void; otro -> any
     * @param cType Tipo en C
     * @return Tipo en TypeScript
     */
    std::string mapTypeToTypeScript(const std::string& cType) const {
        if (cType == "int" || cType == "float") {
            return "number";
        }
        if (cType == "char") {
            return "string";
        }
        if (cType == "void") {
            return "void";
        }
        return "any";
    }

    /**
     * Devuelve un nombre de función seguro para TypeScript (por defecto "main" si está vacío).
     * @param name Nombre original
     * @return Nombre seguro
     */
    std::string safeFunctionName(const std::string& name) const {
        return name.empty() ? "main" : name;
    }

    /**
     * Normaliza la inicialización de la sección init de un for para TypeScript.
     * Convierte, por ejemplo, "int i = 0" a "let i: number = 0".
     * Si no hay inicialización, retorna la cadena original recortada.
     * @param rawInit Texto crudo de la inicialización
     * @return Inicialización en sintaxis TypeScript
     */
    std::string normalizeForInit(const std::string& rawInit) const {
        std::string text = trim(rawInit);
        if (text.empty()) {
            return text;
        }

        const std::string typeKeywords[4] = {"int", "float", "char", "void"};
        for (int i = 0; i < 4; ++i) {
            const std::string& keyword = typeKeywords[i];
            std::string prefix = keyword + " ";
            if (text.size() > prefix.size() && text.substr(0, prefix.size()) == prefix) {
                std::string remainder = trim(text.substr(prefix.size()));
                size_t eqPos = remainder.find('=');

                if (eqPos == std::string::npos) {
                    // Declaración sin inicialización
                    return "let " + remainder + ": " + mapTypeToTypeScript(keyword);
                }

                // Declaración con inicialización
                std::string id = trim(remainder.substr(0, eqPos));
                std::string expr = trim(remainder.substr(eqPos + 1));
                return "let " + id + ": " + mapTypeToTypeScript(keyword) + " = " + expr;
            }
        }

        // Si no es declaración, retorna el texto original
        return text;
    }

    /**
     * Transpila un bloque de sentencias (BlockNode) a TypeScript.
     * @param block Nodo de bloque
     * @param out Flujo de salida donde se escribe el código
     * @param depth Nivel de indentación
     */
    void transpileBlock(const BlockNode* block, std::ostringstream& out, int depth) const {
        if (!block) {
            return;
        }

        for (size_t i = 0; i < block->statements.size(); ++i) {
            transpileStatement(block->statements[i], out, depth);
        }
    }

    /**
     * Transpila una sentencia individual del AST a TypeScript.
     * Soporta declaraciones, asignaciones, if, while, for, do-while.
     * Si el nodo no es soportado, inserta un comentario.
     * @param node Nodo AST de la sentencia
     * @param out Flujo de salida
     * @param depth Nivel de indentación
     */
    void transpileStatement(const ASTNode* node, std::ostringstream& out, int depth) const {
        if (!node) {
            return;
        }

        // Declaración de variable (let x: number = ...;)
        const DeclarationNode* declaration = dynamic_cast<const DeclarationNode*>(node);
        if (declaration) {
            out << indent(depth) << "let " << declaration->identifier
                << ": " << mapTypeToTypeScript(declaration->type);
            if (!declaration->initExpr.empty()) {
                out << " = " << declaration->initExpr;
            }
            out << ";\n";
            return;
        }

        // Asignación (x = ...;)
        const AssignNode* assign = dynamic_cast<const AssignNode*>(node);
        if (assign) {
            out << indent(depth) << assign->identifier << " = " << assign->expressionRaw << ";\n";
            return;
        }

        // Bucle while
        const WhileNode* whileNode = dynamic_cast<const WhileNode*>(node);
        if (whileNode) {
            out << indent(depth) << "while (" << whileNode->conditionRaw << ") {\n";
            transpileBlock(whileNode->body, out, depth + 1);
            out << indent(depth) << "}\n";
            return;
        }

        // Bucle for
        const ForNode* forNode = dynamic_cast<const ForNode*>(node);
        if (forNode) {
            out << indent(depth) << "for ("
                << normalizeForInit(forNode->initRaw) << "; "
                << forNode->conditionRaw << "; "
                << forNode->updateRaw << ") {\n";
            transpileBlock(forNode->body, out, depth + 1);
            out << indent(depth) << "}\n";
            return;
        }

        // Bucle do-while
        const DoWhileNode* doWhileNode = dynamic_cast<const DoWhileNode*>(node);
        if (doWhileNode) {
            out << indent(depth) << "do {\n";
            transpileBlock(doWhileNode->body, out, depth + 1);
            out << indent(depth) << "} while (" << doWhileNode->conditionRaw << ");\n";
            return;
        }

        // Sentencia if/else if/else
        const IfNode* ifNode = dynamic_cast<const IfNode*>(node);
        if (ifNode) {
            out << indent(depth) << "if (" << ifNode->conditionRaw << ") {\n";
            transpileBlock(ifNode->thenBlock, out, depth + 1);
            out << indent(depth) << "}";

            // else if
            for (size_t i = 0; i < ifNode->elseIfBranches.size(); ++i) {
                out << " else if (" << ifNode->elseIfBranches[i].conditionRaw << ") {\n";
                transpileBlock(ifNode->elseIfBranches[i].block, out, depth + 1);
                out << indent(depth) << "}";
            }

            // else
            if (ifNode->elseBlock) {
                out << " else {\n";
                transpileBlock(ifNode->elseBlock, out, depth + 1);
                out << indent(depth) << "}";
            }

            out << "\n";
            return;
        }

        // Nodo no soportado
        out << indent(depth) << "// Unsupported AST node\n";
    }
};

#endif