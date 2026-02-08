#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <cctype>
#include <map>
#include "LinkedList.h"
#include "Token.h"

class Lexer {
private:
    std::string source;
    int index;
    int line;
    int column;
    LinkedList<Token> tokenList;
    std::map<std::string, TokenType> keywords;

    // Inicializa el mapa de palabras reservadas (keywords) con su tipo de token
    void initKeywords() {
        keywords["if"] = TOKEN_IF;
        keywords["else"] = TOKEN_ELSE;
        keywords["while"] = TOKEN_WHILE;
        keywords["for"] = TOKEN_FOR;
        keywords["return"] = TOKEN_RETURN;
        keywords["int"] = TOKEN_INT;
        keywords["float"] = TOKEN_FLOAT;
        keywords["char"] = TOKEN_CHAR;
        keywords["void"] = TOKEN_VOID;
    }

    // Devuelve el carácter actual sin avanzar el índice
    char peek() const {
        return index < (int)source.size() ? source[index] : '\0';
    }
    // Devuelve el siguiente carácter sin avanzar el índice
    char peekNext() const {
        return (index + 1) < (int)source.size() ? source[index + 1] : '\0';
    }
    // Devuelve el carácter actual y avanza el índice. Actualiza línea y columna.
    char advance() {
        char c = peek();
        index++;
        if (c == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
        return c;
    }
    // Si el carácter actual es igual a 'expected', lo consume y devuelve true. Si no, false.
    bool match(char expected) {
        if (peek() == expected) {
            advance();
            return true;
        }
        return false;
    }
    // Avanza el índice mientras el carácter actual sea espacio, tabulación o salto de línea
    void skipWhitespace() {
        while (isspace(peek())) advance();
    }
    // Devuelve true si el carácter es válido para iniciar un identificador (letra o '_')
    bool isIdentifierStart(char c) const {
        return std::isalpha((unsigned char)c) || c == '_';
    }
    // Devuelve true si el carácter es válido para un identificador (letra, número o '_')
    bool isIdentifierChar(char c) const {
        return std::isalnum((unsigned char)c) || c == '_';
    }

    // Reconoce identificadores o palabras reservadas. Devuelve el token correspondiente.
    Token identifierOrKeyword() {
        int startCol = column;
        int startLine = line;
        std::string lexeme;
        lexeme += advance();
        while (isIdentifierChar(peek())) {
            lexeme += advance();
        }
        auto it = keywords.find(lexeme);
        if (it != keywords.end()) {
            return Token(KEYWORD, it->second, lexeme, startLine, startCol);
        }
        return Token(IDENTIFIER, TOKEN_IDENTIFIER, lexeme, startLine, startCol);
    }

    // Reconoce números enteros y flotantes. Devuelve el token correspondiente.
    Token number() {
        int startCol = column;
        int startLine = line;
        std::string lexeme;
        lexeme += advance();
        while (std::isdigit((unsigned char)peek())) {
            lexeme += advance();
        }
        bool isFloat = false;
        if (peek() == '.' && std::isdigit((unsigned char)peekNext())) {
            isFloat = true;
            lexeme += advance();
            while (std::isdigit((unsigned char)peek())) {
                lexeme += advance();
            }
        }
        TokenType type = isFloat ? TOKEN_FLOAT_NUM : TOKEN_INTEGER;
        TokenCategory category = LITERAL;
        return Token(category, type, lexeme, startLine, startCol);
    }

    // Reconoce literales de cadena ("...") o carácter ('...'). Maneja escapes. Devuelve el token correspondiente.
    Token stringLiteral() {
        int startCol = column;
        int startLine = line;
        char delimiter = advance(); // consume '"' or '\''
        std::string lexeme;
        while (peek() != delimiter && peek() != '\0') {
            if (peek() == '\\') {
                lexeme += advance();
                if (peek() != '\0') lexeme += advance();
            } else {
                lexeme += advance();
            }
        }
        if (peek() == delimiter) advance();
        TokenType type = (delimiter == '"') ? TOKEN_STRING : TOKEN_CHAR_LITERAL;
        TokenCategory category = LITERAL;
        return Token(category, type, lexeme, startLine, startCol);
    }

    // Reconoce operadores y delimitadores. Maneja comentarios y devuelve el token correspondiente o de error.
    Token operatorOrDelimiter() {
        int startCol = column;
        int startLine = line;
        char c = advance();
        std::string lexeme(1, c);
        switch (c) {
            case '+': return Token(OPERATOR, TOKEN_PLUS, lexeme, startLine, startCol);
            case '-': return Token(OPERATOR, TOKEN_MINUS, lexeme, startLine, startCol);
            case '*': return Token(OPERATOR, TOKEN_MULTIPLY, lexeme, startLine, startCol);
            case '/':
                if (peek() == '/') {
                    lexeme += advance();
                    while (peek() != '\n' && peek() != '\0') lexeme += advance();
                    return Token(COMMENT, TOKEN_COMMENT, lexeme, startLine, startCol);
                } else if (peek() == '*') {
                    lexeme += advance();
                    while (!(peek() == '*' && peekNext() == '/')) {
                        if (peek() == '\0') break;
                        lexeme += advance();
                    }
                    if (peek() == '*') {
                        lexeme += advance();
                        lexeme += advance();
                    }
                    return Token(COMMENT, TOKEN_COMMENT, lexeme, startLine, startCol);
                } else {
                    return Token(OPERATOR, TOKEN_DIVIDE, lexeme, startLine, startCol);
                }
            case '=':
                if (peek() == '=') {
                    lexeme += advance();
                    return Token(OPERATOR, TOKEN_EQUAL, lexeme, startLine, startCol);
                } else {
                    return Token(OPERATOR, TOKEN_ASSIGN, lexeme, startLine, startCol);
                }
            case '!':
                if (peek() == '=') {
                    lexeme += advance();
                    return Token(OPERATOR, TOKEN_NOT_EQUAL, lexeme, startLine, startCol);
                } else {
                    return Token(ERROR, TOKEN_ERROR, lexeme, startLine, startCol);
                }
            case '<':
                if (peek() == '=') {
                    lexeme += advance();
                    return Token(OPERATOR, TOKEN_LESS_EQUAL, lexeme, startLine, startCol);
                } else {
                    return Token(OPERATOR, TOKEN_LESS, lexeme, startLine, startCol);
                }
            case '>':
                if (peek() == '=') {
                    lexeme += advance();
                    return Token(OPERATOR, TOKEN_GREATER_EQUAL, lexeme, startLine, startCol);
                } else {
                    return Token(OPERATOR, TOKEN_GREATER, lexeme, startLine, startCol);
                }
            case '(': return Token(DELIMITER, TOKEN_LPAREN, lexeme, startLine, startCol);
            case ')': return Token(DELIMITER, TOKEN_RPAREN, lexeme, startLine, startCol);
            case '{': return Token(DELIMITER, TOKEN_LBRACE, lexeme, startLine, startCol);
            case '}': return Token(DELIMITER, TOKEN_RBRACE, lexeme, startLine, startCol);
            case '[': return Token(DELIMITER, TOKEN_LBRACKET, lexeme, startLine, startCol);
            case ']': return Token(DELIMITER, TOKEN_RBRACKET, lexeme, startLine, startCol);
            case ';': return Token(DELIMITER, TOKEN_SEMICOLON, lexeme, startLine, startCol);
            case ',': return Token(DELIMITER, TOKEN_COMMA, lexeme, startLine, startCol);
            default:
                return Token(ERROR, TOKEN_ERROR, lexeme, startLine, startCol);
        }
    }

public:
    // Constructor. Recibe el texto fuente, inicializa índices y palabras reservadas.
    Lexer(const std::string& src)
        : source(src), index(0), line(1), column(1) {
        initKeywords();
    }

    // Tokeniza todo el texto fuente y devuelve la lista enlazada de tokens.
    LinkedList<Token>& tokenize() {
        tokenList = LinkedList<Token>();
        while (index < (int)source.size()) {
            skipWhitespace();
            if (index >= (int)source.size()) break;
            char c = peek();
            if (isIdentifierStart(c)) {
                tokenList.add(identifierOrKeyword());
            } else if (std::isdigit((unsigned char)c)) {
                tokenList.add(number());
            } else if (c == '"' || c == '\'') {
                tokenList.add(stringLiteral());
            } else {
                tokenList.add(operatorOrDelimiter());
            }
        }
        tokenList.add(Token(END, TOKEN_EOF, "", line, column));
        return tokenList;
    }

    // Devuelve el siguiente token de la lista (y lo elimina). Si no hay más, devuelve EOF.
    Token getNextToken() {
        if (tokenList.size() == 0) tokenize();
        try {
            return tokenList.popFront();
        } catch (const std::out_of_range&) {
            return Token(END, TOKEN_EOF, "", line, column);
        }
    }

    // Devuelve el siguiente token de la lista (sin eliminarlo). Si no hay más, devuelve EOF.
    Token peekNextToken() {
        if (tokenList.size() == 0) tokenize();
        try {
            return tokenList.get(0);
        } catch (const std::out_of_range&) {
            return Token(END, TOKEN_EOF, "", line, column);
        }
    }
};

#endif
