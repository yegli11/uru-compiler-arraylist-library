#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "ControlFlowAST.h"
#include <cctype>
#include <map>
#include <set>
#include <string>
#include <vector>

/**
 * Clase SemanticAnalyzer - Realiza el análisis semántico de un AST
 * 
 * Responsabilidades:
 * - Verificar que todas las variables usadas estén declaradas
 * - Gestionar ámbitos (scopes) anidados
 * - Verificar compatibilidad de tipos en asignaciones
 * - Detectar redeclaraciones de variables
 * - Validar estructuras de control
 */
class SemanticAnalyzer {
public:
    /**
     * Punto de entrada principal para el análisis semántico
     * @param program Nodo raíz del programa
     * @return true si el análisis fue exitoso (sin errores), false en caso contrario
     */
    bool analyze(ProgramNode* program) {
        errors.clear();           // Limpiar errores de análisis anteriores
        scopeStack.clear();       // Limpiar pila de ámbitos

        // Verificar que el programa no sea nulo
        if (!program) {
            errors.push_back("[SemanticError] El AST de programa es nulo.");
            return false;
        }

        pushScope();  // Crear ámbito global

        // Verificar que exista el bloque principal (main)
        if (program->mainBlock == nullptr) {
            errors.push_back("[SemanticError] El programa no contiene bloque principal.");
            popScope();
            return false;
        }

        // Analizar el bloque principal
        analyzeBlock(program->mainBlock, true);
        
        popScope();  // Salir del ámbito global
        return errors.empty();
    }

    /**
     * Obtener la lista de errores encontrados durante el análisis
     * @return Vector constante con los mensajes de error
     */
    const std::vector<std::string>& getErrors() const {
        return errors;
    }

private:
    // ==================== ESTRUCTURAS DE DATOS ====================
    
    /**
     * Pila de ámbitos: cada ámbito es un mapa que asocia:
     * - clave: nombre de la variable
     * - valor: tipo de la variable (int, float, char, etc.)
     */
    std::vector<std::map<std::string, std::string> > scopeStack;
    
    /**
     * Lista de errores semánticos encontrados
     */
    std::vector<std::string> errors;

    // ==================== GESTIÓN DE ÁMBITOS ====================

    /**
     * Crea un nuevo ámbito y lo añade a la pila
     * Útil al entrar en bloques: funciones, loops, if, etc.
     */
    void pushScope() {
        scopeStack.push_back(std::map<std::string, std::string>());
    }

    /**
     * Elimina el ámbito actual de la pila
     * Útil al salir de bloques
     */
    void popScope() {
        if (!scopeStack.empty()) {
            scopeStack.pop_back();
        }
    }

    // ==================== VERIFICACIÓN DE PALABRAS RESERVADAS ====================

    /**
     * Verifica si una palabra es una palabra reservada del lenguaje
     * @param word Palabra a verificar
     * @return true si es palabra reservada
     */
    bool isKeyword(const std::string& word) const {
        static const std::set<std::string> keywords = {
            "if", "else", "while", "for", "do", "return",
            "int", "float", "char", "void", "main"
        };
        return keywords.find(word) != keywords.end();
    }

    /**
     * Verifica si una palabra es un tipo de datos del lenguaje
     * @param word Palabra a verificar
     * @return true si es int, float, char o void
     */
    bool isTypeKeyword(const std::string& word) const {
        return word == "int" || word == "float" || word == "char" || word == "void";
    }

    // ==================== GESTIÓN DE SÍMBOLOS ====================

    /**
     * Verifica si una variable ha sido declarada en el ámbito actual o superiores
     * @param name Nombre de la variable
     * @return true si la variable está declarada en algún ámbito accesible
     */
    bool isDeclared(const std::string& name) const {
        // Buscar desde el ámbito más interno hacia el más externo
        for (int i = static_cast<int>(scopeStack.size()) - 1; i >= 0; --i) {
            if (scopeStack[i].find(name) != scopeStack[i].end()) {
                return true;
            }
        }
        return false;
    }

