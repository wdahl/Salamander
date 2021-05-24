#include "lexer.hpp"
#include <string>
#include <iostream>
using namespace std;

Lexer::Lexer(string code){
    this->code = code;
    current = this->code[0]; 
    counter = 0;
    line = 1;
    indentStack.push_back(0);
    indentLevel = 0;
    newLine = true;
    group = 0;
    formattingString = false;
    formattedStringLevel = -1;
}
Lexem* Lexer::loadLexem(){
    //End of file;
    if(counter == code.size()){
        counter++;
        return new Lexem(NEWLINE, "NEWLINE", line);
    }
    else if(counter > code.size()) {
        if(indentStack.back() != 0){
            indentStack.pop_back();
            return new Lexem(DEDENT, "DEDENT", line);
        }

        return new Lexem(TOKEN_EOF, "EOF", line);
    }

    if(indentLevel < indentStack.back()){
        indentStack.pop_back();
        return new Lexem(DEDENT, "DEDENT", line);
    }

    if(formattingString && formattedStringLevel == group && code[counter] != '{'){
        current = "";
        while(counter < code.size() && code[counter] != '"' && code[counter] != '{') {
            if(code[counter] == '\n') {
                line++;
                counter++;
                while(code[counter] == ' ') counter++;
            }
            else if(code[counter] == '\\'){
                counter++;
                if(code[counter] == 'n'){
                    counter++;
                    current += "\n";
                }
                else if(code[counter] == 't'){
                    counter++;
                    current += "\t";
                }
                else current += "\\";
            }
            else current += code[counter++];
        }

        if(counter >= code.size()) return new Lexem(TOKEN_EOF, "EOF", line);

        if(code[counter] == '"') {
            formattingString = false;
            formattedStringLevel = -1;
            counter++;
        }
        return new Lexem(FORMATTED_STRING, current, line);
    }

    // skip whitespace and comments
    int tabCount = 0;
    while(true){
        if(counter >= code.size()) return new Lexem(NEWLINE, "NEWLINE", line);

        char c = code[counter];
        if(c == '\r') counter++;
        else if(c == '\n'){
            line++;
            counter++;
            tabCount = 0;
            if(group == 0 && newLine == false){
                newLine = true;
                return new Lexem(NEWLINE, "NEWLINE", line-1);
            }
        }
        else if(c == '\\' && code[counter+1] == '\n'){
            line++;
            counter++;
            counter++;
        }
        else if(c == '\t' || c == ' ') {
            if(newLine){
                tabCount++;
            }
            counter++;
        }
        else if(c == '#')
            while(counter < code.size() && code[counter] != '\n') counter++;
        else 
            break;
    }

    if(newLine && tabCount > indentStack.back()) {
        indentStack.push_back(tabCount);
        indentLevel = tabCount;
        newLine = false;
        return new Lexem(INDENT, "INDENT", line);
    }
    else if(newLine && tabCount < indentStack.back()) {
        indentStack.pop_back();
        indentLevel = tabCount;
        newLine = false;
        return new Lexem(DEDENT, "DEDENT", line);
    }

    newLine = false;

    //End of file;
    if(counter >= code.size()) return new Lexem(TOKEN_EOF, "EOF", line);

    char c = code[counter++];
    current = c;

    // keywords and identifiers
    if(isalpha(c) || c == '_'){
        while(counter < code.size() && (isalpha(code[counter]) || isdigit(code[counter]) || code[counter] == '_')) current += code[counter++];

        if(current == "and") return new Lexem(AND, current, line);
        if(current == "class") return new Lexem(CLASS, current, line);
        if(current == "else") return new Lexem(ELSE, current, line);
        if(current == "if") return new Lexem(IF, current, line);
        if(current == "None") return new Lexem(NONE, current, line);
        if(current == "or") return new Lexem(OR, current, line);
        if(current == "return") return new Lexem(RETURN, current, line);
        if(current == "super") return new Lexem(SUPER, current, line);
        if(current == "while") return new Lexem(WHILE, current, line);
        if(current == "False") return new Lexem(FALSE, current, line);
        if(current == "for") return new Lexem(FOR, current, line);
        if(current == "def") return new Lexem(DEF, current, line);
        if(current == "self") return new Lexem(SELF, current, line);
        if(current == "True") return new Lexem(TRUE, current, line);
        //if(current == "as") return new Lexem(AS, current, line);
        //if(current == "assert") return new Lexem(ASSERT, current, line);
        //if(current == "async") return new Lexem(ASYNC, current, line);
        //if(current == "await") return new Lexem(AWAIT, current, line);
        //if(current == "break") return new Lexem(BREAK, current, line);
        //if(current == "continue") return new Lexem(CONT, current, line);
        if(current == "del") return new Lexem(DEL, current, line);
        if(current == "elif") return new Lexem(ELIF, current, line);
        //if(current == "except") return new Lexem(EXCEPT, current, line);
        //if(current == "finally") return new Lexem(FINALLY, current, line);
        //if(current == "from") return new Lexem(FROM, current, line);
        if(current == "global") return new Lexem(GLOBAL, current, line);
        if(current == "import") return new Lexem(IMPORT, current, line);
        if(current == "in") return new Lexem(IN, current, line);
        if(current == "is") {
            int newCount = counter;
            string newCurrent = current + code[newCount++];
            while(counter < code.size() && isalpha(code[newCount])) newCurrent += code[newCount++];
            if(newCurrent == "is not"){
                counter = newCount;
                return new Lexem(IS_NOT, newCurrent, line);
            }
            return new Lexem(IS, current, line);
        }
        //if(current == "lambda") return new Lexem(LAMBDA, current, line);
        //if(current == "nonlocal") return new Lexem(NONLOCAL, current, line);
        if(current == "not") {
            int newCount = counter;
            string newCurrent = current + code[newCount++];
            while(counter < code.size() && isalpha(code[newCount])) newCurrent += code[newCount++];
            if(newCurrent == "not in"){
                counter = newCount;
                return new Lexem(NOT_IN, newCurrent, line);
            }
            return new Lexem(NOT, current, line);
        }
        if(current == "pass") return new Lexem(PASS, current, line);
        //if(current == "raise") return new Lexem(RAISE, current, line);
        //if(current == "try") return new Lexem(TRY, current, line);
        //if(current == "with") return new Lexem(WITH, current, line);
        //if(current == "yield") return new Lexem(YIELD, current, line);
        //if(current == "let") return new Lexem(LET, current, line);
        if(current == "int") return new Lexem(DATA_TYPE, current, line);
        if(current == "float") return new Lexem(DATA_TYPE, current, line);
        if(current == "str") return new Lexem(DATA_TYPE, current, line);
        if(current == "bool") return new Lexem(DATA_TYPE, current, line);
        if(current == "list") return new Lexem(DATA_TYPE, current, line);
        if(current == "tuple") return new Lexem(DATA_TYPE, current, line);
        if(current == "set") return new Lexem(DATA_TYPE, current, line);
        if(current == "dict") return new Lexem(DATA_TYPE, current, line);
        if(current == "file") return new Lexem(DATA_TYPE, current, line);
        if(current == "thread") return new Lexem(DATA_TYPE, current, line);
        if(current == "lock") return new Lexem(DATA_TYPE, current, line);
        if(current == "socket") return new Lexem(DATA_TYPE, current, line);
        if(current == "Object") return new Lexem(DATA_TYPE, current, line);

        if(current == "f"){
            if(code[counter] == '"'){
                counter++;
                formattingString = true;
                formattedStringLevel = group;
                current = "";
                while(counter < code.size() && code[counter] != '"' && code[counter] != '{') {
                    if(code[counter] == '\n') {
                        line++;
                        counter++;
                        while(code[counter] == ' ') counter++;
                    }
                    else if(code[counter] == '\\'){
                        counter++;
                        if(code[counter] == 'n'){
                            counter++;
                            current += "\n";
                        }
                        else if(code[counter] == 't'){
                            counter++;
                            current += "\t";
                        }
                        else current += "\\";
                    }
                    else current += code[counter++];
                }
                if(counter >= code.size()) return new Lexem(TOKEN_EOF, "EOF", line);

                if(code[counter] == '"') {
                    formattingString = false;
                    formattedStringLevel = -1;
                    counter++;
                }

                return new Lexem(FORMATTED_STRING, current, line);
            }
        }

        if(current == "r"){
            if(code[counter] == '\''){
                counter++;
                current = "";
                while(counter < code.size() && code[counter] != '\'') {
                    if(code[counter] == '\n') {
                        line++;
                        counter++;
                        while(code[counter] == ' ') counter++;
                    }
                    else current += code[counter++];
                }

                if(counter >= code.size()) return new Lexem(TOKEN_EOF, "EOF", line);

                counter++;
                return new Lexem(STRING, current, line);
            }
            else if(code[counter] == '"'){
                counter++;
                current = "";
                while(counter < code.size() && code[counter] != '"') {
                    if(code[counter] == '\n') {
                        line++;
                        counter++;
                        while(code[counter] == ' ') counter++;
                    }
                    else current += code[counter++];
                }

                if(counter >= code.size()) return new Lexem(TOKEN_EOF, "EOF", line);

                counter++;
                return new Lexem(STRING, current, line);
            }
        }

        return new Lexem(IDENTIFIER, current, line);
    }

    // numbers
    if(isdigit(c)){
        while(counter < code.size() && isdigit(code[counter])) current += code[counter++];
        if(code[counter] == '.' && isdigit(code[counter+1])) {
            current += code[counter++];
            while(counter < code.size() && isdigit(code[counter])) current += code[counter++];
        }

        return new Lexem(NUMBER, current, line);
    }

    // special charecters
    switch(c){
        case '(': group++; return new Lexem(LEFT_PAREN, current, line);
        case ')': group--; return new Lexem(RIGHT_PAREN, current, line);
        case '{': group++; return new Lexem(LEFT_BRACE, current, line);
        case '}': group--; return new Lexem(RIGHT_BRACE, current, line);
        case '[': group++; return new Lexem(LEFT_BRACKET, current, line);
        case ']': group--; return new Lexem(RIGHT_BRACKET, current, line);
        case ';': return new Lexem(SEMICOLON, current, line);
        case ',': return new Lexem(COMMA, current, line);
        case '.': return new Lexem(DOT, current, line);
        case '-': 
            if(code[counter] == '='){
                current += code[counter++];
                return new Lexem(MINUS_EQUAL, current, line);
            }
            return new Lexem(MINUS, current, line);
        case '+': 
            if(code[counter] == '='){
                current += code[counter++];
                return new Lexem(PLUS_EQUAL, current, line);
            }
            return new Lexem(PLUS, current, line);
        case '/': 
            if(code[counter] == '='){
                current += code[counter++];
                return new Lexem(SLASH_EQUAL, current, line);
            }
            if(code[counter] == '/'){
                current += code[counter++];
                if(code[counter] == '='){
                    current += code[counter++];
                    return new Lexem(SLASH_SLASH_EQUAL, current, line);
                }
                return new Lexem(SLASH_SLASH, current, line);
            }
            return new Lexem(SLASH, current, line);
        case '*': 
            if(code[counter] == '='){
                current += code[counter++];
                return new Lexem(STAR_EQUAL, current, line);
            }
            if(code[counter] == '*'){
                current += code[counter++];
                if(code[counter] == '='){
                    current += code[counter++];
                    return new Lexem(STAR_STAR_EQUAL, current, line);
                }
                return new Lexem(STAR_STAR, current, line);
            }
            return new Lexem(STAR, current, line);

        case ':': return new Lexem(COLON, current, line);
        
        case '!':
            if(code[counter] == '='){
                current += code[counter++];
                return new Lexem(BANG_EQUAL, current, line);
            }
            return new Lexem(BANG, current, line);

        case '=':
            if(code[counter] == '='){
                current += code[counter++];
                return new Lexem(EQUAL_EQUAL, current, line);
            }
            return new Lexem(EQUAL, current, line);

        case '<':
            if(code[counter] == '='){
                current += code[counter++];
                return new Lexem(LESS_EQUAL, current, line);
            }
            if(code[counter] == '<'){
                current += code[counter++];
                if(code[counter] == '='){
                    current = code[counter++];
                    return new Lexem(LEFT_SHIFT_EQUAL, current, line);
                }
                return new Lexem(LEFT_SHIFT, current, line);
            }
            return new Lexem(LESS, current, line);

        case '>':
            if(code[counter] == '='){
                current += code[counter++];
                return new Lexem(GREATER_EQUAL, current, line);
            }
            if(code[counter] == '>'){
                current += code[counter++];
                if(code[counter] == '='){
                    current = code[counter++];
                    return new Lexem(RIGHT_SHIFT_EQUAL, current, line);
                }
                return new Lexem(RIGHT_SHIFT, current, line);
            }
            return new Lexem(GREATER, current, line);

        case '%':
            if(code[counter] == '='){
                current += code[counter++];
                return new Lexem(PERCENT_EQUAL, current, line);
            }
            return new Lexem(PERCENT, current, line);

        case '@':
            if(code[counter] == '='){
                current += code[counter++];
                return new Lexem(AT_EQUAL, current, line);
            }
            return new Lexem(AT, current, line);

        case '&':
            if(code[counter] == '='){
                current += code[counter++];
                return new Lexem(BIT_AND_EQUAL, current, line);
            }
            return new Lexem(BIT_AND, current, line);

        case '|':
            if(code[counter] == '='){
                current += code[counter++];
                return new Lexem(BIT_OR_EQUAL, current, line);
            }
            return new Lexem(BIT_OR, current, line);

        case '^':
            if(code[counter] == '='){
                current += code[counter++];
                return new Lexem(BIT_XOR_EQUAL, current, line);
            }
            return new Lexem(BIT_XOR, current, line);

        case '~':
            return new Lexem(BIT_NOT, current, line);

        case '"': 
        /*
            if(code[counter] == '"' && code[counter+1] == '"'){
                counter++;
                counter++;
                current = "";
                while(counter < code.size() && (code[counter] != '"' && code[counter+1] != '"' && code[counter+2] != '"')) {
                    current += code[counter++];
                }

                if(counter >= code.size()) return new Lexem(TOKEN_EOF, "EOF", line);

                counter++;
                counter++;
                counter++;
                return new Lexem(STRING, current, line);
            }
            */

            current = "";
            while(counter < code.size() && code[counter] != '"') {
                if(code[counter] == '\n') {
                    line++;
                    counter++;
                    while(code[counter] == ' ') counter++;
                }
                else if(code[counter] == '\\'){
                    counter++;
                    if(code[counter] == 'n'){
                        counter++;
                        current += "\n";
                    }
                    else if(code[counter] == 't'){
                        counter++;
                        current += "\t";
                    }
                    else if(code[counter] == '\''){
                        counter++;
                        current += "\'";
                    }
                    else if(code[counter] == '\"'){
                        counter++;
                        current += "\"";
                    }
                    else current += "\\";
                }
                else current += code[counter++];
            }

            if(counter >= code.size()) return new Lexem(TOKEN_EOF, "EOF", line);

            counter++;
            return new Lexem(STRING, current, line);

        case '\'': 
            current = "";
            while(counter < code.size() && code[counter] != '\'') {
                if(code[counter] == '\n') {
                    line++;
                    counter++;
                    while(code[counter] == ' ') counter++;
                }
                else if(code[counter] == '\\'){
                    counter++;
                    if(code[counter] == 'n'){
                        counter++;
                        current += "\n";
                    }
                    else if(code[counter] == 't'){
                        counter++;
                        current += "\t";
                    }
                    else if(code[counter] == '\''){
                        counter++;
                        current += "\'";
                    }
                    else if(code[counter] == '\"'){
                        counter++;
                        current += "\"";
                    }
                    else current += "\\";
                }
                else current += code[counter++];
            }

            if(counter >= code.size()) return new Lexem(TOKEN_EOF, "EOF", line);

            counter++;
            return new Lexem(STRING, current, line);
    }

    return new Lexem(ERROR, "Unexpected character: "+c, line); // unknown charecter
}

Lexem::Lexem(LexemType type, string value, int line){
    this->type = type;
    this->value = value;
    this->line = line;
}