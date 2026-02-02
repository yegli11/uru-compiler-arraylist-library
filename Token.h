#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <iostream>

// Categorías generales de tokens
enum TokenCategory { 
    KEYWORD, 
    IDENTIFIER, 
    LITERAL, 
    OPERATOR, 
    DELIMITER, 
    COMMENT, 
    END, 
    ERROR 
};

// Tipos de tokens que vamos a reconocer
enum TokenType {
    // Palabras reservadas
    TOKEN_IF, // if
    TOKEN_ELSE, // else
    TOKEN_WHILE, // while
    TOKEN_FOR, // for
    TOKEN_RETURN, // return
    TOKEN_INT, // int
    TOKEN_FLOAT, // float
    TOKEN_CHAR, // char
    TOKEN_VOID, // void
    
    // Identificadores y literales
    TOKEN_IDENTIFIER, // Nombres de variables, funciones, etc.
    TOKEN_INTEGER, // Literales enteros
    TOKEN_FLOAT_NUM, // Literales flotantes
    TOKEN_STRING, // Literales de cadena
    TOKEN_CHAR_LITERAL, // Literales de caracter
    
    // Operadores
    TOKEN_PLUS, // +
    TOKEN_MINUS, // -
    TOKEN_MULTIPLY, // *
    TOKEN_DIVIDE, // /
    TOKEN_ASSIGN, // =
    TOKEN_EQUAL, // ==
    TOKEN_NOT_EQUAL, // !=
    TOKEN_LESS, // <
    TOKEN_LESS_EQUAL, // <=
    TOKEN_GREATER, // >
    TOKEN_GREATER_EQUAL, // >=
    
    // Delimitadores
    TOKEN_LPAREN,    // (
    TOKEN_RPAREN,    // )
    TOKEN_LBRACE,    // {
    TOKEN_RBRACE,    // }
    TOKEN_LBRACKET,  // [
    TOKEN_RBRACKET,  // ]
    TOKEN_SEMICOLON, // ;
    TOKEN_COMMA,     // ,
    
    // Comentarios y fin de archivo
    TOKEN_COMMENT, // Comentarios
    TOKEN_EOF, // Fin de archivo
    TOKEN_ERROR // Error
};

class Token {
private:
    TokenCategory category;
    TokenType type;
    std::string lexeme;
    int line;
    int column;

public:
    Token(TokenCategory category = ERROR,
          TokenType type = TOKEN_ERROR,
          const std::string& lexeme = "",
          int line = 0,
          int column = 0)
        : category(category), type(type), lexeme(lexeme), line(line), column(column) {}

    // Getters
    TokenCategory getCategory() const { return category; }
    TokenType getType() const { return type; }
    std::string getLexeme() const { return lexeme; }
    int getLine() const { return line; }
    int getColumn() const { return column; }

    // Para depuración
    std::string toString() const {
        return "Token[" + std::to_string(line) + ":" + std::to_string(column) +
               "] Category: " + std::to_string(category) +
               ", Type: " + std::to_string(type) + ", Lexeme: '" + lexeme + "'";
    }

    // Verificación de tipos
    bool isOperator() const {
        return category == OPERATOR;
    }
    
    bool isKeyword() const {
        return category == KEYWORD;
    }
};

#endif
