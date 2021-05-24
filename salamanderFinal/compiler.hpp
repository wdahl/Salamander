#include <string>
#include <functional>
#include "types.hpp"
#include "lexer.hpp"
using namespace std;

class Reader{
    public:
    Lexem* next;
    Lexem* prev;
    bool error;
    bool handelingError;

    Reader(){
        error = false;
        handelingError = false;
    }
};

enum Order {
    NONE_ORDER,
    ASSIGN_ORDER,
    LAMBDA_ORDER,
    COND_ORDER,
    OR_ORDER,
    AND_ORDER,
    NOT_ORDER,
    EQ_ORDER,
    COMPARISON_ORDER,
    BIT_OR_ORDER,
    BIT_XOR_ORDER,
    BIT_AND_ORDER,
    SHIFT_ORDER,
    TERM_ORDER,
    FACTOR_ORDER,
    UNARY_ORDER,
    EXP_ORDER,
    AWAIT_ORDER,
    CALL_ORDER,
    PRIMARY_ORDER
};

class Parser{
    public:
    function<string(bool)> pre;
    function<string(bool)> in;
    Order order;

    Parser(function<string(bool)> pre, function<string(bool)> in, Order order){
        this->pre = pre;
        this->in = in;
        this->order = order;
    }
};

class Name{
    public:
    Lexem* id;
    string type;
    int scope;
    bool caught;
    vector<string> argTypes;

    Name(string type){
        id = new Lexem();
        scope = 0;
        this->type = type;
        caught = false;
    }
};

class Nonlocal{
    public:
    int index;
    bool notGlobal;
    string type;

    Nonlocal(string type){
        this->type = type;
    }
};

enum FuncType{
    FUNC_FUNC,
    INIT_FUNC,
    METHOD_FUNC,
    SCRIPT_FUNC,
};

class FuncParser{
    public:
    Function* func;
    FuncType type;
    vector<Name*> names;
    vector<Nonlocal*> nonlocals;
    int scopeCounter;

    FuncParser(FuncType, string);
    int loadName(Lexem*);
    int storeNonlocal(int, bool);
    void storeName(Lexem*, string);
    void assignName(string);
    void initName();
};

class ClassParser{
    public:
    Lexem* className;
    bool hasBase;
    vector<Name*> names;

    ClassParser(Lexem*);
    int loadName(Lexem*);
    void storeName(Lexem*, string);
    void assignName(string);
    void initName();
};

Function* runCompiler(string, string);

void forward();
void read(LexemType, string);

void handleError(Lexem*, string);
bool check(LexemType);
void synch();

int writeJump(int);
void writeJumpOffset(int);
void writeLoop(int);

void parseStmt();
string parseExpr(Order);

// Expressions
string parseNumber(bool);
string parseParen(bool);
string parseUnary(bool);
string parseBinOp(bool);
string parseLiteral(bool);
string parseString(bool);
string parseIdentifier(bool);
string parseAnd(bool);
string parseOr(bool);
string parseCall(bool);
string parseDot(bool);
string parseSuper(bool);
string parseSelf(bool);
string parseFString(bool);
string parseList(bool);
string parseSet(bool);
string parseIfExp(bool);
string parseSubscript(bool);