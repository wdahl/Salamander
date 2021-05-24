#include "virtualMachine.hpp"
#include <iostream>
#include <functional>
#include <cmath>
#include <sstream>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
using namespace std; 

vector<VirtualMachine*> threads;
//ska::flat_hash_map<string, Expr*> globalPool;

ExitCode globalExitCode;

Expr* len(vector<Expr*> args){
    if(args.size() != 1){
        string str = "Incorrect number of Arguments: 'len' expects 1 argument";
        return new Expr(ERROR_EXPR, str);
    }
    
    Expr* l = args[0];
    if(l->type != OBJECT_EXPR || l->object->type != ITER){
        if(l->type == STRING_EXPR){
            double length = l->toString().size();
            return new Expr(length);
        }
        string str = "Incorrect type: 'len' expects an iterable";
        return new Expr(ERROR_EXPR, str);
    }

    Iterator* iter = (Iterator*)l->object;
    switch(iter->itertype){
        case ITER_LIST: {
            List* list = (List*)iter;
            double size = list->list.size();
            return new Expr(size);
        }
        case ITER_TUPLE: {
            Tuple* tuple = (Tuple*)iter;
            double size = tuple->tuple.size();
            return new Expr(size);
        }
        case ITER_SET: {
            Set* set = (Set*)iter;
            double size = set->data.size();
            return new Expr(size);
        }
        case ITER_DICT: {
            Dict* dict = (Dict*)iter;
            double size = dict->dict.size();
            return new Expr(size);
        }
        default: return new Expr();
    }
}

Expr* input(vector<Expr*> args){
    if(args.size() != 1){
        string str = "Incorrect number of Arguments: 'input' expects 1 argument";
        return new Expr(ERROR_EXPR, str);
    }
    
    Expr* l = args[0];
    if(l->type != STRING_EXPR){
        string str = "Incorrect type: 'input' expects a string";
        return new Expr(ERROR_EXPR, str);
    }

    string str;
    cout << endl;
    cout << l->toString() << endl;
    getline(cin, str);
    cout << endl;

    return new Expr(str);
}

Expr* openFile(vector<Expr*> args){
    if(args.size() != 2){
        string str = "Incorrect number of Arguments: 'open' expects 2 argument, a path to a file to open and read/write/append flag";
        return new Expr(ERROR_EXPR, str);
    }
    
    Expr* l = args[0];
    if(l->type != STRING_EXPR){
        string str = "Incorrect type: 'open' expects the first argument to be a string";
        return new Expr(ERROR_EXPR, str);
    }
    Expr* v = args[1];
    if(v->type != STRING_EXPR){
        string str = "Incorrect type: 'open' expects the second argument to be a string";
        return new Expr(ERROR_EXPR, str);
    }

    if(v->toString() != "r" && v->toString() != "w" && v->toString() != "a"){
        string str = "Incorrect file type: file type should be either \"r\", \"w\", or \"a\"";
        return new Expr(ERROR_EXPR, str);
    }

    File* file = new File(l->toString(), v->toString());
    
    if(!file->file.is_open()){
        string str = "Could not open file: " + l->toString();
        return new Expr(ERROR_EXPR, str);
    }

    return new Expr(file);
}

Expr* readFile(vector<Expr*> args){
    if(args.size() != 1){
        string str = "Incorrect number of Arguments: 'read' expects 1 argument";
        return new Expr(ERROR_EXPR, str);
    }
    
    Expr* l = args[0];
    if(l->type != OBJECT_EXPR || l->object->type != FILE_OBJ){
        string str = "Incorrect type: 'read' expects a file to read from";
        return new Expr(ERROR_EXPR, str);
    }
    File* file = (File*)l->object;

    ostringstream stream;
    string str;

    stream << file->file.rdbuf();
    str = stream.str();

    return new Expr(str);
}
Expr* writeFile(vector<Expr*> args){
    if(args.size() != 2){
        string str = "Incorrect number of Arguments: 'write' expects 2 argument";
        return new Expr(ERROR_EXPR, str);
    }
    
    Expr* l = args[0];
    if(l->type != OBJECT_EXPR || l->object->type != FILE_OBJ){
        string str = "Incorrect type: 'write' expects the first argument to be file to write to";
        return new Expr(ERROR_EXPR, str);
    }
    File* file = (File*)l->object;

    Expr* v = args[1];
    if(v->type != STRING_EXPR){
        string str = "Incorrect type: 'write' expects the second argument to be string to write to the file";
        return new Expr(ERROR_EXPR, str);
    }
    string input = v->toString();

    file->file << input;

    return new Expr();
}
Expr* closeFile(vector<Expr*> args){
    if(args.size() != 1){
        string str = "Incorrect number of Arguments: 'close' expects 1 argument";
        return new Expr(ERROR_EXPR, str);
    }
    
    Expr* l = args[0];
    if(l->type != OBJECT_EXPR || l->object->type != FILE_OBJ){
        string str = "Incorrect type: 'close' expects a file to close";
        return new Expr(ERROR_EXPR, str);
    }
    File* file = (File*)l->object;

    file->file.close();

    return new Expr();
}

Expr* createThread(vector<Expr*> args){
    if(args.size() == 0){
        string str = "Incorrect number of Arguments: 'thread' expects atleast 1 argument";
        return new Expr(ERROR_EXPR, str);
    }
    if(args.size() > 2){
        string str = "Incorrect number of Arguments: 'thread' expects at most 2 argument";
        return new Expr(ERROR_EXPR, str);
    }

    Expr* name = args[0];
    if(name->type != OBJECT_EXPR || name->object->type != FUNCTION_DEF){
        string str = "Incorrect argument type: 'thread' expects the first arguement to be function";
        return new Expr(ERROR_EXPR, str);
    }
    FunctionDef* funcDef = (FunctionDef*) name->object;

    Thread* threadFunc = new Thread(funcDef);

    if(args.size() > 1){
        name = args[1];
        if(name->type != OBJECT_EXPR || name->object->type != ITER){
            string str = "Incorrect argument type: 'thread' expects the second argument to be a tuple";
            return new Expr(ERROR_EXPR, str);
        }

        Iterator* iter = (Iterator*) name->object;
        if(iter->itertype != ITER_TUPLE){
            string str = "Incorrect argument type: 'thread' expects the second argument to be a tuple";
            return new Expr(ERROR_EXPR, str);
        }

        Tuple* tuple = (Tuple*)iter;
        threadFunc->args = tuple;
    }

    return new Expr(threadFunc);
}

void runThread(Thread* threadFunc){
    VirtualMachine* vm = new VirtualMachine();

    vm->variables = threads[0]->variables;

    vm->push(new Expr(threadFunc->funcDef));
    if(threadFunc->args != NULL){
        for(Expr* v : threadFunc->args->tuple){
            vm->push(v);
        }
        vm->funcCall(threadFunc->funcDef, threadFunc->args->tuple.size());
    }
    else{
        vm->funcCall(threadFunc->funcDef, 0);
    }
    
    
    ExitCode exitcode = vm->start();

    if(exitcode != OK){
        globalExitCode = exitcode;
    }

    threads.pop_back();
}

Expr* startThread(vector<Expr*> args){
    if(args.size() != 1){
        string str = "Incorrect number of Arguments: 'startThread' expects 1 argument";
        return new Expr(ERROR_EXPR, str);
    }

    Expr* name = args[0];
    if(name->type != OBJECT_EXPR || name->object->type != THREAD){
        string str = "Incorrect argument type: 'startThread' expects a thread";
        return new Expr(ERROR_EXPR, str);
    }
    Thread* threadFunc = (Thread*)name->object;

    threadFunc->threadFunc = thread(runThread, threadFunc);

    return new Expr();   
}

Expr* joinThread(vector<Expr*> args){
    if(args.size() != 1){
        string str = "Incorrect number of Arguments: 'joinThread' expects 1 argument";
        return new Expr(ERROR_EXPR, str);
    }

    Expr* name = args[0];
    if(name->type != OBJECT_EXPR || name->object->type != THREAD){
        string str = "Incorrect argument type: 'joinThread' expects a thread";
        return new Expr(ERROR_EXPR, str);
    }
    Thread* threadFunc = (Thread*)name->object;

    threadFunc->threadFunc.join();

    return new Expr();
}

