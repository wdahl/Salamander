#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <functional>
#include "flat_hash_map.hpp"
#include <set>
#include <map>
#include <fstream>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h> 
using namespace std;

enum ObjectType{
    METHOD,
    CLASS_DEF,
    FUNCTION_DEF,
    FUNCTION,
    INSTANCE,
    BUILT_IN,
    NONLOCAL,
    ITER,
    FILE_OBJ,
    THREAD,
    LOCK,
    SOCKET,
    LIST_ITER,
    SET_ITER,
    DICT_ITER,
};

enum IterType{
    ITER_LIST,
    ITER_TUPLE,
    ITER_SET,
    ITER_DICT
};

class Object{
    public:
    ObjectType type;
    bool marked;

    Object(ObjectType);
};

enum ExprType {
    BOOL_EXPR,
    NONE_EXPR,
    NUMBER_EXPR,
    OBJECT_EXPR,
    STRING_EXPR,
    ERROR_EXPR,
};

class Expr {
    public:
    ExprType type;
    string str;
    union {
        bool boolean;
        double number;
        Object* object;
    };
    
    Expr();
    Expr(bool);
    Expr(double);
    Expr(int);
    Expr(string);
    Expr(Object*);
    Expr(ExprType, string);
    string toString();
    bool equals(Expr*);
    bool less(Expr*);
    bool greater(Expr*);
};

enum OpType {
    CONSTANT_OP,
    NONE_OP,
    TRUE_OP,
    FALSE_OP,
    POP_OP,
    LOAD_NAME_OP,
    STORE_NAME_OP,
    LOAD_GLOBAL_OP,
    ASSIGN_GLOBAL_OP,
    STORE_GLOBAL_OP,
    LOAD_NONLOCAL_OP,
    STORE_NONLOCAL_OP,
    LOAD_ATTRIBUTE_OP,
    STORE_ATTRIBUTE_OP,
    LOAD_SUPER_OP,
    EQ_OP,
    GT_OP,
    LT_OP,
    ADD_OP,
    SUB_OP,
    MULT_OP,
    DIV_OP,
    NOT_OP,
    INVERT_OP,
    JUMP_OP,
    JUMP_IF_FALSE_OP,
    LOOP_OP,
    CALL_OP,
    METHOD_CALL_OP,
    SUPER_CALL_OP,
    FUNCTION_DEF_OP,
    DEL_NONLOCAL_OP,
    RETURN_OP,
    CLASS_DEF_OP,
    BASE_OP,
    FIELD_DEF_OP,
    METHOD_DEF_OP,
    LIST_OP,
    TUPLE_OP,
    SET_OP,
    DICT_OP,
    FLOOR_DIV_OP,
    MOD_OP,
    POW_OP,
    LSHIFT_OP,
    RSHIFT_OP,
    BIT_OR_OP,
    BIT_XOR_OP,
    BIT_AND_OP,
    BIT_NOT_OP,
    IS_OP,
    IN_OP,
    IF_EXP_OP,
    LOAD_SLICE_OP,
    STORE_SLICE_OP,
    LOAD_SUBSCRIPT_OP,
    STORE_SUBSCRIPT_OP,
    DEL_SLICE_OP,
    DEL_SUBSCRIPT_OP,
    LOAD_ITER_START_OP,
    LOAD_ITER_STOP_OP,
    LOAD_ITER_OP,
    INCREMENT_ITER_OP,
};

class ByteCode{
    public:
    vector<int> code;
    vector<Expr*> exprs;
    vector<int> lines;

    void generateCode(int, int);
    int storeExpr(Expr*);
    void clear();
    void translate(string);
    int translateCode(int, int);
    int printIndex(string, int);
    int printCode(string, int);
    int printExpr(string, int);
    int printJump(string, int);
    int printMethodCall(string, int);
    int printFunc(string, int);
};

class Function: public Object {
    public:
    int argCount;
    int nonlocalCount;
    ByteCode* code;
    string name;
    string returnType;

    Function(string);
    string toString();
};

class BuiltIn: public Object{
    public:
    function<Expr*(vector<Expr*>)> func;

    BuiltIn(function<Expr*(vector<Expr*>)>);
    string toString();
};

class NonlocalObj: public Object{
    public:
    Expr* scope;
    Expr* deleted;

    NonlocalObj(Expr*);
    string toString();
};

class FunctionDef: public Object{
    public:
    Function* function;
    vector<NonlocalObj*> nonlocals;

    FunctionDef(Function*);
    string toString();
};

class ClassDef: public Object{
    public:
    string name;
    ska::flat_hash_map<string, Expr*> methods;
    ska::flat_hash_map<string, Expr*> fields;

    ClassDef(string);
    string toString();
};

class Instance: public Object{
    public:
    ClassDef* classPtr;
    ska::flat_hash_map<string, Expr*> fields;

    Instance(ClassDef*);
    string toString();
};

class Method: public Object{
    public:
    Expr* classDef;
    FunctionDef* method;

    Method(Expr*, FunctionDef*);
    string toString();
};

class File: public Object{
    public:
    fstream file;
    string fileName;

    File(string, string);
    string toString();
};

class Iterator: public Object{
    public:
    IterType itertype;

    Iterator(IterType);
    string toString();
};
class List: public Iterator{
    public:
    vector<Expr*> list;

    List(vector<Expr*>);
    string toString();
};
class Tuple: public Iterator{
    public:
    vector<Expr*> tuple;

    Tuple(const vector<Expr*>);
    string toString();
};
class Set: public Iterator{
    public:
    set<Expr*> data;

    Set(set<Expr*>);
    string toString();
};
class Dict: public Iterator{
    public:
    map<Expr*, Expr*> dict;

    Dict(map<Expr*, Expr*>);
    string toString();
};

class Thread: public Object{
    public:
    thread threadFunc;
    FunctionDef* funcDef;
    Tuple* args;

    Thread(FunctionDef*);
    string toString();
};
class Lock: public Object{
    public:
    mutex mtx;
    static int id;

    Lock();
    string toString();
};
class Socket: public Object{
    public:
    int fd;
    struct sockaddr_in address;
    const int addrlen = sizeof(address);
    const int opt = 1;

    Socket();
    string toString();
};
class ListIter: public Object{
    public:
    vector<Expr*>::iterator it;

    ListIter(vector<Expr*>::iterator);
    string toString();
};
class SetIter: public Object{
    public:
    set<Expr*>::iterator it;

    SetIter(set<Expr*>::iterator);
    string toString();
};
class DictIter: public Object{
    public:
    map<Expr*, Expr*>::iterator it;

    DictIter(map<Expr*, Expr*>::iterator);
    string toString();
};