#include "types.hpp"
#include <iostream>
#include <string>
#include <vector>
using namespace std;

Object::Object(ObjectType type){
    this->type = type;
    marked = false; 
}

Expr::Expr(){
    type = NONE_EXPR;
    number = 0;
} 
Expr::Expr(bool value){
    type = BOOL_EXPR;
    boolean = value;
}
Expr::Expr(double value){
    type = NUMBER_EXPR;
    number = value;
}
Expr::Expr(string s){
    type = STRING_EXPR;
    str = s;
}
Expr::Expr(Object* o){
    type = OBJECT_EXPR;
    object = o;
}
Expr::Expr(ExprType type, string s){
    this->type = type;
    str = s;
}
string Expr::toString(){
    switch(type){
        case BOOL_EXPR: return (boolean ? "True" : "False");
        case NONE_EXPR: return "None";
        case NUMBER_EXPR: {
            ostringstream strNumber;
            strNumber << number;
            return strNumber.str();
        }
        case STRING_EXPR: return str;
        case ERROR_EXPR: return str;
        case OBJECT_EXPR: {
            switch(object->type){
                case FUNCTION: return ((Function*)object)->toString();
                case METHOD: return ((Method*)object)->toString();
                case CLASS_DEF: return ((ClassDef*)object)->toString(); 
                case FUNCTION_DEF: return ((FunctionDef*)object)->toString(); 
                case INSTANCE: return ((Instance*)object)->toString(); 
                case BUILT_IN: return ((BuiltIn*)object)->toString(); 
                case NONLOCAL: return ((NonlocalObj*)object)->toString();
                case ITER: return ((Iterator*)object)->toString();
                case FILE_OBJ: return ((File*)object)->toString();
                case THREAD: return ((Thread*)object)->toString();
                case LOCK: return ((Lock*)object)->toString();
                case SOCKET: return ((Socket*)object)->toString();
                case LIST_ITER: return ((ListIter*)object)->toString();
                case SET_ITER: return ((SetIter*)object)->toString();
                case DICT_ITER: return ((DictIter*)object)->toString();
            }
        }
        default: return "";
    }
}
bool Expr::equals(Expr* value){
    if(type != value->type) return false;
    
    switch(type){
        case BOOL_EXPR: return boolean == value->boolean;
        case NONE_EXPR: return true;
        case NUMBER_EXPR: return number == value->number;
        case STRING_EXPR: return str == value->str;
        case OBJECT_EXPR: {
            if(object->type != value->object->type) return false;
            switch(object->type){
                case LIST_ITER: {
                    ListIter* iter1 = (ListIter*)object;
                    ListIter* iter2 = (ListIter*)value->object;
                    return iter1->it == iter2->it;
                }
                case SET_ITER: {
                    SetIter* iter1 = (SetIter*)object;
                    SetIter* iter2 = (SetIter*)value->object;
                    return iter1->it == iter2->it;
                }
                case DICT_ITER: {
                    DictIter* iter1 = (DictIter*)object;
                    DictIter* iter2 = (DictIter*)value->object;
                    return iter1->it == iter2->it;
                }
                default: return object == value->object;
            }
        }
        default: return false;
    }
}
bool Expr::greater(Expr* value){
    switch(type){
        case BOOL_EXPR: return boolean > value->boolean;
        case NONE_EXPR: return false;
        case NUMBER_EXPR: return number > value->number;
        case STRING_EXPR: return str > value->str;
        case OBJECT_EXPR: return object > value->object;
        default: return false;
    }
}
bool Expr::less(Expr* value){
    switch(type){
        case BOOL_EXPR: return boolean < value->boolean;
        case NONE_EXPR: return false;
        case NUMBER_EXPR: return number < value->number;
        case STRING_EXPR: return str < value->str;
        case OBJECT_EXPR: return object < value->object;
        default: return false;
    }
}

void ByteCode::clear(){
    code.clear();
    exprs.clear();
    lines.clear();
}
void ByteCode::generateCode(int opcode, int line){
    code.push_back(opcode);
    lines.push_back(line);
}
int ByteCode::storeExpr(Expr* value){
    exprs.push_back(value);
    return exprs.size()-1;
}

