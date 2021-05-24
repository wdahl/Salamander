#include "compiler.hpp"
#include <vector>
#include <functional>
#include <set>
using namespace std;

enum ExitCode{
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR
};

class Call{
    public: 
    FunctionDef* funcDef;
    int counter;
    int start;
};

class VirtualMachine{
    public:
    vector<Call*> calls;
    vector<Expr*> stack;
    set<Object*> objects;
    ska::flat_hash_map<string, Expr*>* variables;
    vector<NonlocalObj*> nonlocals;
    vector<Object*> garbageCollector;

    ExitCode start();

    VirtualMachine();
    void reset();
    void throwError(string);
    void builtInDef(string, function<Expr*(vector<Expr*>)>);
    void clear();
    ExitCode readByteCode(string, string);
    int getInstruction();
    double getNumber();
    ExitCode compareOp(OpType);
    ExitCode binOp(function<double(double,double)>);
    void push(Expr*);
    Expr* pop();
    void clearObjects();
    bool funcCall(FunctionDef*, int);
    bool exprCall(Expr*, int);
    bool superCall(ClassDef*, string, int);
    bool methodCall(string, int);
    bool storeMethod(ClassDef*, string);
    NonlocalObj* findNonlocal(Expr*);
    void delNonlocals(Expr*);
    void methodDef(string);
    void fieldDef(string);

    // garbage collection
    void markObject(Object*);
    void markExpr(Expr*);
    void markVector(vector<Expr*>);
    void markMap(ska::flat_hash_map<string, Expr*>);
    void mark();
    void trace();
    void clearObject(Object*);
    void sweep();
    void runGC();
};