Expr* createLock(vector<Expr*> args){
    if(args.size() > 0){
        string str = "Incorrect number of Arguments: 'Lock' expects 0 argument";
        return new Expr(ERROR_EXPR, str);
    }

    return new Expr(new Lock());
}

Expr* getLock(vector<Expr*> args){
    if(args.size() != 1){
        string str = "Incorrect number of Arguments: 'getLock' expects 1 argument";
        return new Expr(ERROR_EXPR, str);
    }

    Expr* name = args[0];
    if(name->type != OBJECT_EXPR || name->object->type != LOCK){
        string str = "Incorrect argument type: 'getLock' expects a Lock";
        return new Expr(ERROR_EXPR, str);
    }
    Lock* lock = (Lock*)name->object;

    lock->mtx.lock();

    return new Expr();
}

Expr* releaseLock(vector<Expr*> args){
    if(args.size() != 1){
        string str = "Incorrect number of Arguments: 'getLock' expects 1 argument";
        return new Expr(ERROR_EXPR, str);
    }

    Expr* name = args[0];
    if(name->type != OBJECT_EXPR || name->object->type != LOCK){
        string str = "Incorrect argument type: 'getLock' expects a Lock";
        return new Expr(ERROR_EXPR, str);
    }
    Lock* lock = (Lock*)name->object;

    lock->mtx.unlock();

    return new Expr();
}
Expr* createSocket(vector<Expr*> args){
    if(args.size() != 0){
        string str = "Incorrect number of Arguments: 'Socket' expects 0 argument";
        return new Expr(ERROR_EXPR, str);
    }

    Socket* sock = new Socket();

    if ((sock->fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        string str = "socket failed";
        return new Expr(ERROR_EXPR, str);
    }
    if (setsockopt(sock->fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &(sock->opt), sizeof(sock->opt)))
    {
        string str = "setsockopt failed";
        return new Expr(ERROR_EXPR, str);
    }

    sock->address.sin_family = AF_INET;

    return new Expr(sock);
}
Expr* bindSocket(vector<Expr*> args){
    if(args.size() != 2){
        string str = "Incorrect number of Arguments: 'bind' expects 2 argument";
        return new Expr(ERROR_EXPR, str);
    }

    Expr* name = args[0];
    if(name->type != OBJECT_EXPR || name->object->type != SOCKET){
        string str = "Incorrect argument type: 'bind' expects the 1st argument to be a socket to bind";
        return new Expr(ERROR_EXPR, str);
    }
    Socket* sock = (Socket*)name->object;

    name = args[1];
    if(name->type != NUMBER_EXPR){
        string str = "Incorrect argument type: 'bind' expects 2nd argument to be an int with the port to bind too";
        return new Expr(ERROR_EXPR, str);
    }
    int port = (int)name->number;

    sock->address.sin_addr.s_addr = INADDR_ANY;
    sock->address.sin_port = htons( port );

    if (bind(sock->fd, (struct sockaddr *)&(sock->address), sizeof(sock->address))<0)
    {
        string str = "bind failed";
        return new Expr(ERROR_EXPR, str);
    }
    return new Expr();
}
Expr* listenSocket(vector<Expr*> args){
    if(args.size() != 1){
        string str = "Incorrect number of Arguments: 'listen' expects 1 argument";
        return new Expr(ERROR_EXPR, str);
    }

    Expr* name = args[0];
    if(name->type != OBJECT_EXPR || name->object->type != SOCKET){
        string str = "Incorrect argument type: 'listen' expects the 1st argument to be a socket to bind";
        return new Expr(ERROR_EXPR, str);
    }
    Socket* sock = (Socket*)name->object;

    if (listen(sock->fd, 3) < 0)
    {
        string str = "listen failed";
        return new Expr(ERROR_EXPR, str);
    }
    return new Expr();
}
Expr* acceptSocket(vector<Expr*> args){
    if(args.size() != 1){
        string str = "Incorrect number of Arguments: 'accept' expects 1 argument";
        return new Expr(ERROR_EXPR, str);
    }

    Expr* name = args[0];
    if(name->type != OBJECT_EXPR || name->object->type != SOCKET){
        string str = "Incorrect argument type: 'accept' expects the 1st argument to be a socket";
        return new Expr(ERROR_EXPR, str);
    }
    Socket* sock = (Socket*)name->object;

    Socket* newSock = new Socket();
    if ((newSock->fd = accept(sock->fd, (struct sockaddr *)&(sock->address), (socklen_t*)&(sock->addrlen)))<0)
    {
        string str = "accept failed";
        return new Expr(ERROR_EXPR, str);
    }

    return new Expr(newSock);
}
Expr* recvSocket(vector<Expr*> args){
    if(args.size() != 2){
        string str = "Incorrect number of Arguments: 'recv' expects 2 argument";
        return new Expr(ERROR_EXPR, str);
    }

    Expr* name = args[0];
    if(name->type != OBJECT_EXPR || name->object->type != SOCKET){
        string str = "Incorrect argument type: 'recv' expects the 1st argument to be a socket";
        return new Expr(ERROR_EXPR, str);
    }
    Socket* sock = (Socket*)name->object;

    name = args[1];
    if(name->type != NUMBER_EXPR){
        string str = "Incorrect argument type: 'recv' expects 2nd argument to be an int";
        return new Expr(ERROR_EXPR, str);
    }
    int bufferSize = (int)name->number;

    char buffer[bufferSize] = {0};
    int valread = read(sock->fd , buffer, bufferSize);
    string str = buffer;
    return new Expr(str);
}
Expr* sendSocket(vector<Expr*> args){
    if(args.size() != 2){
        string str = "Incorrect number of Arguments: 'send' expects 2 argument";
        return new Expr(ERROR_EXPR, str);
    }

    Expr* name = args[0];
    if(name->type != OBJECT_EXPR || name->object->type != SOCKET){
        string str = "Incorrect argument type: 'send' expects the 1st argument to be a socket";
        return new Expr(ERROR_EXPR, str);
    }
    Socket* sock = (Socket*)name->object;

    name = args[1];
    if(name->type != STRING_EXPR){
        string str = "Incorrect argument type: 'send' expects 2nd argument to be a string";
        return new Expr(ERROR_EXPR, str);
    }
    string data = name->toString();
    const char *cstr = data.c_str();

    send(sock->fd , cstr , strlen(cstr) , 0 );

    return new Expr();
}
Expr* connectSocket(vector<Expr*> args){
    if(args.size() != 3){
        string str = "Incorrect number of Arguments: 'connect' expects 3 argument";
        return new Expr(ERROR_EXPR, str);
    }

    Expr* name = args[0];
    if(name->type != OBJECT_EXPR || name->object->type != SOCKET){
        string str = "Incorrect argument type: 'connect' expects the 1st argument to be a socket";
        return new Expr(ERROR_EXPR, str);
    }
    Socket* sock = (Socket*)name->object;

    name = args[1];
    if(name->type != STRING_EXPR){
        string str = "Incorrect argument type: 'connect' expects 2nd argument to be a string denoting the host";
        return new Expr(ERROR_EXPR, str);
    }
    string host = name->toString();
    const char *cstr = host.c_str();

    name = args[2];
    if(name->type != NUMBER_EXPR){
        string str = "Incorrect argument type: 'connect' expects 3rd argument to be a int denoting the port";
        return new Expr(ERROR_EXPR, str);
    }
    int port = (int)name->number;

    sock->address.sin_port = htons(port);

    if(inet_pton(AF_INET, cstr, &(sock->address.sin_addr))<=0) 
    {
        string str = "\nInvalid address/ Address not supported \n";
        return new Expr(ERROR_EXPR, str);
    }
    if (connect(sock->fd, (struct sockaddr *)&(sock->address), sizeof(sock->address)) < 0)
    {
        string str = "\nConnection Failed \n";
        return new Expr(ERROR_EXPR, str);
    }

    return new Expr();
}

Expr* toStr(vector<Expr*> args){
    if(args.size() != 1){
        string str = "Incorrect number of Arguments: 'toStr' expects 1 argument";
        return new Expr(ERROR_EXPR, str);
    }

    Expr* name = args[0];
    return new Expr(name->toString());
}
Expr* toInt(vector<Expr*> args){
    if(args.size() != 1){
        string str = "Incorrect number of Arguments: 'toInt' expects 1 argument";
        return new Expr(ERROR_EXPR, str);
    }
    Expr* name = args[0];

    try{
        int i = stoi(name->toString());
        double d = i;
        return new Expr(d);
    }catch(invalid_argument){
        string str = name->toString() + " could not be converted to a integer";
        return new Expr(ERROR_EXPR, str);
    }
}
Expr* toFloat(vector<Expr*> args){
    if(args.size() != 1){
        string str = "Incorrect number of Arguments: 'toFloat' expects 1 argument";
        return new Expr(ERROR_EXPR, str);
    }
    Expr* name = args[0];

    try{
        double d = stod(name->toString());
        return new Expr(d);
    }catch(invalid_argument){
        string str = name->toString() + " could not be converted to a float";
        return new Expr(ERROR_EXPR, str);
    }
}
Expr* print(vector<Expr*> args){
    if(args.size() != 1){
        string str = "Incorrect number of Arguments: 'print' expects 1 argument";
        return new Expr(ERROR_EXPR, str);
    }
    Expr* name = args[0];

    cout << name->toString() << endl;
    return new Expr();
}
Expr* range(vector<Expr*> args){
    if(args.size() < 1 || args.size() > 2){
        string str = "Incorrect number of Arguments: 'range' expects at least 1 argument and at most 2";
        return new Expr(ERROR_EXPR, str);
    }

    Expr* value1 = args[0];
    if(value1->type != NUMBER_EXPR){
        string str = "Incorrect argument type: 'range' expects the first argument to be an integer";
        return new Expr(ERROR_EXPR, str);
    }
    double num1 = value1->number;
    if(floor(num1) != num1){
        string str = "Incorrect argument type: 'range' expects the first argument to be an integer";
        return new Expr(ERROR_EXPR, str);
    }

    if(args.size() == 1){
        vector<Expr*> v;
        for(double i=0; i<num1; i++){
            v.push_back(new Expr(i));
        }

        return new Expr(new List(v));
    }

    Expr* value2 = args[1];
    if(value2->type != NUMBER_EXPR){
        string str = "Incorrect argument type: 'range' expects the second argument to be an integer";
        return new Expr(ERROR_EXPR, str);
    }
    double num2 = value2->number;
    if(floor(num2) != num2){
        string str = "Incorrect argument type: 'range' expects the second argument to be an integer";
        return new Expr(ERROR_EXPR, str);
    }

    if(num2 <= num1){
        string str = "The first argument must be smaller than the second argument in 'range'";
        return new Expr(ERROR_EXPR, str);
    }

    vector<Expr*> v;
    for(double i=num1; i<num2; i++){
        v.push_back(new Expr(i));
    }

    return new Expr(new List(v));
}

Expr* createDict(vector<Expr*> args){
    if(args.size() != 0){
        string str = "Incorrect number of Arguments: 'Dict' expects 0 arguments";
        return new Expr(ERROR_EXPR, str);
    }

    map<Expr*, Expr*> m;
    return new Expr(new Dict(m));
}

void VirtualMachine::reset(){
    stack.clear();
    calls.clear();
    nonlocals.clear();
}
void VirtualMachine::throwError(string message){
    cerr << message << endl;
    for(int i=calls.size()-1; i>=0; i--){
        Call* funcCall = calls[i];
        Function* func = funcCall->funcDef->function;

        int opcode = func->code->code[funcCall->counter-1];
        cerr << "[line " << func->code->lines[opcode] << "] in ";
        if(func->name == ""){
            cerr << "script" << endl;
        }
        else{
            cerr << func->name << "()" << endl;
        }
    }

    reset();
}
void VirtualMachine::builtInDef(string name, function<Expr*(vector<Expr*>)> func){
    push(new Expr(name));
    push(new Expr(new BuiltIn(func)));
    (*variables)[stack[0]->toString()] = stack[1];
    stack.pop_back();
    stack.pop_back();
}
VirtualMachine::VirtualMachine(){
    variables = new ska::flat_hash_map<string, Expr*>();
    threads.push_back(this);

    builtInDef("len", len);
    builtInDef("input", input);
    builtInDef("open", openFile);
    builtInDef("read", readFile);
    builtInDef("write", writeFile);
    builtInDef("close", closeFile);
    builtInDef("Thread", createThread);
    builtInDef("start", startThread);
    builtInDef("join", joinThread);
    builtInDef("Lock", createLock);
    builtInDef("getLock", getLock);
    builtInDef("releaseLock", releaseLock);
    builtInDef("Socket", createSocket);
    builtInDef("bind", bindSocket);
    builtInDef("listen", listenSocket);
    builtInDef("accept", acceptSocket);
    builtInDef("recv", recvSocket);
    builtInDef("send", sendSocket);
    builtInDef("connect", connectSocket);
    builtInDef("toStr", toStr);
    builtInDef("toInt", toInt);
    builtInDef("toFloat", toFloat);
    builtInDef("print", print);
    builtInDef("range", range);
    builtInDef("Dict", createDict);
}
void VirtualMachine::clear(){
    variables->clear();
    clearObjects();
}
void VirtualMachine::push(Expr* name){
    stack.push_back(name);
    if(name->type == OBJECT_EXPR){
        objects.insert(name->object);
    }
}
Expr* VirtualMachine::pop(){
    Expr* name = stack.back();
    stack.pop_back();
    return name; 
}
bool VirtualMachine::funcCall(FunctionDef* funcDef, int argCount){
    if(argCount != funcDef->function->argCount){
        string message = "Expected " + funcDef->function->argCount;
        message += " args";
        throwError(message);
        return false;
    }

    if (calls.size() == 2048) {
        throwError("Stack overflow.");
        return false;
    }

    Call* funcCall = new Call();
    funcCall->funcDef = funcDef;
    funcCall->counter = 0;
    funcCall->start = stack.size() - argCount - 1;

    calls.push_back(funcCall);
    return true;
}
bool VirtualMachine::exprCall(Expr* exprCall, int argCount){
    if(exprCall->type == OBJECT_EXPR){
        switch(exprCall->object->type){
            case METHOD:{
                Method* method = (Method*)exprCall->object;
                stack[stack.size()-argCount-1] = method->classDef;
                return funcCall(method->method, argCount);
            }

            case CLASS_DEF:{
                ClassDef* classDef = (ClassDef*) exprCall->object;
                stack[stack.size()-argCount-1] = new Expr(new Instance(classDef));
                Expr* init;
                if(classDef->methods.find("__init__") != classDef->methods.end()){
                    init = classDef->methods["__init__"];
                    return funcCall((FunctionDef*)(init->object), argCount);
                }
                else if(argCount != 0){
                    throwError("Expected 0 arguments");
                    return false;
                }

                return true;
            }

            case FUNCTION_DEF: return funcCall((FunctionDef*)exprCall->object, argCount);
            case BUILT_IN: {
                BuiltIn* builtIn = (BuiltIn*)exprCall->object;
                vector<Expr*> args;
                for(int i=stack.size()-argCount; i<stack.size(); i++){
                    args.push_back(stack[i]);
                }

                Expr* result = builtIn->func(args);
                if(result->type == ERROR_EXPR){
                    throwError(result->toString());
                    return false;
                }
                for(int i=0; i<argCount+1; i++){
                    stack.pop_back();
                }
                push(result);
                return true;
            }

            default: break;
        }
    }

    throwError("cannot funcCall constant");
    return false;
}
bool VirtualMachine::superCall(ClassDef* classDef, string name, int argCount){
    Expr* method;
    if(classDef->methods.find(name) == classDef->methods.end()){
        throwError(name + " property is undefined");
        return false;
    }
    method = classDef->methods[name];

    return funcCall((FunctionDef*)method->object, argCount);
}
bool VirtualMachine::methodCall(string className, int argCount){
    Expr* classDef = stack[stack.size()-1-argCount];

    if(classDef->object->type != INSTANCE){
        throwError("Expr does not have methods");
        return false;
    }

    Instance* instance = (Instance*)classDef->object;

    Expr* name;
    if(instance->fields.find(className) != instance->fields.end()){
        name = instance->fields[className];
        stack[stack.size()-argCount-1] = name;
        return exprCall(name, argCount);
    }

    return superCall(instance->classPtr, className, argCount);
}
bool VirtualMachine::storeMethod(ClassDef* classDef, string name){
    Expr* func;
    if(classDef->methods.find(name) == classDef->methods.end()){
        throwError(name + " property is Undefined");
        return false;
    }
    func = classDef->methods[name];

    Method* method = new Method(stack.back(), (FunctionDef*)func->object);
    stack.pop_back();
    push(new Expr(method));
    return true;
}
NonlocalObj* VirtualMachine::findNonlocal(Expr* expr){
    NonlocalObj* curr;
    int i;
    for(i=0; i<nonlocals.size(); i++){
        curr = nonlocals[i];
        if(curr->scope <= expr) break;
    }

    if(i != nonlocals.size() && curr->scope == expr) return curr;

    NonlocalObj* newNonlocal = new NonlocalObj(expr);
    nonlocals.insert(nonlocals.begin()+i, newNonlocal);
    return newNonlocal;
}
void VirtualMachine::delNonlocals(Expr* last){
    for(int i=0; i<nonlocals.size(); i++){
        if(nonlocals[i]->scope < last) break;
        NonlocalObj* nonlocal = nonlocals[i];
        nonlocal->deleted = nonlocal->scope;
        nonlocal->scope = nonlocal->deleted;
    }
}
void VirtualMachine::fieldDef(string name){
    Expr* field = stack.back();
    ClassDef* classDef = (ClassDef*)stack[stack.size()-2]->object;
    classDef->fields[name] = field;
    stack.pop_back();
}
void VirtualMachine::methodDef(string name){
    Expr* method = stack.back();
    ClassDef* classDef = (ClassDef*)stack[stack.size()-2]->object;
    classDef->methods[name] = method;
    stack.pop_back();
}
ExitCode VirtualMachine::binOp(function<double(double,double)> func){
    if(stack.back()->type != NUMBER_EXPR || stack[stack.size()-2]->type != NUMBER_EXPR){
        throwError("Operands must be numbers.");
        return RUNTIME_ERROR;
    }

    double b = pop()->number;
    double a = pop()->number;
    push(new Expr(func(a, b)));
    return start();
}
ExitCode VirtualMachine::compareOp(OpType opcode){
    Expr* b = pop();
    Expr* a = pop();

    if(a->type != b->type) {
        throwError("Type mismatch.");
        return RUNTIME_ERROR;
    }

    switch(opcode){
        case EQ_OP: push(new Expr(a->equals(b))); break;
        case GT_OP: push(new Expr(a->greater(b))); break;
        case LT_OP: push(new Expr(a->less(b))); break;
    }
    return start();
}

ExitCode VirtualMachine::start(){
    Call* funcCall = calls.back();

    int step = 0;
    while(true){
        ByteCode* code = funcCall->funcDef->function->code;
        
        int instruction = code->code[funcCall->counter++];
        
        switch(instruction){
            case CONSTANT_OP: {
                double constantIndex = code->code[funcCall->counter++];
                Expr* constant = code->exprs[constantIndex];
                push(constant);
                break;
            }

            case NONE_OP: push(new Expr()); break;
            case TRUE_OP: push(new Expr(true)); break;
            case FALSE_OP: push(new Expr(false)); break;

            case POP_OP: {
                if(stack.size() > 0) stack.pop_back(); 
                break;
            }

            case LOAD_NAME_OP: {
                int exprIndex = code->code[funcCall->counter++];
                push(stack[exprIndex+funcCall->start]);
                break;
            }
            case STORE_NAME_OP: {
                int exprIndex = code->code[funcCall->counter++];
                stack[exprIndex+funcCall->start] = stack.back();
                break;
            }

            case LOAD_ITER_START_OP: {
                int exprIndex = code->code[funcCall->counter++];
                Iterator* iter = (Iterator*)(stack[exprIndex+funcCall->start]->object);
                switch(iter->itertype){
                    case ITER_LIST: {
                        List* list = (List*)iter;
                        ListIter* listIter = new ListIter(list->list.begin());
                        push(new Expr(listIter));
                        break;
                    }
                    case ITER_TUPLE: {
                        Tuple* tuple = (Tuple*)iter;
                        ListIter* listIter = new ListIter(tuple->tuple.begin());
                        push(new Expr(listIter));
                        break;
                    }
                    case ITER_SET: {
                        Set* s = (Set*)iter;
                        SetIter* setIter = new SetIter(s->data.begin());
                        push(new Expr(setIter));
                        break;
                    }
                    case ITER_DICT: {
                        Dict* dict = (Dict*)iter;
                        DictIter* dictIter = new DictIter(dict->dict.begin());
                        push(new Expr(dictIter));
                        break;
                    }
                }
                break;
            }
            case LOAD_ITER_STOP_OP: {
                int exprIndex = code->code[funcCall->counter++];
                Expr* name = stack[exprIndex+funcCall->start];
                if(name->type != OBJECT_EXPR || name->object->type != ITER){
                    throwError(name->toString() + " is not an iterable");
                    return RUNTIME_ERROR;
                }

                Iterator* iter = (Iterator*) name->object;
                
                switch(iter->itertype){
                    case ITER_LIST: {
                        List* list = (List*)iter;
                        ListIter* listIter = new ListIter(list->list.end());
                        push(new Expr(listIter));
                        break;
                    }
                    case ITER_TUPLE: {
                        Tuple* tuple = (Tuple*)iter;
                        ListIter* listIter = new ListIter(tuple->tuple.end());
                        push(new Expr(listIter));
                        break;
                    }
                    case ITER_SET: {
                        Set* s = (Set*)iter;
                        SetIter* setIter = new SetIter(s->data.end());
                        push(new Expr(setIter));
                        break;
                    }
                    case ITER_DICT: {
                        Dict* dict = (Dict*)iter;
                        DictIter* dictIter = new DictIter(dict->dict.end());
                        push(new Expr(dictIter));
                        break;
                    }
                    default: push(new Expr());
                }
                break;
            }
            case INCREMENT_ITER_OP: {
                int exprIndex = code->code[funcCall->counter++];
                Expr* name = stack[exprIndex+funcCall->start];
                switch(name->object->type){
                    case LIST_ITER: {
                        ListIter* iter = (ListIter*)(name->object);
                        iter->it++;
                        break;
                    }
                    case SET_ITER: {
                        SetIter* iter = (SetIter*)(name->object);
                        iter->it++;
                        break;
                    }
                    case DICT_ITER: {
                        DictIter* iter = (DictIter*)(name->object);
                        iter->it++;
                        break;
                    }
                }
                break;
            }
            case LOAD_ITER_OP: {
                int exprIndex = code->code[funcCall->counter++];
                Expr* name = stack[exprIndex+funcCall->start];
                switch(name->object->type){
                    case LIST_ITER: {
                        ListIter* iter = (ListIter*)(name->object);
                        push(*(iter->it));
                        break;
                    }
                    case SET_ITER: {
                        SetIter* iter = (SetIter*)(name->object);
                        push(*(iter->it));
                        break;
                    }
                    case DICT_ITER: {
                        DictIter* iter = (DictIter*)(name->object);
                        push(iter->it->first);
                        break;
                    }
                }
                break;
            }

            case LOAD_GLOBAL_OP: {
                int stringIndex = code->code[funcCall->counter++];
                string id = code->exprs[stringIndex]->toString();
                Expr* name;
                if(variables->find(id) == variables->end()){
                    throwError(id + " is Undefined");
                    return RUNTIME_ERROR;
                }
                name = (*variables)[id];
                push(name);
                break;
            }
            case ASSIGN_GLOBAL_OP: {
                int stringIndex = code->code[funcCall->counter++];
                string name = code->exprs[stringIndex]->toString();
                (*variables)[name] = stack.back();
                stack.pop_back();
                break;
            }
            case STORE_GLOBAL_OP: {
                int stringIndex = code->code[funcCall->counter++];
                string name = code->exprs[stringIndex]->toString();
                if(variables->find(name) == variables->end()){
                    throwError(name + " is Undefined");
                    return RUNTIME_ERROR;
                }
                (*variables)[name] = stack.back();
                break;
            }

            case LOAD_NONLOCAL_OP: {
                int nonlocalIndex = code->code[funcCall->counter++];
                push(funcCall->funcDef->nonlocals[nonlocalIndex]->scope);
                break;
            }
            case STORE_NONLOCAL_OP: {
                int index = code->code[funcCall->counter++];
                funcCall->funcDef->nonlocals[index]->scope = stack.back();
                break;
            }

            case LOAD_ATTRIBUTE_OP: {
                if(stack.back()->object->type != INSTANCE){
                    throwError("identifier is not an instance of a class");
                    return RUNTIME_ERROR;
                }

                Instance* instance = (Instance*) stack.back()->object;
                int stringIndex = code->code[funcCall->counter++];
                string id = code->exprs[stringIndex]->toString();

                Expr* name;
                if(instance->fields.find(id) != instance->fields.end()){
                    name = instance->fields[id];
                    stack.pop_back();
                    push(name);
                    break;
                }

                if(instance->classPtr->fields.find(id) != instance->classPtr->fields.end()){
                    name = instance->classPtr->fields[id];
                    stack.pop_back();
                    push(name);
                    break;
                }

                if(!storeMethod(instance->classPtr, id)){
                    return RUNTIME_ERROR;
                }

                break;
            }
            case STORE_ATTRIBUTE_OP: {
                if(stack[stack.size()-2]->object->type != INSTANCE){
                    throwError("identifier is not an insatnce of a class");
                    return RUNTIME_ERROR;
                }

                Instance* instance = (Instance*)stack[stack.size()-2]->object;
                int stringIndex = code->code[funcCall->counter++];
                string id = code->exprs[stringIndex]->toString();
                instance->fields[id] = stack.back();

                Expr* name = pop();
                stack.pop_back();
                push(name);
                break;
            }

            case LOAD_SUPER_OP: {
                int stringIndex = code->code[funcCall->counter++];
                string name = code->exprs[stringIndex]->toString();
                ClassDef* baseClass = (ClassDef*)pop()->object;
                if(!storeMethod(baseClass, name)){
                    return RUNTIME_ERROR;
                }
                break;
            }
            
            case EQ_OP: return compareOp(EQ_OP);
            case GT_OP: return compareOp(GT_OP);
            case LT_OP: return compareOp(LT_OP);
            case IS_OP: {
                Expr* b = pop();
                Expr* a = pop();
                push(new Expr(a==b));
                break;
            }
            case IN_OP: {
                Expr* b = pop();
                Expr* a = pop();
                if(b->type != OBJECT_EXPR || b->object->type != ITER){
                    throwError("Cannot iterate over non-iterable");
                    return RUNTIME_ERROR;
                }

                Iterator* iter = (Iterator*)b->object;
                bool has = false;
                switch(iter->itertype){
                    case ITER_LIST: {
                        List* list = (List*) iter;
                        for(auto name : list->list){
                            if(a->equals(name)){
                                has = true;
                                break;
                            }
                        }
                        break;
                    }
                    case ITER_SET: {
                        Set* set = (Set*) iter;
                        for(auto name : set->data){
                            if(a->equals(name)){
                                has = true;
                                break;
                            }
                        }
                        break;
                    }
                    case ITER_DICT: {
                        Dict* dict = (Dict*) iter;
                        for(auto name : dict->dict){
                            if(a->equals(name.first)){
                                has = true;
                                break;
                            }
                        }
                        break;
                    }
                }

                push(new Expr(has));
                break;
            }

            case ADD_OP: {
                if(stack.back()->type == STRING_EXPR || stack[stack.size()-2]->type == STRING_EXPR){
                    string b = pop()->toString();
                    string a = pop()->toString();
                    push(new Expr(a+b));
                }
                else if(stack.back()->type == OBJECT_EXPR && stack[stack.size()-2]->type == OBJECT_EXPR){
                    if(stack.back()->object->type == ITER && stack[stack.size()-2]->object->type == ITER){
                        Iterator* iterB = (Iterator*)(pop()->object);
                        Iterator* iterA = (Iterator*)(pop()->object);
                        if(iterA->itertype != iterB->itertype){
                            throwError("operands do not match");
                            return RUNTIME_ERROR;
                        }
                        switch(iterA->itertype){
                            case ITER_LIST: {
                                List* listA = (List*)iterA;
                                List* listB = (List*)iterB;
                                vector<Expr*> v;
                                for(int i=0; i<listA->list.size(); i++){
                                    v.push_back(listA->list[i]);
                                }
                                for(int i=0; i<listB->list.size(); i++){
                                    v.push_back(listB->list[i]);
                                }

                                push(new Expr(new List(v)));
                                break;
                            }
                            case ITER_TUPLE: {
                                Tuple* listA = (Tuple*)iterA;
                                Tuple* listB = (Tuple*)iterB;
                                vector<Expr*> v;
                                for(int i=0; i<listA->tuple.size(); i++){
                                    v.push_back(listA->tuple[i]);
                                }
                                for(int i=0; i<listB->tuple.size(); i++){
                                    v.push_back(listB->tuple[i]);
                                }

                                push(new Expr(new Tuple(v)));
                                break;
                            }
                            case ITER_SET: {
                                Set* listA = (Set*)iterA;
                                Set* listB = (Set*)iterB;
                                set<Expr*> v;
                                for(auto i : listA->data){
                                    v.insert(i);
                                }
                                for(auto i : listB->data){
                                    v.insert(i);
                                }

                                push(new Expr(new Set(v)));
                                break;
                            }
                            default: 
                                throwError("cannot concatinate types");
                                return RUNTIME_ERROR;
                        }
                    }
                    else{
                        throwError("Operands cannot be added");
                        return RUNTIME_ERROR;
                    }
                }
                else if(stack.back()->type == NUMBER_EXPR && stack[stack.size()-2]->type == NUMBER_EXPR){
                    return binOp(plus<double>());
                }
                else{
                    throwError("Operands cannot be added");
                    return RUNTIME_ERROR;
                }

                break;
            }
            case SUB_OP: return binOp(minus<double>());
            case MULT_OP: return binOp(multiplies<double>());
            case DIV_OP: return binOp(divides<double>());
            case MOD_OP: {
                if(stack.back()->type == NUMBER_EXPR && stack[stack.size()-2]->type == NUMBER_EXPR){
                    double b = pop()->number;
                    double a = pop()->number;
                    double result = fmod(a, b);
                    push(new Expr(result));
                }
                else{
                    throwError("Operands must be numbers");
                    return RUNTIME_ERROR;
                }

                break;
            }
            case FLOOR_DIV_OP: {
                if(stack.back()->type == NUMBER_EXPR && stack[stack.size()-2]->type == NUMBER_EXPR){
                    double b = pop()->number;
                    double a = pop()->number;
                    double result = floor(a/b);
                    push(new Expr(result));
                }
                else{
                    throwError("Operands must be numbers");
                    return RUNTIME_ERROR;
                }

                break;
            }
            case POW_OP: {
                if(stack.back()->type == NUMBER_EXPR && stack[stack.size()-2]->type == NUMBER_EXPR){
                    double b = pop()->number;
                    double a = pop()->number;
                    double result = pow(a,b);
                    push(new Expr(result));
                }
                else{
                    throwError("Operands must be numbers");
                    return RUNTIME_ERROR;
                }

                break;
            }
            case LSHIFT_OP: {
                if(stack.back()->type == NUMBER_EXPR && stack[stack.size()-2]->type == NUMBER_EXPR){
                    int b = (int) pop()->number;
                    int a = (int) pop()->number;

                    double result = a << b;
                    push(new Expr(result));
                }
                else{
                    throwError("Operands must be numbers");
                    return RUNTIME_ERROR;
                }

                break;
            }
            case RSHIFT_OP: {
                if(stack.back()->type == NUMBER_EXPR && stack[stack.size()-2]->type == NUMBER_EXPR){
                    int b = (int) pop()->number;
                    int a = (int) pop()->number;

                    double result = a >> b;
                    push(new Expr(result));
                }
                else{
                    throwError("Operands must be numbers");
                    return RUNTIME_ERROR;
                }

                break;
            }
            case BIT_OR_OP: {
                if(stack.back()->type == NUMBER_EXPR && stack[stack.size()-2]->type == NUMBER_EXPR){
                    int b = (int) pop()->number;
                    int a = (int) pop()->number;

                    double result = a | b;
                    push(new Expr(result));
                }
                else{
                    throwError("Operands must be numbers");
                    return RUNTIME_ERROR;
                }

                break;
            }
            case BIT_XOR_OP: {
                if(stack.back()->type == NUMBER_EXPR && stack[stack.size()-2]->type == NUMBER_EXPR){
                    int b = (int) pop()->number;
                    int a = (int) pop()->number;

                    double result = a ^ b;
                    push(new Expr(result));
                }
                else{
                    throwError("Operands must be numbers");
                    return RUNTIME_ERROR;
                }

                break;
            }
            case BIT_AND_OP: {
                if(stack.back()->type == NUMBER_EXPR && stack[stack.size()-2]->type == NUMBER_EXPR){
                    int b = (int) pop()->number;
                    int a = (int) pop()->number;

                    double result = a & b;
                    push(new Expr(result));
                }
                else{
                    throwError("Operands must be numbers");
                    return RUNTIME_ERROR;
                }

                break;
            }

            case NOT_OP: {
                if(stack.back()->type != NONE_EXPR && stack.back()->type != NUMBER_EXPR && stack.back()->type != BOOL_EXPR){
                    throwError("Operand must be boolean, number, or null");
                    return RUNTIME_ERROR;
                }

                bool boolean = pop()->boolean;
                push(new Expr(!boolean));
                break;
            }
            case BIT_NOT_OP: {
                if(stack.back()->type == NUMBER_EXPR){
                    int b = (int) pop()->number;

                    double result = ~b;
                    push(new Expr(result));
                }
                else{
                    throwError("Operands must be numbers");
                    return RUNTIME_ERROR;
                }

                break;
            }

            case INVERT_OP: {
                if(stack.back()->type != NUMBER_EXPR){
                    throwError("Operand must be a number.");
                    return RUNTIME_ERROR;
                }

                double number = pop()->number;
                push(new Expr(-number));
                break;
            }

            case JUMP_OP: funcCall->counter += code->code[funcCall->counter]+1; break;
            case JUMP_IF_FALSE_OP: {
                if(!stack.back()->boolean) funcCall->counter += code->code[funcCall->counter]+1;
                else funcCall->counter++;
                break;
            }
            case LOOP_OP: funcCall->counter -= code->code[funcCall->counter];break;

            case IF_EXP_OP: {
                Expr* c = pop();
                Expr* b = pop();
                Expr* a = pop();

                if(b->boolean) push(a);
                else push(c);
                break;
            }

            case CALL_OP: {
                int argCount = code->code[funcCall->counter++];
                if(!exprCall(stack[stack.size()-1-argCount], argCount)){
                    return RUNTIME_ERROR;
                }

                funcCall = calls.back();
                break;
            }
            case LIST_OP: {
                int size = code->code[funcCall->counter++];
                vector<Expr*> list;
                for(int i=stack.size()-size; i<stack.size(); i++){
                    list.push_back(stack[i]);
                }

                int stackSize = stack.size();
                for(int i=stackSize-size; i<stackSize; i++){
                    stack.pop_back();
                }

                push(new Expr(new List(list)));
                break;
            }
            case TUPLE_OP: {
                int size = code->code[funcCall->counter++];
                vector<Expr*> tuple;
                for(int i=stack.size()-size; i<stack.size(); i++){
                    tuple.push_back(stack[i]);
                }

                int stackSize = stack.size();
                for(int i=stackSize-size; i<stackSize; i++){
                    stack.pop_back();
                }

                push(new Expr(new Tuple(tuple)));
                break;
            }
            case SET_OP: {
                int size = code->code[funcCall->counter++];
                set<Expr*> data;
                for(int i=stack.size()-size; i<stack.size(); i++){
                    bool has = false;
                    for(auto it : data){
                        if(stack[i]->equals(it)){
                            has = true;
                            break;
                        }
                    }
                    if(!has){
                        data.insert(stack[i]);
                    }
                }

                int stackSize = stack.size();
                for(int i=stackSize-size; i<stackSize; i++){
                    stack.pop_back();
                }

                push(new Expr(new Set(data)));
                break;
            }
            case DICT_OP: {
                int size = code->code[funcCall->counter++];
                map<Expr*, Expr*> dict;
                for(int i=stack.size()-size; i<stack.size(); i+=2){
                    bool has = false;
                    for(auto it : dict){
                        if(stack[i]->equals(it.first)){
                            has = true;
                            break;
                        }
                    }
                    if(!has){
                        dict[stack[i]] = stack[i+1];
                    }
                }

                int stackSize = stack.size();
                for(int i=stackSize-size; i<stackSize; i++){
                    stack.pop_back();
                }

                push(new Expr(new Dict(dict)));
                break;
            }

            case LOAD_SLICE_OP: {
                Expr* end = pop();
                Expr* start = pop();
                Expr* l = pop();

                if(end->type != NUMBER_EXPR || start->type != NUMBER_EXPR){
                    throwError("Subscripts must be numbers");
                    return RUNTIME_ERROR;
                }
                if(l->type != OBJECT_EXPR || l->object->type != ITER){
                    throwError("Cannot subscript a non-iterable");
                    return RUNTIME_ERROR;
                }
                Iterator* iter = (Iterator*)l->object;

                if(iter->itertype != ITER_LIST && iter->itertype != ITER_TUPLE){
                    throwError("Can only slice list and tuple objects");
                    return RUNTIME_ERROR;
                }
                switch(iter->itertype){
                    case ITER_LIST: {
                        List* list = (List*)iter;
                        vector<Expr*> newList;
                        for(int i=start->number; i<end->number && i<list->list.size(); i++){
                            newList.push_back(list->list[i]);
                        }
                        push(new Expr(new List(newList)));
                        break;
                    }
                    case ITER_TUPLE: {
                        Tuple* list = (Tuple*)iter;
                        vector<Expr*> newList;
                        for(int i=start->number; i<end->number && i<list->tuple.size(); i++){
                            newList.push_back(list->tuple[i]);
                        }
                        push(new Expr(new Tuple(newList)));
                        break;
                    }
                }
                break;
            }
            case STORE_SLICE_OP: {
                Expr* exp = pop();
                Expr* end = pop();
                Expr* start = pop();
                Expr* l = pop();

                if(end->type != NUMBER_EXPR || start->type != NUMBER_EXPR){
                    throwError("Subscripts must be numbers");
                    return RUNTIME_ERROR;
                }
                if(start->number >= end->number){
                    throwError("start subscript must be smaller than end subscript");
                    return RUNTIME_ERROR;
                }
                if(l->type != OBJECT_EXPR || l->object->type != ITER){
                    throwError("Cannot subscript a non-iterable");
                    return RUNTIME_ERROR;
                }
                Iterator* iter = (Iterator*)l->object;

                if(iter->itertype != ITER_LIST && iter->itertype != ITER_TUPLE){
                    throwError("Can only slice list and tuple objects");
                    return RUNTIME_ERROR;
                }

                if(exp->type != OBJECT_EXPR || exp->object->type != ITER){
                    throwError("Incompatiable types, Expected list");
                    return RUNTIME_ERROR;
                }
                Iterator* iterExp = (Iterator*)exp->object;

                if(iterExp->itertype != ITER_LIST){
                    throwError("Incompatiable types, Expected list");
                    return RUNTIME_ERROR;
                }
                List* listExp = (List*)iterExp;

                switch(iter->itertype){
                    case ITER_LIST: {
                        List* list = (List*)iter;
                        if(start->number >= list->list.size()){
                            throwError("index out of bounds");
                            return RUNTIME_ERROR;
                        }
                        vector<Expr*> newList;
                        for(double i=0; i<start->number; i++){
                            newList.push_back(list->list[i]);
                        }
                        for(double i=0; i<listExp->list.size(); i++){
                            newList.push_back(listExp->list[i]);
                        }
                        for(double i=end->number; i<list->list.size(); i++){
                            newList.push_back(list->list[i]);
                        }

                        list->list = newList;
                        break;
                    }
                    case ITER_TUPLE: {
                        throwError("tuples are constants so its contents cannot be changed");
                        return RUNTIME_ERROR;
                    }
                }
                push(new Expr());
                break;
            }
            case DEL_SLICE_OP: {
                Expr* end = pop();
                Expr* start = pop();
                Expr* l = pop();

                if(end->type != NUMBER_EXPR || start->type != NUMBER_EXPR){
                    throwError("Subscripts must be numbers");
                    return RUNTIME_ERROR;
                }
                if(l->type != OBJECT_EXPR || l->object->type != ITER){
                    throwError("Cannot subscript a non-iterable");
                    return RUNTIME_ERROR;
                }
                Iterator* iter = (Iterator*)l->object;

                if(iter->itertype != ITER_LIST && iter->itertype != ITER_TUPLE){
                    throwError("Can only slice list and tuple objects");
                    return RUNTIME_ERROR;
                }
                switch(iter->itertype){
                    case ITER_LIST: {
                        List* list = (List*)iter;
                        if(end->number >= list->list.size()){
                            list->list.erase(list->list.begin()+start->number, list->list.end());
                        }
                        else{
                            list->list.erase(list->list.begin()+start->number, list->list.begin()+end->number);
                        }
                        
                        break;
                    }
                    case ITER_TUPLE: {
                        throwError("tuples are constants so its contents cannot be changed");
                        return RUNTIME_ERROR;
                    }
                }
                break;
            }
            case LOAD_SUBSCRIPT_OP: {
                Expr* sub = pop();
                Expr* l = pop();

                if(l->type != OBJECT_EXPR || l->object->type != ITER){
                    throwError("Cannot subscript a non-iterable");
                    return RUNTIME_ERROR;
                }
                Iterator* iter = (Iterator*)l->object;

                if(iter->itertype != ITER_LIST && iter->itertype != ITER_TUPLE && iter->itertype != ITER_DICT){
                    throwError("Can only subscript lists, tuples, and dictionaires");
                    return RUNTIME_ERROR;
                }

                switch(iter->itertype){
                    case ITER_LIST: {
                        if(sub->type != NUMBER_EXPR){
                            throwError("Subscripts must be numbers");
                            return RUNTIME_ERROR;
                        }

                        List* list = (List*)iter;
                        if(sub->number >= list->list.size()){
                            throwError("index out of bounds");
                            return RUNTIME_ERROR;
                        }

                        if(sub->number >= 0){
                            push(list->list[sub->number]);
                        }
                        else{
                            if(list->list.size()-sub->number < 0){
                                throwError("index out of bounds");
                                return RUNTIME_ERROR;
                            }

                            push(list->list[list->list.size()-(-(sub->number))]);
                        }

                        break;
                    }
                    case ITER_TUPLE: {
                        if(sub->type != NUMBER_EXPR){
                            throwError("Subscripts must be numbers");
                            return RUNTIME_ERROR;
                        }

                        Tuple* tuple = (Tuple*)iter;
                        if(sub->number >= tuple->tuple.size()){
                            throwError("index out of bounds");
                            return RUNTIME_ERROR;
                        }

                        if(sub->number >= 0){
                            push(tuple->tuple[sub->number]);
                        }
                        else{
                            if(tuple->tuple.size()-sub->number < 0){
                                throwError("index out of bounds");
                                return RUNTIME_ERROR;
                            }

                            push(tuple->tuple[tuple->tuple.size()-(-(sub->number))]);
                        }

                        break;
                    }
                    case ITER_DICT: {
                        Dict* dict = (Dict*)iter;
                        bool found = false;
                        for(auto pair : dict->dict){
                            if(pair.first->equals(sub)) {
                                found = true;
                                push(pair.second);
                                break;
                            }
                        }

                        if(!found){
                            throwError("key not found");
                            return RUNTIME_ERROR;
                        }
                        break;
                    }
                }
                break;
            }
            case STORE_SUBSCRIPT_OP: {
                Expr* exp = pop();
                Expr* sub = pop();
                Expr* l = pop();

                if(l->type != OBJECT_EXPR || l->object->type != ITER){
                    throwError("Cannot subscript a non-iterable");
                    return RUNTIME_ERROR;
                }
                Iterator* iter = (Iterator*)l->object;

                if(iter->itertype != ITER_LIST && iter->itertype != ITER_TUPLE && iter->itertype != ITER_DICT){
                    throwError("Can only subscript lists, tuples, and dictionaires");
                    return RUNTIME_ERROR;
                }

                switch(iter->itertype){
                    case ITER_LIST: {
                        if(sub->type != NUMBER_EXPR){
                            throwError("Subscripts must be an integer");
                            return RUNTIME_ERROR;
                        }

                        if(floor(sub->number) != sub->number){
                            throwError("Subscripts must be an integer");
                            return RUNTIME_ERROR;
                        }

                        List* list = (List*)iter;
                        if(sub->number > list->list.size()){
                            throwError("index out of bounds");
                            return RUNTIME_ERROR;
                        }

                        if(sub->number == list->list.size()){
                            list->list.push_back(exp);
                        }

                        if(sub->number >= 0){
                            list->list[sub->number] = exp;
                        }
                        else{
                            if(list->list.size()-sub->number < 0){
                                throwError("index out of bounds");
                                return RUNTIME_ERROR;
                            }

                            list->list[list->list.size()-(-(sub->number))] = exp;
                        }

                        break;
                    }
                    case ITER_TUPLE: {
                        throwError("Tuples are constants so there content cannot be changed");
                        return RUNTIME_ERROR;
                    }
                    case ITER_DICT: {
                        Dict* dict = (Dict*)iter;
                        bool found = false;
                        for(auto pair : dict->dict){
                            if(pair.first->equals(sub)) {
                                found = true;
                                dict->dict[pair.first] = exp;
                                break;
                            }
                        }

                        if(!found){
                            dict->dict[sub] = exp;
                        }
                        break;
                    }
                }
                push(new Expr());
                break;
            }
            case DEL_SUBSCRIPT_OP: {
                Expr* sub = pop();
                Expr* l = pop();

                if(l->type != OBJECT_EXPR || l->object->type != ITER){
                    throwError("Cannot subscript a non-iterable");
                    return RUNTIME_ERROR;
                }
                Iterator* iter = (Iterator*)l->object;

                if(iter->itertype != ITER_LIST && iter->itertype != ITER_TUPLE && iter->itertype != ITER_DICT){
                    throwError("Can only subscript lists, tuples, and dictionaires");
                    return RUNTIME_ERROR;
                }

                switch(iter->itertype){
                    case ITER_LIST: {
                        if(sub->type != NUMBER_EXPR){
                            throwError("Subscripts must be numbers");
                            return RUNTIME_ERROR;
                        }

                        List* list = (List*)iter;
                        if(sub->number >= list->list.size()){
                            throwError("index out of bounds");
                            return RUNTIME_ERROR;
                        }

                        if(sub->number >= 0){
                            list->list.erase(list->list.begin()+sub->number);
                        }
                        else{
                            if(list->list.size()-sub->number < 0){
                                throwError("index out of bounds");
                                return RUNTIME_ERROR;
                            }

                            int i = list->list.size()-(-(sub->number));
                            list->list.erase(list->list.begin()+i);
                        }

                        break;
                    }
                    case ITER_TUPLE: {
                        throwError("Tuples are constants so there contents cannot change");
                        return RUNTIME_ERROR;
                    }
                    case ITER_DICT: {
                        Dict* dict = (Dict*)iter;
                        bool found = false;
                        for(auto pair : dict->dict){
                            if(pair.first->equals(sub)) {
                                found = true;
                                dict->dict.erase(pair.first);
                                break;
                            }
                        }
                        break;
                    }
                }
                break;
            }

            case METHOD_CALL_OP: {
                int stringIndex = code->code[funcCall->counter++];
                string method = code->exprs[stringIndex]->toString();
                int argCount = code->code[funcCall->counter++];
                if(!methodCall(method, argCount)){
                    return RUNTIME_ERROR;
                }
                funcCall = calls.back();
                break;
            }
            case SUPER_CALL_OP: {
                int stringIndex = code->code[funcCall->counter++];
                string method = code->exprs[stringIndex]->toString();
                int argCount = code->code[funcCall->counter++];
                ClassDef* baseClass = (ClassDef*)pop()->object;
                if(!superCall(baseClass, method, argCount)){
                    return RUNTIME_ERROR;
                }
                funcCall = calls.back();
                break;
            }
            case FUNCTION_DEF_OP: {
                int i = code->code[funcCall->counter++];
                Expr* name = code->exprs[i];
                Function* func = (Function*) name->object;
                FunctionDef* funcDef = new FunctionDef(func);
                push(new Expr(funcDef));

                for(int i=0; i<funcDef->nonlocals.size(); i++){
                    int isLocal = code->code[funcCall->counter++];
                    int index = code->code[funcCall->counter++];
                    if(isLocal){
                        funcDef->nonlocals[i] = findNonlocal(stack[index+funcCall->start]);
                    }
                    else{
                        funcDef->nonlocals[i] = funcCall->funcDef->nonlocals[index+funcCall->start];
                    }
                }
                break;
            }
            case DEL_NONLOCAL_OP: {
                delNonlocals(stack.back());
                stack.pop_back();
                break;
            }

            case RETURN_OP: {
                Expr* result = pop();
                delNonlocals(stack[funcCall->start]);
                calls.pop_back();
                if(calls.size() == 0){
                    stack.pop_back();
                    return OK;
                }

                for(int i=stack.size()-1; i>=funcCall->start; i--){
                    stack.pop_back();
                }
                
                push(result);
                funcCall = calls.back();
                break;
            }
            case CLASS_DEF_OP: {
                int stringIndex = code->code[funcCall->counter++];
                string name = code->exprs[stringIndex]->toString();
                push(new Expr(new ClassDef(name)));
                break;
            }
            case BASE_OP: {
                Expr* baseClass = stack[stack.size()-2];
                if(baseClass->type != OBJECT_EXPR || baseClass->object->type != CLASS_DEF){
                    throwError("super class does not exists");
                    return RUNTIME_ERROR;
                }

                ClassDef* subclass = (ClassDef*)stack.back()->object;
                ClassDef* super = (ClassDef*)baseClass->object;
                for(auto e : super->methods){
                    subclass->methods[e.first] = super->methods[e.first];
                }
                for(auto e : super->fields){
                    subclass->fields[e.first] = super->fields[e.first];
                }
                stack.pop_back();
                break;
            }
            case FIELD_DEF_OP: {
                int stringIndex = code->code[funcCall->counter++];
                string name = code->exprs[stringIndex]->toString();
                fieldDef(name);
                break;
            }
            case METHOD_DEF_OP:{
                int stringIndex = code->code[funcCall->counter++];
                string name = code->exprs[stringIndex]->toString();
                methodDef(name);
                break;
            }
        }

        if(threads.size() == 1){
            runGC();
        }
    }
}
ExitCode VirtualMachine::readByteCode(string fileName, string sourceCode){
    Function* function = runCompiler(fileName, sourceCode);
    if(function == NULL) return COMPILE_ERROR;
    push(new Expr(function));
    
    FunctionDef* funcDef = new FunctionDef(function);
    stack.pop_back();
    push(new Expr(funcDef));
    funcCall(funcDef, 0);
    
    return start();
}

void VirtualMachine::markObject(Object* object){
    if(object->marked) return;

    object->marked = true;
    
    garbageCollector.push_back(object);
}
void VirtualMachine::markExpr(Expr* name){
    if(name->type == OBJECT_EXPR) markObject(name->object);
}
void VirtualMachine::markVector(vector<Expr*> vec){
    for(Expr* v : vec) markExpr(v);
}
void VirtualMachine::markMap(ska::flat_hash_map<string, Expr*> map){
    for(auto pair : map){
        markExpr(pair.second);
    }
}
void VirtualMachine::trace(){
    while(garbageCollector.size() > 0){
        Object* object = garbageCollector.back();
        garbageCollector.pop_back();

        switch(object->type){
            case METHOD: {
                Method* bound = (Method*)object;
                markExpr(bound->classDef);
                markObject(bound->method);
                break;
            }
            case CLASS_DEF: {
                ClassDef* classDef = (ClassDef*)object;
                markMap(classDef->methods);
                markMap(classDef->fields);
                break;
            }
            case FUNCTION_DEF: {
                FunctionDef* funcDef = (FunctionDef*)object;
                markObject(funcDef->function);
                for(NonlocalObj* nonlocal : funcDef->nonlocals) markObject(nonlocal);
                break;
            }
            case FUNCTION: {
                Function* func = (Function*)object;
                markVector(func->code->exprs);
                break;
            }
            case INSTANCE: {
                Instance* instance = (Instance*)object;
                markObject(instance->classPtr);
                markMap(instance->fields);
                break;
            }
            case NONLOCAL: markExpr(((NonlocalObj*)object)->deleted); break;
            case BUILT_IN: break;
            case ITER: {
                Iterator* iter = (Iterator*)object;
                switch(iter->itertype){
                    case ITER_LIST: {
                        List* list = (List*)iter;
                        markVector(list->list);
                        break;
                    }
                    case ITER_TUPLE: {
                        Tuple* tuple = (Tuple*)iter;
                        markVector(tuple->tuple);
                        break;
                    }
                    case ITER_SET: {
                        Set* set = (Set*)iter;
                        for(auto e : set->data){
                            markExpr(e);
                        }
                        break;
                    }
                    case ITER_DICT: {
                        Dict* dict = (Dict*)iter;
                        for(auto pair : dict->dict){
                            markExpr(pair.first);
                            markExpr(pair.second);
                        }
                        break;
                    }
                }
                break;
            }
            case FILE_OBJ: break;
            case THREAD: {
                Thread* thread = (Thread*)object;
                markObject(thread->funcDef);
                if(thread->args != NULL) markObject(thread->args);
                break;
            }
            case LOCK:
            case SOCKET:
            case LIST_ITER:
            case SET_ITER:
            case DICT_ITER: break;
        }
    }    
}
void VirtualMachine::clearObject(Object* object){
    switch(object->type){
        case METHOD: delete ((Method*)object); break;
        case CLASS_DEF: {
            ClassDef* classDef = (ClassDef*)object;
            classDef->methods.clear();
            classDef->fields.clear();
            delete classDef;
            break;
        }
        case FUNCTION_DEF: {
            FunctionDef* funcDef = (FunctionDef*)object;
            funcDef->nonlocals.clear();
            delete funcDef;
            break;
        }
        case FUNCTION: {
            Function* func = (Function*)object;
            func->code->clear();
            delete func;
            break;
        }
        case INSTANCE: {
            Instance* instance = (Instance*)object;
            instance->fields.clear();
            delete instance;
            break;
        }
        case BUILT_IN: delete ((BuiltIn*)object); break;
        case NONLOCAL: delete((NonlocalObj*)object); break;
        case ITER: {
            Iterator* iter = (Iterator*)object;
            switch(iter->itertype){
                case ITER_LIST:{
                    List* list = (List*)iter;
                    list->list.clear();
                    delete list;
                    break;
                }
                case ITER_TUPLE:{
                    Tuple* tuple = (Tuple*)iter;
                    tuple->tuple.clear();
                    delete tuple;
                    break;
                }
                case ITER_SET:{
                    Set* set = (Set*)iter;
                    set->data.clear();
                    delete set;
                    break;
                }
                case ITER_DICT:{
                    Dict* dict = (Dict*)iter;
                    dict->dict.clear();
                    delete dict;
                    break;
                }
            }
            break;
        }
        case FILE_OBJ: delete ((File*)object); break;
        case THREAD: delete ((Thread*)object); break;
        case LOCK: delete ((Lock*)object); break;
        case SOCKET: delete ((Socket*)object); break;
        case LIST_ITER: delete ((ListIter*)object); break;
        case SET_ITER: delete ((SetIter*)object); break;
        case DICT_ITER: delete ((DictIter*)object); break;
    }
}
void VirtualMachine::mark(){
    for(Expr* name : stack) markExpr(name);
    for(Call* funcCall : calls) markObject(funcCall->funcDef);
    for(NonlocalObj* nonlocal : nonlocals) markObject(nonlocal);

    markMap(*variables);
}
void VirtualMachine::sweep(){
    set<Object*> cpObjects;
    for(auto object : objects){
        cpObjects.insert(object);
    }

    for(auto object : cpObjects){
        if(object->marked) object->marked = false;
        else {
            objects.erase(object);
            clearObject(object);
        }
    }

    cpObjects.clear();
}
void VirtualMachine::runGC(){

    mark();
    trace();
    sweep();

}
void VirtualMachine::clearObjects(){
    for(Object* object : objects){
        clearObject(object);
    }

    objects.clear();
    garbageCollector.clear();
}