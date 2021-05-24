#include <string>
#include <vector>
using namespace std;

enum LexemType{
    LEFT_PAREN, RIGHT_PAREN,
    LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, MINUS, PLUS,
    SEMICOLON, SLASH, STAR, 

    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,

    IDENTIFIER, STRING, NUMBER,

    AND, CLASS, ELSE, FALSE,
    FOR, DEF, IF, NONE, OR,
    RETURN, SUPER, SELF,
    TRUE, DATA_TYPE, WHILE,

    ERROR,
    TOKEN_EOF,
    
    NEWLINE,
    INDENT,
    DEDENT,
    COLON,
    AS,
    ASSERT,
    ASYNC,
    AWAIT,
    BREAK,
    CONT,
    DEL,
    ELIF,
    EXCEPT,
    FINALLY,
    FROM,
    GLOBAL,
    IMPORT,
    IN,
    NOT_IN,
    IS,
    IS_NOT,
    LAMBDA,
    NOT,
    PASS,
    RAISE,
    TRY,
    WITH,
    YIELD,

    LEFT_BRACKET,
    RIGHT_BRACKET,
    STAR_STAR,
    SLASH_SLASH,
    PERCENT,
    AT,
    LEFT_SHIFT,
    RIGHT_SHIFT,
    BIT_AND,
    BIT_OR,
    BIT_XOR,
    BIT_NOT,
    PLUS_EQUAL,
    MINUS_EQUAL,
    STAR_EQUAL,
    SLASH_EQUAL,
    SLASH_SLASH_EQUAL,
    PERCENT_EQUAL,
    AT_EQUAL,
    BIT_AND_EQUAL,
    BIT_OR_EQUAL,
    BIT_XOR_EQUAL,
    LEFT_SHIFT_EQUAL,
    RIGHT_SHIFT_EQUAL,
    STAR_STAR_EQUAL,

    FORMATTED_STRING,
};

class Lexem{
    public:
    LexemType type;
    string value;
    int line;
    
    Lexem(){
        value = "";
    }
    Lexem(string str){
        value = str;
    }
    Lexem(LexemType, string, int);
};

class Lexer{
    public:
    string code;
    string current;
    int counter;
    int line;
    vector<int> indentStack;
    int indentLevel;
    bool newLine;
    int group;
    bool formattingString;
    int formattedStringLevel;
    Lexer(string);
    Lexem* loadLexem();
    Lexem* lexem(LexemType);
};