Method::Method(Expr* classDef, FunctionDef* method): Object(METHOD){
    this->classDef = classDef;
    this->method = method;
}
ClassDef::ClassDef(string name): Object(CLASS_DEF){
    this->name = name;
}
FunctionDef::FunctionDef(Function* function): Object(FUNCTION_DEF){
    for(int i=0; i < function->nonlocalCount; i++){
        nonlocals.push_back(NULL);
    }

    this->function = function;
}
Function::Function(string returnType): Object(FUNCTION){
    code = new ByteCode();
    argCount = 0;
    nonlocalCount = 0;
    this->returnType = returnType;
    name = "";
}
Instance::Instance(ClassDef* classPtr): Object(INSTANCE){
    this->classPtr = classPtr;
}
BuiltIn::BuiltIn(function<Expr*(vector<Expr*>)> func): Object(BUILT_IN){
    this->func = func;
}
NonlocalObj::NonlocalObj(Expr* scope): Object(NONLOCAL){
    deleted = new Expr();
    this->scope = scope;
}
string Function::toString(){
    if(name == "") return "<script>";
    return "<fn " + name + ">";
}
string Method::toString(){
    return method->function->toString();
}
string ClassDef::toString(){
    return name;
}
string FunctionDef::toString(){
    return function->toString();
}
string Instance::toString(){
    return classPtr->name + " instance";
}
string BuiltIn::toString(){
    return "<native fn>";
}
string NonlocalObj::toString(){
    return "upvalue";
}

Iterator::Iterator(IterType type): Object(ITER){
    itertype = type;
}
List::List(vector<Expr*> list): Iterator(ITER_LIST){
    this->list = list;
}
Tuple::Tuple(const vector<Expr*> tuple): Iterator(ITER_TUPLE){
    this->tuple = tuple;
}
Set::Set(set<Expr*> data): Iterator(ITER_SET){
    this->data = data;
}
Dict::Dict(map<Expr*, Expr*> dict): Iterator(ITER_DICT){
    this->dict = dict;
}

string Iterator::toString(){
    switch(itertype){
        case ITER_LIST: return ((List*)this)->toString();
        case ITER_TUPLE: return ((Tuple*)this)->toString();
        case ITER_SET: return ((Set*)this)->toString();
        case ITER_DICT: return ((Dict*)this)->toString();
        default: return "";
    }
}
string List::toString(){
    if(list.size() == 0) return "[]";

    string str = "";
    str += "[";
    int i=0;
    while(i < list.size()-1){
        str += list[i]->toString() + ", ";
        i++;
    }
    str += list.back()->toString() + "]";

    return str;
}
string Tuple::toString(){
    if(tuple.size() == 0) return "()";

    string str = "";
    str += "(";
    int i=0;
    while(i < tuple.size()-1){
        str += tuple[i]->toString() + ", ";
        i++;
    }
    str += tuple.back()->toString() + ")";

    return str;
}
string Set::toString(){
    if(data.size() == 0) return "{}";

    string str = "";
    str += "{";
    auto i=data.begin();
    auto end = --data.end();
    while(i != end){
        str += (*i)->toString() + ", ";
        ++i;
    }
    str += (*i)->toString() + "}";

    return str;
}
string Dict::toString(){
    if(dict.size() == 0) return "{}";

    string str = "";
    str += "{";
    auto i=dict.begin();
    auto end = --dict.end();
    while(i != end){
        str += i->first->toString() + ": ";
        str += i->second->toString() + ", ";
        ++i;
    }
    str += i->first->toString() + ": ";
    str += i->second->toString() + "}";

    return str;
}

File::File(string name, string fileType): Object(FILE_OBJ){
    fileName = name;

    if(fileType == "r") file.open(name, fstream::in);
    else if(fileType == "w") file.open(name, fstream::out);
    else file.open(name, fstream::app);
}
string File::toString(){
    return fileName;
}

Thread::Thread(FunctionDef* funcDef): Object(THREAD){
    this->funcDef = funcDef;
    args = NULL;
}

string Thread::toString(){
    return "<Thread " + funcDef->function->name + ">";
}

int Lock::id = 0;
Lock::Lock(): Object(LOCK){
    id++;
}
string Lock::toString(){
    return "<Lock " + to_string(id) + ">";
}

Socket::Socket(): Object(SOCKET){}
string Socket::toString(){
    return "<Socket " + to_string(fd) + ">";
}

ListIter::ListIter(vector<Expr*>::iterator it): Object(LIST_ITER){
    this->it = it;
}
string ListIter::toString(){
    return "<ListIter>";
}

SetIter::SetIter(set<Expr*>::iterator it): Object(SET_ITER){
    this->it = it;
}
string SetIter::toString(){
    return "<SetIter>";
}

DictIter::DictIter(map<Expr*, Expr*>::iterator it): Object(DICT_ITER){
    this->it = it;
}
string DictIter::toString(){
    return "<DictIter>";
}