    /**
     * Busca y retorna el tipo de una variable declarada
     * @param name Nombre de la variable
     * @return Tipo de la variable o "<unknown>" si no está declarada
     */
    std::string lookupDeclaredType(const std::string& name) const {
        // Buscar desde el ámbito más interno hacia el más externo
        for (int i = static_cast<int>(scopeStack.size()) - 1; i >= 0; --i) {
            std::map<std::string, std::string>::const_iterator it = scopeStack[i].find(name);
            if (it != scopeStack[i].end()) {
                return it->second;  // Retornar el tipo asociado
            }
        }
        return "<unknown>";
    }

    /**
     * Declara una nueva variable en el ámbito actual
     * @param type Tipo de la variable
     * @param name Nombre de la variable
     * @param context Contexto para mensajes de error
     * @return true si la declaración fue exitosa
     */
    bool declareInCurrentScope(const std::string& type, const std::string& name, const std::string& context) {
        // Asegurar que existe un ámbito
        if (scopeStack.empty()) {
            pushScope();
        }

        std::map<std::string, std::string>& current = scopeStack.back();
        
        // Verificar redeclaración en el mismo ámbito
        if (current.find(name) != current.end()) {
            errors.push_back("[SemanticError] Redeclaración de '" + name + "' en " + context + ".");
            return false;
        }

        // Almacenar la variable con su tipo
        current[name] = type;
        return true;
    }

    /**
     * Verifica que una variable esté declarada, añade error si no
     * @param name Nombre de la variable
     * @param context Contexto para mensajes de error
     */
    void requireDeclared(const std::string& name, const std::string& context) {
        if (!isDeclared(name)) {
            errors.push_back("[SemanticError] Variable no declarada '" + name + "' en " + context + ".");
        }
    }

    // ==================== ANÁLISIS DE EXPRESIONES ====================

    /**
     * Extrae todos los identificadores (nombres de variables) de una expresión
     * Ignora contenido dentro de comillas (strings y caracteres)
     * @param expr Expresión a analizar
     * @return Vector con los identificadores encontrados
     */
    std::vector<std::string> extractIdentifiers(const std::string& expr) const {
        std::vector<std::string> ids;
        std::string current;
        bool inSingleQuote = false;  // Dentro de comillas simples: 'a'
        bool inDoubleQuote = false;  // Dentro de comillas dobles: "hola"

        for (size_t i = 0; i < expr.size(); ++i) {
            const char c = expr[i];

            // Manejo de comillas simples
            if (!inDoubleQuote && c == '\'') {
                inSingleQuote = !inSingleQuote;
                continue;
            }
            // Manejo de comillas dobles
            if (!inSingleQuote && c == '"') {
                inDoubleQuote = !inDoubleQuote;
                continue;
            }

            // Ignorar contenido dentro de strings/literales
            if (inSingleQuote || inDoubleQuote) {
                continue;
            }

            // Detectar inicio de identificador
            if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
                current.clear();
                current.push_back(c);
                size_t j = i + 1;
                // Leer el identificador completo
                while (j < expr.size()) {
                    const char cj = expr[j];
                    if (std::isalnum(static_cast<unsigned char>(cj)) || cj == '_') {
                        current.push_back(cj);
                        ++j;
                    } else {
                        break;
                    }
                }
                // Solo añadir si no es palabra reservada
                if (!isKeyword(current)) {
                    ids.push_back(current);
                }
                i = j - 1;  // Avanzar el índice
            }
        }

        return ids;
    }

    /**
     * Elimina espacios en blanco al inicio y final de un string
     * @param text Texto a procesar
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

    // ==================== DETECCIÓN DE LITERALES ====================

    /**
     * Determina si una expresión es un literal entero
     * @param expr Expresión a evaluar
     * @return true si es un entero válido
     */
    bool isIntegerLiteral(const std::string& expr) const {
        if (expr.empty()) {
            return false;
        }

        size_t i = 0;
        // Manejar signo opcional
        if (expr[i] == '+' || expr[i] == '-') {
            ++i;
        }
        if (i >= expr.size()) {
            return false;
        }

        // Todos los caracteres restantes deben ser dígitos
        for (; i < expr.size(); ++i) {
            if (!std::isdigit(static_cast<unsigned char>(expr[i]))) {
                return false;
            }
        }
        return true;
    }

    /**
     * Determina si una expresión es un literal flotante
     * @param expr Expresión a evaluar
     * @return true si es un flotante válido
     */
    bool isFloatLiteral(const std::string& expr) const {
        if (expr.empty()) {
            return false;
        }

        size_t i = 0;
        // Manejar signo opcional
        if (expr[i] == '+' || expr[i] == '-') {
            ++i;
        }
        if (i >= expr.size()) {
            return false;
        }

        bool hasDot = false;
        bool hasDigit = false;
        for (; i < expr.size(); ++i) {
            const char c = expr[i];
            if (std::isdigit(static_cast<unsigned char>(c))) {
                hasDigit = true;
                continue;
            }
            // Solo se permite un punto decimal
            if (c == '.' && !hasDot) {
                hasDot = true;
                continue;
            }
            return false;
        }

        return hasDot && hasDigit;  // Debe tener punto y al menos un dígito
    }

    /**
     * Verifica si una expresión es un identificador válido
     * @param expr Expresión a evaluar
     * @return true si es un identificador válido
     */
    bool isIdentifierLexeme(const std::string& expr) const {
        if (expr.empty()) {
            return false;
        }
        // Debe comenzar con letra o underscore
        if (!(std::isalpha(static_cast<unsigned char>(expr[0])) || expr[0] == '_')) {
            return false;
        }
        // El resto puede ser alfanumérico o underscore
        for (size_t i = 1; i < expr.size(); ++i) {
            if (!(std::isalnum(static_cast<unsigned char>(expr[i])) || expr[i] == '_')) {
                return false;
            }
        }
        // No puede ser palabra reservada
        return !isKeyword(expr);
    }

    /**
     * Detecta si una expresión es un literal carácter
     * @param expr Expresión a evaluar
     * @return true si está entre comillas simples
     */
    bool isCharLiteral(const std::string& expr) const {
        return expr.size() >= 2 && expr[0] == '\'' && expr[expr.size() - 1] == '\'';
    }

    /**
     * Detecta si una expresión es un literal string
     * @param expr Expresión a evaluar
     * @return true si está entre comillas dobles
     */
    bool isStringLiteral(const std::string& expr) const {
        return expr.size() >= 2 && expr[0] == '"' && expr[expr.size() - 1] == '"';
    }

    /**
     * Verifica que el contenido de un literal carácter sea válido
     * @param expr Literal carácter completo (ej: 'a')
     * @return true si el contenido es válido
     */
    bool isValidCharLiteralContent(const std::string& expr) const {
        if (!isCharLiteral(expr)) {
            return false;
        }

        const std::string content = expr.substr(1, expr.size() - 2);
        if (content.empty()) {
            return false;
        }

        // Caso: secuencia de escape (ej: '\n')
        if (content[0] == '\\') {
            return content.size() == 2;
        }

        // Caso: carácter normal
        return content.size() == 1;
    }

    // ==================== VERIFICACIÓN DE TIPOS ====================

    /**
     * Infiere el tipo de una expresión simple
     * @param rawExpr Expresión a analizar
     * @return Tipo inferido: int, float, char, string, o tipo de variable
     */
    std::string inferSimpleExpressionType(const std::string& rawExpr) const {
        const std::string expr = trim(rawExpr);
        if (expr.empty()) {
            return "<unknown>";
        }

        // Detectar literales en orden de especificidad
        if (isStringLiteral(expr)) {
            return "string";
        }

        if (isCharLiteral(expr)) {
            if (isValidCharLiteralContent(expr)) {
                return "char";
            }
            return "string";  // Múltiples caracteres se tratan como string
        }

        if (isIntegerLiteral(expr)) {
            return "int";
        }

        if (isFloatLiteral(expr)) {
            return "float";
        }

        if (isIdentifierLexeme(expr)) {
            return lookupDeclaredType(expr);
        }

        return "<unknown>";
    }

    /**
     * Verifica si dos tipos son compatibles para asignación
     * @param targetType Tipo destino
     * @param sourceType Tipo fuente
     * @return true si son compatibles
     */
    bool isTypeCompatible(const std::string& targetType, const std::string& sourceType) const {
        // No se puede asignar a void
        if (targetType == "void") {
            return false;
        }
        // Tipo desconocido se considera compatible (evita falsos positivos)
        if (sourceType == "<unknown>") {
            return true;
        }
        // Tipos iguales son compatibles
        if (targetType == sourceType) {
            return true;
        }
        // Promoción implícita: int puede asignarse a float
        if (targetType == "float" && sourceType == "int") {
            return true;
        }
        return false;
    }

    /**
     * Verifica la compatibilidad de tipos en una expresión
     * @param targetType Tipo esperado
     * @param expr Expresión a verificar
     * @param context Contexto para mensajes de error
     */
    void checkTypeCompatibility(const std::string& targetType,
                                const std::string& expr,
                                const std::string& context) {
        const std::string sourceType = inferSimpleExpressionType(expr);
        if (sourceType == "<unknown>") {
            return;  // No se puede inferir el tipo, no reportar error
        }

        if (!isTypeCompatible(targetType, sourceType)) {
            errors.push_back("[SemanticError] Incompatibilidad de tipos en " + context +
                             ": no se puede asignar '" + sourceType + "' a '" + targetType + "'.");
        }
    }

    /**
     * Verifica que todos los identificadores en una expresión estén declarados
     * @param expr Expresión a verificar
     * @param context Contexto para mensajes de error
     */
    void checkExpressionIdentifiers(const std::string& expr, const std::string& context) {
        const std::vector<std::string> ids = extractIdentifiers(expr);
        std::set<std::string> seen;  // Para evitar errores duplicados
        for (size_t i = 0; i < ids.size(); ++i) {
            if (seen.insert(ids[i]).second) {  // Si es la primera vez que vemos este ID
                requireDeclared(ids[i], context);
            }
        }
    }

    // ==================== ANÁLISIS ESPECÍFICO POR NODO ====================

    /**
     * Analiza la inicialización de un bucle for
     * Maneja dos casos: declaración con inicialización o expresión de asignación
     * @param initRaw Texto de inicialización del for
     */
    void analyzeForInit(const std::string& initRaw) {
        const std::string init = trim(initRaw);
        if (init.empty()) {
            return;
        }

        // Extraer la primera palabra para ver si es un tipo
        std::string firstWord;
        size_t i = 0;
        while (i < init.size() && !std::isspace(static_cast<unsigned char>(init[i]))) {
            firstWord.push_back(init[i]);
            ++i;
        }

        // Caso 1: No es declaración, es expresión (ej: i = 0)
        if (!isTypeKeyword(firstWord)) {
            checkExpressionIdentifiers(init, "inicialización del for");
            return;
        }

        // Caso 2: Es declaración
        if (firstWord == "void") {
            errors.push_back("[SemanticError] No se puede declarar una variable de tipo void en inicialización de for.");
            return;
        }

        // Saltar espacios después del tipo
        while (i < init.size() && std::isspace(static_cast<unsigned char>(init[i]))) {
            ++i;
        }

        // Verificar que sigue un identificador válido
        if (i >= init.size() || !(std::isalpha(static_cast<unsigned char>(init[i])) || init[i] == '_')) {
            errors.push_back("[SemanticError] Se esperaba identificador en inicialización de for.");
            return;
        }

        // Extraer el identificador
        std::string id;
        id.push_back(init[i]);
        ++i;
        while (i < init.size() && (std::isalnum(static_cast<unsigned char>(init[i])) || init[i] == '_')) {
            id.push_back(init[i]);
            ++i;
        }

        // Declarar la variable
        declareInCurrentScope(firstWord, id, "inicialización de for");

        // Saltar espacios antes del '='
        while (i < init.size() && std::isspace(static_cast<unsigned char>(init[i]))) {
            ++i;
        }

        // Verificar si hay inicialización
        if (i < init.size() && init[i] == '=') {
            ++i;
            const std::string rhs = trim(init.substr(i));
            if (!rhs.empty()) {
                checkExpressionIdentifiers(rhs, "inicialización de for");
                checkTypeCompatibility(firstWord, rhs, "inicialización de for");
            }
        }
    }

    /**
     * Analiza un bloque de código completo
     * @param block Nodo del bloque
     * @param createScope Indica si debe crear un nuevo ámbito
     */
    void analyzeBlock(BlockNode* block, bool createScope) {
        if (block == nullptr) {
            return;
        }

        if (createScope) {
            pushScope();  // Crear nuevo ámbito para el bloque
        }

        // Analizar cada sentencia del bloque
        for (size_t i = 0; i < block->statements.size(); ++i) {
            analyzeNode(block->statements[i]);
        }

        if (createScope) {
            popScope();  // Salir del ámbito del bloque
        }
    }

    /**
     * Función principal de dispatch que analiza cada tipo de nodo AST
     * @param node Nodo a analizar
     */
    void analyzeNode(ASTNode* node) {
        if (node == nullptr) {
            return;
        }

        // === NODO DE DECLARACIÓN ===
        if (DeclarationNode* decl = dynamic_cast<DeclarationNode*>(node)) {
            // Verificar que no se declare void
            if (decl->type == "void") {
                errors.push_back("[SemanticError] No se puede declarar una variable de tipo void: '" + decl->identifier + "'.");
            }

            // Declarar la variable en el ámbito actual
            declareInCurrentScope(decl->type, decl->identifier, "declaración");

            // Verificar inicialización si existe
            if (!decl->initExpr.empty()) {
                checkExpressionIdentifiers(decl->initExpr, "inicialización de '" + decl->identifier + "'");
                checkTypeCompatibility(decl->type,
                                     decl->initExpr,
                                     "inicialización de '" + decl->identifier + "'");
            }
            return;
        }

        // === NODO DE ASIGNACIÓN ===
        if (AssignNode* assign = dynamic_cast<AssignNode*>(node)) {
            // Verificar que la variable esté declarada
            requireDeclared(assign->identifier, "asignación");
            
            // Verificar identificadores en la expresión
            checkExpressionIdentifiers(assign->expressionRaw, "expresión de asignación a '" + assign->identifier + "'");
            
            // Verificar compatibilidad de tipos
            const std::string targetType = lookupDeclaredType(assign->identifier);
            if (targetType != "<unknown>") {
                checkTypeCompatibility(targetType,
                                     assign->expressionRaw,
                                     "asignación a '" + assign->identifier + "'");
            }
            return;
        }

        // === NODO WHILE ===
        if (WhileNode* whileNode = dynamic_cast<WhileNode*>(node)) {
            checkExpressionIdentifiers(whileNode->conditionRaw, "condición de while");
            analyzeBlock(whileNode->body, true);  // Crear nuevo ámbito para el cuerpo
            return;
        }

        // === NODO FOR ===
        if (ForNode* forNode = dynamic_cast<ForNode*>(node)) {
            pushScope();  // Ámbito para la variable de inicialización
            
            // Analizar cada parte del for
            analyzeForInit(forNode->initRaw);
            checkExpressionIdentifiers(forNode->conditionRaw, "condición de for");
            checkExpressionIdentifiers(forNode->updateRaw, "actualización de for");
            
            // Analizar el cuerpo (crea su propio ámbito)
            analyzeBlock(forNode->body, true);
            
            popScope();  // Salir del ámbito del for
            return;
        }

        // === NODO DO-WHILE ===
        if (DoWhileNode* doWhileNode = dynamic_cast<DoWhileNode*>(node)) {
            analyzeBlock(doWhileNode->body, true);  // Crear ámbito para el cuerpo
            checkExpressionIdentifiers(doWhileNode->conditionRaw, "condición de do-while");
            return;
        }

        // === NODO IF ===
        if (IfNode* ifNode = dynamic_cast<IfNode*>(node)) {
            // Analizar condición
            checkExpressionIdentifiers(ifNode->conditionRaw, "condición de if");
            
            // Analizar bloque then
            analyzeBlock(ifNode->thenBlock, true);
            
            // Analizar ramas else-if
            for (size_t i = 0; i < ifNode->elseIfBranches.size(); ++i) {
                checkExpressionIdentifiers(ifNode->elseIfBranches[i].conditionRaw, "condición de else-if");
                analyzeBlock(ifNode->elseIfBranches[i].block, true);
            }
            
            // Analizar bloque else
            analyzeBlock(ifNode->elseBlock, true);
            return;
        }

        // === NODO BLOQUE (manejo recursivo) ===
        if (BlockNode* block = dynamic_cast<BlockNode*>(node)) {
            analyzeBlock(block, true);
        }
    }
};

#endif