#include "compiler.hpp"
#include <string>
#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
using namespace std;

Parser parsers[] = {
    Parser(parseParen, parseCall, CALL_ORDER),        //LEFT_PAREN
    Parser(NULL, NULL, NONE_ORDER),                   //RIGHT_PAREN
    Parser(parseSet, NULL, CALL_ORDER),               //LEFT_BRACE
    Parser(NULL, NULL, NONE_ORDER),                   //RIGHT_BRACE
    Parser(NULL, NULL, NONE_ORDER),                   //COMMA
    Parser(NULL, parseDot, CALL_ORDER),               //DOT
    Parser(parseUnary, parseBinOp, TERM_ORDER),       //MINUS
    Parser(NULL, parseBinOp, TERM_ORDER),             //PLUS
    Parser(NULL, NULL, NONE_ORDER),                   //SEMICOLON
    Parser(NULL, parseBinOp, FACTOR_ORDER),           //SLASH
    Parser(NULL, parseBinOp, FACTOR_ORDER),           //STAR
    Parser(parseUnary, NULL, NONE_ORDER),             //BANG
    Parser(NULL, parseBinOp, EQ_ORDER),               //BANG_EQUAL
    Parser(NULL, NULL, NONE_ORDER),                   //EQUAL
    Parser(NULL, parseBinOp, EQ_ORDER),               //EQUAL_EQUAL
    Parser(NULL, parseBinOp, COMPARISON_ORDER),       //GREATER
    Parser(NULL, parseBinOp, COMPARISON_ORDER),       //GREATER_EQUAL
    Parser(NULL, parseBinOp, COMPARISON_ORDER),       //LESS
    Parser(NULL, parseBinOp, COMPARISON_ORDER),       //LESS_EQUAL
    Parser(parseIdentifier, NULL, NONE_ORDER),        //IDENTIFIER
    Parser(parseString, NULL, NONE_ORDER),            //STRING
    Parser(parseNumber, NULL, NONE_ORDER),            //NUMBER
    Parser(NULL, parseAnd, AND_ORDER),                //AND 
    Parser(NULL, NULL, NONE_ORDER),                   //CLASS
    Parser(NULL, NULL, NONE_ORDER),                   //ELSE  
    Parser(parseLiteral, NULL, NONE_ORDER),           //FALSE
    Parser(NULL, NULL, NONE_ORDER),                   //FOR
    Parser(NULL, NULL, NONE_ORDER),                   //FUN
    Parser(NULL, parseIfExp, COND_ORDER),             //IF
    Parser(parseLiteral, NULL, NONE_ORDER),           //NONE
    Parser(NULL, parseOr, OR_ORDER),                  //OR
    Parser(NULL, NULL, NONE_ORDER),                   //RETURN
    Parser(parseSuper, NULL, NONE_ORDER),             //SUPER
    Parser(parseSelf, NULL, NONE_ORDER),              //SELF
    Parser(parseLiteral, NULL, NONE_ORDER),           //TRUE
    Parser(NULL, NULL, NONE_ORDER),                   //DATA_TYPE
    Parser(NULL, NULL, NONE_ORDER),                   //WHILE
    Parser(NULL, NULL, NONE_ORDER),                   //ERROR
    Parser(NULL, NULL, NONE_ORDER),                   //TOKEN_EOF
    Parser(NULL, NULL, NONE_ORDER),                   //NEWLINE
    Parser(NULL, NULL, NONE_ORDER),                   //INDENT
    Parser(NULL, NULL, NONE_ORDER),                   //DEDENT
    Parser(NULL, NULL, NONE_ORDER),                   //COLON
    Parser(NULL, NULL, NONE_ORDER),                   //AS
    Parser(NULL, NULL, NONE_ORDER),                   //ASSERT
    Parser(NULL, NULL, NONE_ORDER),                   //ASYNC
    Parser(NULL, NULL, AWAIT_ORDER),                  //AWAIT
    Parser(NULL, NULL, NONE_ORDER),                   //BREAK
    Parser(NULL, NULL, NONE_ORDER),                   //CONT
    Parser(NULL, NULL, NONE_ORDER),                   //DEL
    Parser(NULL, NULL, NONE_ORDER),                   //ELIF
    Parser(NULL, NULL, NONE_ORDER),                   //EXCEPT
    Parser(NULL, NULL, NONE_ORDER),                   //FINALLY
    Parser(NULL, NULL, NONE_ORDER),                   //FROM
    Parser(NULL, NULL, NONE_ORDER),                   //GLOBAL
    Parser(NULL, NULL, NONE_ORDER),                   //IMPORT
    Parser(NULL, parseBinOp, COMPARISON_ORDER),       //IN
    Parser(NULL, parseBinOp, COMPARISON_ORDER),       //NOT_IN
    Parser(NULL, parseBinOp, COMPARISON_ORDER),       //IS
    Parser(NULL, parseBinOp, COMPARISON_ORDER),       //IS_NOT
    Parser(NULL, NULL, LAMBDA_ORDER),                 //LAMBDA
    Parser(parseUnary, NULL, NOT_ORDER),              //NOT
    Parser(NULL, NULL, NONE_ORDER),                   //PASS
    Parser(NULL, NULL, NONE_ORDER),                   //RAISE
    Parser(NULL, NULL, NONE_ORDER),                   //TRY
    Parser(NULL, NULL, NONE_ORDER),                   //WITH
    Parser(NULL, NULL, NONE_ORDER),                   //YIELD
    Parser(parseList, parseSubscript, CALL_ORDER),    //LEFT_BRACKET
    Parser(NULL, NULL, NONE_ORDER),                   //RIGHT_BRACKET
    Parser(NULL, parseBinOp, EXP_ORDER),              //STAR_STAR
    Parser(NULL, parseBinOp, FACTOR_ORDER),           //SLASH_SLASH
    Parser(NULL, parseBinOp, FACTOR_ORDER),           //PERCENT
    Parser(NULL, parseBinOp, FACTOR_ORDER),           //AT
    Parser(NULL, parseBinOp, SHIFT_ORDER),            //LEFT_SHIT
    Parser(NULL, parseBinOp, SHIFT_ORDER),            //RIGHT_SHIFT
    Parser(NULL, parseBinOp, BIT_AND_ORDER),          //BIT_AND
    Parser(NULL, parseBinOp, BIT_OR_ORDER),           //BIT_OR
    Parser(NULL, parseBinOp, BIT_XOR_ORDER),          //BIT_XOR
    Parser(parseUnary, NULL, UNARY_ORDER),            //BIT_NOT
    Parser(NULL, NULL, NONE_ORDER),                   //PLUS_EQUAL
    Parser(NULL, NULL, NONE_ORDER),                   //MINUS_EQUAL
    Parser(NULL, NULL, NONE_ORDER),                   //STAR_EQUAL
    Parser(NULL, NULL, NONE_ORDER),                   //SLASH_EQUAL
    Parser(NULL, NULL, NONE_ORDER),                   //SLASH_SLASH_EQUAL
    Parser(NULL, NULL, NONE_ORDER),                   //PERCENT_EQUALS
    Parser(NULL, NULL, NONE_ORDER),                   //AT_EQUAL
    Parser(NULL, NULL, NONE_ORDER),                   //BIT_AND_EQUAL
    Parser(NULL, NULL, NONE_ORDER),                   //BIT_OR_EQUAL
    Parser(NULL, NULL, NONE_ORDER),                   //BIT_XOR_EQUAL
    Parser(NULL, NULL, NONE_ORDER),                   //LEFT_SHITF_EQUAL
    Parser(NULL, NULL, NONE_ORDER),                   //RIGHT_SHIFT_EQUAL
    Parser(NULL, NULL, NONE_ORDER),                   //STAR_STAR_EQUAL
    Parser(parseFString, NULL, NONE_ORDER),           //FORMATTED_STRING
};

vector<Lexer*> lexers;
vector<string> sourceCodeNames;
Reader* reader = new Reader();
vector<FuncParser*> functions;
vector<ClassParser*> classes;
ska::flat_hash_map<string, ClassParser*> classMap;
ska::flat_hash_map<string, string> superMap;

string lastCall;
int lastCallIndex;
Lexem* lastCallLexem;

ska::flat_hash_map<string, string> globals;

set<string> classTypes;

ByteCode* getCode(){
    return functions.back()->func->code;
}
void handleError(Lexem* lexem, string message){
    if(reader->handelingError) return;

    reader->handelingError = true;
    cerr << "[line " << lexem->line << " in file " << sourceCodeNames.back() << "] Error";

    if(lexem->type == TOKEN_EOF){
        cerr << " at end";
    }
    else{
        cerr << " at '" << lexem->value << "'";
    }

    cerr << ": " << message << endl;
    reader->error = true;
}
void nextLexem(){
    reader->prev = reader->next;
    while(true){
        reader->next = lexers.back()->loadLexem();
        if(reader->next->type != ERROR) break;
        handleError(reader->next, reader->next->value);
    }
}
void read(LexemType type, string message){
    if(reader->next->type == type){
        nextLexem();
        return;
    }

    if(type == NEWLINE && reader->next->type == SEMICOLON){
        nextLexem();
        return;
    }

    handleError(reader->next, message);
}
bool peek(LexemType type){
    if(reader->next->type == type){
        nextLexem();
        return true;
    }
    return false;
}
void generateLoop(int loopStart){
    getCode()->generateCode(LOOP_OP, reader->prev->line);
    int offset = getCode()->code.size() - loopStart;
    getCode()->generateCode(offset, reader->prev->line);
}
int generateJump(int jump){
    getCode()->generateCode(jump, reader->prev->line);
    getCode()->generateCode(0xff, reader->prev->line);
    return getCode()->code.size();
}
void generateReturn(){
    if(functions.back()->type == INIT_FUNC){
        getCode()->generateCode(LOAD_NAME_OP, reader->prev->line);
        getCode()->generateCode(0, reader->prev->line);
    }
    else{
        getCode()->generateCode(NONE_OP, reader->prev->line);
    }

    getCode()->generateCode(RETURN_OP, reader->prev->line);
}
void generateConstant(Expr* value){
    getCode()->generateCode(CONSTANT_OP, reader->prev->line);
    int valIndex = getCode()->storeExpr(value);  
    getCode()->generateCode(valIndex, reader->prev->line);
}
void generateJumpOffset(int jumpStart){
    int offset = getCode()->code.size() - jumpStart;
    getCode()->code[jumpStart-1] = offset;
}
FuncParser::FuncParser(FuncType type, string returnType){
    func = NULL;
    this->type = type;
    scopeCounter = 0;
    func = new Function(returnType);
    functions.push_back(this);

    if(type != SCRIPT_FUNC){
        functions.back()->func->name = reader->prev->value;
    }

    Name* name;
    if(type != FUNC_FUNC && type != SCRIPT_FUNC){
        name = new Name(classes.back()->className->value);
        name->id->value = "self";
    }
    else{
        name = new Name("Object");
        name->id->value = "";
    }

    names.push_back(name);
}
ClassParser::ClassParser(Lexem* className){
    this->className = className;
    hasBase = false;
}
void optimizePropagation(Function* func){
    ska::flat_hash_map<int, int> propagationVarMap;
    ska::flat_hash_map<int, vector<int>> reverseVarMap;

    ska::flat_hash_map<int, int> propagationUpMap;
    ska::flat_hash_map<int, vector<int>> reverseUpMap;
    
    for(int i=0; i<func->code->code.size(); i++){
        int OpType = func->code->code[i];
        switch(OpType){
            case CONSTANT_OP: i++; break;
            case NONE_OP: 
            case TRUE_OP: 
            case FALSE_OP: 
            case POP_OP: break;
            case LOAD_NAME_OP: {
                int index = func->code->code[i+1];
                if(propagationVarMap.find(index) != propagationVarMap.end()){
                    func->code->code[i+1] = propagationVarMap[index];
                }

                i++;
                break;
            }
            case STORE_NAME_OP:{
                int index = func->code->code[i+1];
                if(propagationVarMap.find(index) != propagationVarMap.end()){
                    propagationVarMap.erase(index);
                }
                else if(reverseVarMap.find(index) != reverseVarMap.end()){
                    for(int i : reverseVarMap[index]){
                        propagationVarMap.erase(i);
                    }
                    reverseVarMap.erase(index);
                }

                int priorOp = func->code->code[i-2];
                if(priorOp == LOAD_NAME_OP && func->code->code[i-3] != CONSTANT_OP){
                    int getIndex = func->code->code[i-1];
                    while(propagationVarMap.find(getIndex) != propagationVarMap.end()){
                        getIndex = propagationVarMap[getIndex];
                    }
                    propagationVarMap[index] = getIndex;
                    reverseVarMap[getIndex].push_back(index);
                }

                i++; 
                break;
            }
            case LOAD_GLOBAL_OP: i++; break;
            case ASSIGN_GLOBAL_OP: i++; break;
            case STORE_GLOBAL_OP: i++; break;
            case LOAD_NONLOCAL_OP: {
                int index = func->code->code[i+1];
                if(propagationUpMap.find(index) != propagationUpMap.end()){
                    func->code->code[i+1] = propagationUpMap[index];
                }

                i++;
                break;
            }
            case STORE_NONLOCAL_OP: {
                int index = func->code->code[i+1];
                if(propagationUpMap.find(index) != propagationUpMap.end()){
                    propagationUpMap.erase(index);
                }
                else if(reverseUpMap.find(index) != reverseUpMap.end()){
                    for(int i : reverseUpMap[index]){
                        propagationUpMap.erase(i);
                    }
                    reverseUpMap.erase(index);
                }

                int priorOp = func->code->code[i-2];
                if(priorOp == LOAD_NAME_OP && func->code->code[i-3] != CONSTANT_OP){
                    int getIndex = func->code->code[i-1];
                    while(propagationUpMap.find(getIndex) != propagationUpMap.end()){
                        getIndex = propagationUpMap[getIndex];
                    }
                    propagationUpMap[index] = getIndex;
                    reverseUpMap[getIndex].push_back(index);
                }

                i++; 
                break;
            }
            case LOAD_ATTRIBUTE_OP: i++; break;
            case STORE_ATTRIBUTE_OP: i++; break;
            case LOAD_SUPER_OP: i++; break;
            case EQ_OP: 
            case GT_OP:
            case LT_OP: 
            case IS_OP: 
            case IN_OP: 
            case ADD_OP:
            case SUB_OP:
            case MULT_OP:
            case DIV_OP: 
            case FLOOR_DIV_OP:
            case MOD_OP: 
            case POW_OP: 
            case LSHIFT_OP:
            case RSHIFT_OP:
            case BIT_OR_OP:
            case BIT_XOR_OP:
            case BIT_AND_OP:
            case NOT_OP: 
            case BIT_NOT_OP:
            case INVERT_OP: break;
            case JUMP_OP: 
            case JUMP_IF_FALSE_OP: 
            case LOOP_OP: 
            case CALL_OP: i++; break;
            case METHOD_CALL_OP: 
            case SUPER_CALL_OP: i++; i++; break;
            case FUNCTION_DEF_OP: i++; i++; i++; break;
            case DEL_NONLOCAL_OP: 
            case RETURN_OP: break;
            case CLASS_DEF_OP: i++; break;
            case BASE_OP: break;
            case FIELD_DEF_OP:
            case METHOD_DEF_OP: 
            case LIST_OP: 
            case TUPLE_OP: 
            case SET_OP: 
            case DICT_OP: i++; break;
            case IF_EXP_OP: 
            case LOAD_SLICE_OP: 
            case LOAD_SUBSCRIPT_OP:
            case STORE_SLICE_OP: 
            case STORE_SUBSCRIPT_OP:
            case DEL_SLICE_OP: 
            case DEL_SUBSCRIPT_OP: break;
            case LOAD_ITER_START_OP:
            case LOAD_ITER_STOP_OP:
            case LOAD_ITER_OP:
            case INCREMENT_ITER_OP: i++; break;
        }
    }
}
void optimizeDeletion(Function* func){
    ska::flat_hash_map<int, int> lastVarInstruction;
    ska::flat_hash_map<int, int> lastVarInstructionIndex;
    
    ska::flat_hash_map<int, int> lastUpInstruction;
    ska::flat_hash_map<int, int> lastUpInstructionIndex;

    vector<int> markedForDeletion;
    for(int i=0; i<func->code->code.size(); i++){
        int OpType = func->code->code[i];
        switch(OpType){
            case CONSTANT_OP: i++; break;
            case NONE_OP: 
            case TRUE_OP: 
            case FALSE_OP: 
            case POP_OP: break;
            case LOAD_NAME_OP: {
                int index = func->code->code[i+1];
                lastVarInstruction[index] = LOAD_NAME_OP;
                lastVarInstructionIndex[index] = i;
                i++;
                break;
            }
            case STORE_NAME_OP:{
                int index = func->code->code[i+1];
                if(lastVarInstruction.find(index) != lastVarInstruction.end() && lastVarInstruction[index] == STORE_NAME_OP){
                    markedForDeletion.push_back(lastVarInstructionIndex[index]);
                }

                lastVarInstruction[index] = STORE_NAME_OP;
                lastVarInstructionIndex[index] = i;
                i++; 
                break;
            }
            case LOAD_GLOBAL_OP: i++; break;
            case ASSIGN_GLOBAL_OP: i++; break;
            case STORE_GLOBAL_OP: i++; break;
            case LOAD_NONLOCAL_OP: {
                int index = func->code->code[i+1];
                lastUpInstruction[index] = LOAD_NONLOCAL_OP;
                lastUpInstructionIndex[index] = i;
                i++;
                break;
            }
            case STORE_NONLOCAL_OP: {
                int index = func->code->code[i+1];
                if(lastUpInstruction.find(index) != lastUpInstruction.end() && lastUpInstruction[index] == STORE_NONLOCAL_OP){
                    markedForDeletion.push_back(lastUpInstructionIndex[index]);
                }

                lastUpInstruction[index] = STORE_NONLOCAL_OP;
                lastUpInstructionIndex[index] = i;
                i++; 
                break;
            }
            case LOAD_ATTRIBUTE_OP: i++; break;
            case STORE_ATTRIBUTE_OP: i++; break;
            case LOAD_SUPER_OP: i++; break;
            case EQ_OP: 
            case GT_OP:
            case LT_OP: 
            case IS_OP: 
            case IN_OP: 
            case ADD_OP:
            case SUB_OP:
            case MULT_OP:
            case DIV_OP: 
            case FLOOR_DIV_OP:
            case MOD_OP: 
            case POW_OP: 
            case LSHIFT_OP:
            case RSHIFT_OP:
            case BIT_OR_OP:
            case BIT_XOR_OP:
            case BIT_AND_OP:
            case NOT_OP: 
            case BIT_NOT_OP:
            case INVERT_OP: break;
            case JUMP_OP: 
            case JUMP_IF_FALSE_OP: 
            case LOOP_OP: 
            case CALL_OP: i++; break;
            case METHOD_CALL_OP: 
            case SUPER_CALL_OP: i++; i++; break;
            case FUNCTION_DEF_OP: i++; i++; i++; break;
            case DEL_NONLOCAL_OP: 
            case RETURN_OP: break;
            case CLASS_DEF_OP: i++; break;
            case BASE_OP: break;
            case FIELD_DEF_OP:
            case METHOD_DEF_OP: 
            case LIST_OP: 
            case TUPLE_OP: 
            case SET_OP: 
            case DICT_OP: i++; break;
            case IF_EXP_OP: 
            case LOAD_SLICE_OP: 
            case LOAD_SUBSCRIPT_OP:
            case STORE_SLICE_OP: 
            case STORE_SUBSCRIPT_OP:
            case DEL_SLICE_OP: 
            case DEL_SUBSCRIPT_OP: break;
            case LOAD_ITER_START_OP:
            case LOAD_ITER_STOP_OP:
            case LOAD_ITER_OP:
            case INCREMENT_ITER_OP: i++; break;
        }
    }

    if(markedForDeletion.size() > 0){
        vector<int> newCode;
        vector<int> newLines;
        int deletionCounter = 0;
        for(int i=0; i<func->code->code.size(); i++){
            if(i == markedForDeletion[deletionCounter]){
                while(newLines.back() >= func->code->lines[i]){
                    newCode.pop_back();
                    newLines.pop_back();
                }
                i++; // moves pass name index
                deletionCounter++;
            }
            else{
                newCode.push_back(func->code->code[i]);
                newLines.push_back(func->code->lines[i]);
            }
        }

        func->code->code = newCode;
        func->code->lines = newLines;
    }
}
Function* finish(){
    if(functions.back()->func->returnType != "None" && getCode()->code.back() != RETURN_OP){
        handleError(reader->prev, "Expected return statment");
        //return NULL;
    }
    generateReturn();

    Function* func = functions.back()->func;

    optimizePropagation(func);
    optimizeDeletion(func);

    functions.pop_back();
    return func;
}
void endBlock(){
    functions.back()->scopeCounter--;

    while(functions.back()->names.size() > 0 && functions.back()->names.back()->scope > functions.back()->scopeCounter){
        if(functions.back()->names.back()->caught){
            getCode()->generateCode(DEL_NONLOCAL_OP, reader->prev->line);
        }
        else{
            getCode()->generateCode(POP_OP, reader->prev->line);
        }

        functions.back()->names.pop_back();
    }
}
int FuncParser::loadName(Lexem* lexem){
    for(int i=names.size()-1; i >= 0; i--){
        Name* name = names[i];
        if(lexem->value == name->id->value){
            if(name->scope == -1) handleError(lexem, "Name not initalized");
            return i;
        }
    }
    return -1;
}
int ClassParser::loadName(Lexem* lexem){
    for(int i=names.size()-1; i >= 0; i--){
        Name* name = names[i];
        if(lexem->value == name->id->value){
            if(name->scope == -1) handleError(lexem, "Name not initalized");
            return i;
        }
    }
    return -1;
}

string lastNonlocalType;
int FuncParser::storeNonlocal(int index, bool notGlobal){
    int nonlocalCount = func->nonlocalCount;
    for(int i=0; i<nonlocalCount; i++){
        Nonlocal* nonlocal = nonlocals[i];
        if(nonlocal->index == index && nonlocal->notGlobal == notGlobal) return i;
    }

    Nonlocal* newNonlocal = new Nonlocal(lastNonlocalType);
    newNonlocal->notGlobal = notGlobal;
    newNonlocal->index = index;
    nonlocals.push_back(newNonlocal);
    return func->nonlocalCount++;
}
int loadNonlocal(Lexem* nonlocalName, int start){
    if(start == 0) return -1;

    int name = functions[start-1]->loadName(nonlocalName);
    if(name != -1){
        functions[start-1]->names[name]->caught = true;
        lastNonlocalType = functions[start-1]->names[name]->type;
        return functions[start]->storeNonlocal(name, true);
    }

    int nonlocalIndex = loadNonlocal(nonlocalName, start-1);
    if(nonlocalIndex != -1) return functions[start]->storeNonlocal(nonlocalIndex, false);

    return -1;
}
void FuncParser::storeName(Lexem* id, string type){
    Name* name = new Name(type);
    name->id = id;
    name->scope = -1;
    name->caught = false;
    names.push_back(name);
}
void ClassParser::storeName(Lexem* id, string type){
    Name* name = new Name(type);
    name->id = id;
    name->scope = -1;
    name->caught = false;
    names.push_back(name);
}
void FuncParser::assignName(string type){
    Lexem* id = reader->prev;
    for(int i=names.size()-1; i>=0; i--){
        Name* name = names[i];
        if(name->scope != -1 && name->scope < scopeCounter) break;
        if(id->value == name->id->value) handleError(reader->prev, "Name already declare");
    }
    storeName(id, type);
}
void ClassParser::assignName(string type){
    Lexem* id = reader->prev;
    for(int i=names.size()-1; i>=0; i--){
        Name* name = names[i];
        if(name->scope != -1) break;
        if(id->value == name->id->value) handleError(reader->prev, "Name already declare");
    }
    storeName(id, type);
}
bool checkType(){
    if(peek(DATA_TYPE)) return true;
    
    if(classTypes.find(reader->next->value) != classTypes.end()) {
        nextLexem();
        return true;
    }
    
    return false;
}
void readType(string message){
    if(reader->next->type == DATA_TYPE) nextLexem();
    else if(classTypes.find(reader->next->value) != classTypes.end()) nextLexem();
    else handleError(reader->next, message);
}
int assignName(string message, string type){
    read(IDENTIFIER, message);
    
    functions.back()->assignName(type);
    if(functions.back()->scopeCounter >= 0) return 0;
    
    return getCode()->storeExpr(new Expr(reader->prev->value));
}
void FuncParser::initName(){
    names.back()->scope = functions.back()->scopeCounter;
}
void ClassParser::initName(){
    names.back()->scope = 0;
}
int parseArgs(){
    int argCount = 0;
    if(reader->next->type != RIGHT_PAREN){
        do{
            parseExpr(ASSIGN_ORDER);
            argCount++;
        } while(peek(COMMA));
    }

    read(RIGHT_PAREN, "Expected ')' after args");
    return argCount;
}
int parseFuncArgs(int funcIndex){
    int argCount = 0;
    Name* name = functions.back()->names[funcIndex];
    if(reader->next->type != RIGHT_PAREN){
        do{
            string argtype = parseExpr(ASSIGN_ORDER);
            if(name->argTypes[argCount] != "Object"){
                if(argtype != name->argTypes[argCount]){
                    if(name->argTypes[argCount] == "float"){
                        if(argtype != "int"){
                            handleError(reader->prev, "Incorrect argument type");
                            return 0;
                        }
                    }
                    else{
                        handleError(reader->prev, "Incorrect argument type");
                        return 0;
                    }
                }
            }
            argCount++;
        } while(peek(COMMA));
    }

    read(RIGHT_PAREN, "Expected ')' after args");
    return argCount;
}
int parseMethodArgs(Name* name){
    int argCount = 0;
    if(name->argTypes.size() == 0) return parseArgs();
    
    if(reader->next->type != RIGHT_PAREN){
        do{
            string argtype = parseExpr(ASSIGN_ORDER);
            if(name->argTypes[argCount] != "Object"){
                if(argtype != name->argTypes[argCount]){
                    if(name->argTypes[argCount] == "float"){
                        if(argtype != "int"){
                            handleError(reader->prev, "Incorrect argument type");
                            return 0;
                        }
                    }
                    else{
                        handleError(reader->prev, "Incorrect argument type");
                        return 0;
                    }
                }
            }
            argCount++;
        } while(peek(COMMA));
    }

    read(RIGHT_PAREN, "Expected ')' after args");
    return argCount;
}
int parseInitArgs(){
    ClassParser* classParser = classMap[lastCall];
    int initIndex = classParser->loadName(new Lexem("__init__"));
    Name* name = classParser->names[initIndex];
    int argCount = 0;
    if(reader->next->type != RIGHT_PAREN){
        do{
            string argtype = parseExpr(ASSIGN_ORDER);
            if(name->argTypes[argCount] != "Object"){
                if(argtype != name->argTypes[argCount]){
                    if(name->argTypes[argCount] == "float"){
                        if(argtype != "int"){
                            handleError(reader->prev, "Incorrect argument type");
                            return 0;
                        }
                    }
                    else{
                        handleError(reader->prev, "Incorrect argument type");
                        return 0;
                    }
                }
            }
            argCount++;
        } while(peek(COMMA));
    }

    read(RIGHT_PAREN, "Expected ')' after args");
    return argCount;
}

string parseAnd(bool assign){
    int jumpStart = generateJump(JUMP_IF_FALSE_OP);
    getCode()->generateCode(POP_OP, reader->prev->line);
    parseExpr(AND_ORDER);
    generateJumpOffset(jumpStart);
    return "bool";
}
string priorType;
string parseBinOp(bool assign){
    LexemType type = reader->prev->type;
    string op1Type = priorType;
    
    Parser parser = parsers[type];
    string op2Type = parseExpr((Order)(parser.order+1));

    int index2 = getCode()->code[getCode()->code.size()-1];
    int op2 = getCode()->code[getCode()->code.size()-2];
    int index1 = getCode()->code[getCode()->code.size()-3];
    int op1 = getCode()->code[getCode()->code.size()-4];

    if(type == IN || type == NOT_IN){
        if(op2Type != "Object"){
            if(op2Type != "list" && op2Type != "tuple" && op2Type != "set" && op2Type != "dict"){
                handleError(reader->prev, "Expected iteratable");
                return "None";
            }
        }
    }
    else if(op1Type != "Object" && op2Type != "Object"){
        if(op1Type != op2Type){
            if(op1Type == "float"){
                if(op2Type != "int"){
                    handleError(reader->prev, "Type mismatch");
                    return "None";
                }
            }
            else if(op2Type == "float"){
                if(op1Type != "int"){
                    handleError(reader->prev, "Type mismatch");
                    return "None";
                }
            }
            else{
                handleError(reader->prev, "Type mismatch");
                return "None";
            }
        }
    }

    switch(type){
        case BANG_EQUAL: 
            if(op1Type != "Object" && op2Type != "Object"){
                if(op1Type != "str" && op1Type != "int" && op1Type != "float" && op1Type != "list" && op1Type != "tuple" && op1Type != "set" && op1Type != "dict"){
                    handleError(reader->prev, "Type mismatch: incompaitable types");
                    return "None";
                }
            }
            getCode()->generateCode(EQ_OP, reader->prev->line);
            getCode()->generateCode(NOT_OP, reader->prev->line);
            return "bool";

        case EQUAL_EQUAL: 
            if(op1Type != "Object" && op2Type != "Object"){
                if(op1Type != "str" && op1Type != "int" && op1Type != "float" && op1Type != "list" && op1Type != "tuple" && op1Type != "set" && op1Type != "dict"){
                    handleError(reader->prev, "Type mismatch: incompaitable types");
                    return "None";
                }
            }
            getCode()->generateCode(EQ_OP, reader->prev->line);
            return "bool";
        case GREATER: 
            if(op1Type != "Object" && op2Type != "Object"){
                if(op1Type != "str" && op1Type != "int" && op1Type != "float"){
                    handleError(reader->prev, "Type mismatch: incompaitable types");
                    return "None";
                }
            }
            getCode()->generateCode(GT_OP, reader->prev->line); 
            return "bool";
        case GREATER_EQUAL:
            if(op1Type != "Object" && op2Type != "Object"){
                if(op1Type != "str" && op1Type != "int" && op1Type != "float"){
                    handleError(reader->prev, "Type mismatch: incompaitable types");
                    return "None";
                }
            }
            getCode()->generateCode(LT_OP, reader->prev->line);
            getCode()->generateCode(NOT_OP, reader->prev->line);
            return "bool";

        case LESS: 
            if(op1Type != "Object" && op2Type != "Object"){
                if(op1Type != "str" && op1Type != "int" && op1Type != "float"){
                    handleError(reader->prev, "Type mismatch: incompaitable types");
                    return "None";
                }
            }
            getCode()->generateCode(LT_OP, reader->prev->line); 
            return "bool";
        case LESS_EQUAL:
            if(op1Type != "Object" && op2Type != "Object"){
                if(op1Type != "str" && op1Type != "int" && op1Type != "float"){
                    handleError(reader->prev, "Type mismatch: incompaitable types");
                    return "None";
                }
            }
            getCode()->generateCode(GT_OP, reader->prev->line);
            getCode()->generateCode(NOT_OP, reader->prev->line);
            return "bool";

        case IS: getCode()->generateCode(IS_OP, reader->prev->line); return "bool";
        case IS_NOT:
            getCode()->generateCode(IS_OP, reader->prev->line);
            getCode()->generateCode(NOT_OP, reader->prev->line); 
            return "bool";
        case IN: 
            getCode()->generateCode(IN_OP, reader->prev->line); 
            return "bool";
        case NOT_IN:
            getCode()->generateCode(IN_OP, reader->prev->line);
            getCode()->generateCode(NOT_OP, reader->prev->line); 
            return "bool";

        case PLUS: {
            if(op1Type != "Object" && op2Type != "Object"){
                if(op1Type != "str" && op1Type != "int" && op1Type != "float" && op1Type != "list" && op1Type != "tuple" && op1Type != "set"){
                    handleError(reader->prev, "Type mismatch: incompaitable types");
                    return "None";
                }
            }

            bool canShift = false;
            if(op1 == CONSTANT_OP && op2 == CONSTANT_OP){
                Expr* value2 = getCode()->exprs[index2];
                Expr* value1 = getCode()->exprs[index1];
                if(value1->type == NUMBER_EXPR && value2->type == NUMBER_EXPR){
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    double result = value1->number + value2->number;
                    generateConstant(new Expr(value1->number + value2->number));
                    
                    if(floor(result) == result) return "int";
                    return "float";
                }
                else if(value1->type == STRING_EXPR && value2->type == STRING_EXPR){
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    generateConstant(new Expr(value1->toString() + value2->toString()));
                    return "str";
                }
            }
            else if(op1Type == "int" && op2Type == "int"){
                if(op1 == op2 && index1 == index2){
                    canShift = true;
                }
            }


            if(canShift) {
                getCode()->code.pop_back();
                getCode()->code.pop_back();
                generateConstant(new Expr((double)1));
                getCode()->generateCode(LSHIFT_OP, reader->prev->line);
            }
            else getCode()->generateCode(ADD_OP, reader->prev->line); 
            break;
        }
        case MINUS: {
            if(op1Type != "Object" && op2Type != "Object"){
                if(op1Type != "int" && op1Type != "float"){
                    handleError(reader->prev, "Type mismatch: incompaitable types");
                    return "None";
                }
            }

            if(op1 == CONSTANT_OP && op2 == CONSTANT_OP){
                Expr* value2 = getCode()->exprs[index2];
                Expr* value1 = getCode()->exprs[index1];
                if(value1->type == NUMBER_EXPR && value2->type == NUMBER_EXPR){
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    generateConstant(new Expr(value1->number - value2->number));
                    break;
                }
            }

            getCode()->generateCode(SUB_OP, reader->prev->line); 
            break;
        }
        case STAR: {
            if(op1Type != "Object" && op2Type != "Object"){
                if(op1Type != "int" && op1Type != "float"){
                    handleError(reader->prev, "Type mismatch: incompaitable types");
                    return "None";
                }
            }

            bool canShift = false;
            if(op1 == CONSTANT_OP && op2 == CONSTANT_OP){
                Expr* value2 = getCode()->exprs[index2];
                Expr* value1 = getCode()->exprs[index1];
                if(value1->type == NUMBER_EXPR && value2->type == NUMBER_EXPR){
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    generateConstant(new Expr(value1->number * value2->number));
                    break;
                }
            }
            else if(op1 == CONSTANT_OP && op2Type == "int"){
                Expr* value1 = getCode()->exprs[index1];
                if(value1->type == NUMBER_EXPR && value1->number == 2){
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    getCode()->generateCode(op2, reader->prev->line);
                    getCode()->generateCode(index2, reader->prev->line);
                    canShift = true;
                }
            }
            else if(op1Type == "int" && op2 == CONSTANT_OP){
                Expr* value2 = getCode()->exprs[index2];
                if(value2->type == NUMBER_EXPR && value2->number == 2){
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    canShift = true;
                }
            }

            if(canShift) {
                generateConstant(new Expr((double)1));
                getCode()->generateCode(LSHIFT_OP, reader->prev->line);
            }
            else getCode()->generateCode(MULT_OP, reader->prev->line); 
            break;
        }
        case SLASH: {
            if(op1Type != "Object" && op2Type != "Object"){
                if(op1Type != "int" && op1Type != "float"){
                    handleError(reader->prev, "Type mismatch: incompaitable types");
                    return "None";
                }
            }

            if(op1 == CONSTANT_OP && op2 == CONSTANT_OP){
                Expr* value2 = getCode()->exprs[index2];
                Expr* value1 = getCode()->exprs[index1];
                if(value1->type == NUMBER_EXPR && value2->type == NUMBER_EXPR){
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    generateConstant(new Expr(value1->number / value2->number));
                    return "float";
                }
            }

            getCode()->generateCode(DIV_OP, reader->prev->line); 
            return "float";
        }
        case SLASH_SLASH: {
            if(op1Type != "Object" && op2Type != "Object"){
                if(op1Type != "int" && op1Type != "float"){
                    handleError(reader->prev, "Type mismatch: incompaitable types");
                    return "None";
                }
            }

            bool canShift = false;
            if(op1 == CONSTANT_OP && op2 == CONSTANT_OP){
                Expr* value2 = getCode()->exprs[index2];
                Expr* value1 = getCode()->exprs[index1];
                if(value1->type == NUMBER_EXPR && value2->type == NUMBER_EXPR){
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    double result = floor(value1->number / value2->number);
                    generateConstant(new Expr(result));
                    return "int";
                }
            }
            else if(op2 == CONSTANT_OP){
                Expr* value2 = getCode()->exprs[index2];
                if(value2->type == NUMBER_EXPR && value2->number == 2){
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    canShift = true;
                }
            }

            if(canShift){
                generateConstant(new Expr((double)1));
                getCode()->generateCode(RSHIFT_OP, reader->prev->line);
            }
            else getCode()->generateCode(FLOOR_DIV_OP, reader->prev->line); 
            
            return "int";
        }
        case PERCENT: {
            if(op1Type != "Object" && op2Type != "Object"){
                if(op1Type != "int" && op2Type != "int"){
                    handleError(reader->prev, "Type mismatch: incompaitable types");
                    return "None";
                }
            }

            if(op1 == CONSTANT_OP && op2 == CONSTANT_OP){
                Expr* value2 = getCode()->exprs[index2];
                Expr* value1 = getCode()->exprs[index1];
                if(value1->type == NUMBER_EXPR && value2->type == NUMBER_EXPR){
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    double result = ((int)value1->number) % ((int)value2->number);
                    generateConstant(new Expr(result));
                    break;
                }
            }

            getCode()->generateCode(MOD_OP, reader->prev->line); 
            break;
        }
        case STAR_STAR: {
            if(op1Type != "Object" && op2Type != "Object"){
                if(op1Type != "int" && op1Type != "float"){
                    handleError(reader->prev, "Type mismatch: incompaitable types");
                    return "None";
                }
            }

            if(op1 == CONSTANT_OP && op2 == CONSTANT_OP){
                Expr* value2 = getCode()->exprs[index2];
                Expr* value1 = getCode()->exprs[index1];
                if(value1->type == NUMBER_EXPR && value2->type == NUMBER_EXPR){
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    generateConstant(new Expr(pow(value1->number, value2->number)));
                    break;
                }
            }

            getCode()->generateCode(POW_OP, reader->prev->line); 
            break;
        }
        case LEFT_SHIFT: {
            if(op1Type != "Object" && op2Type != "Object"){
                if(op1Type != "int" && op2Type != "int"){
                    handleError(reader->prev, "Operand Types must be integers");
                    return "None";
                }
            }

            if(op1 == CONSTANT_OP && op2 == CONSTANT_OP){
                Expr* value2 = getCode()->exprs[index2];
                Expr* value1 = getCode()->exprs[index1];
                if(value1->type == NUMBER_EXPR && value2->type == NUMBER_EXPR){
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    if(floor(value1->number) != value1->number || floor(value2->number) != value2->number){
                        handleError(reader->prev, "Operand Types must be integers");
                        return "None";
                    }

                    double result = ((int)value1->number) << ((int)value2->number);
                    generateConstant(new Expr(result));
                    break;
                }
            }

            getCode()->generateCode(LSHIFT_OP, reader->prev->line); 
            break;
        }
        case RIGHT_SHIFT: {
            if(op1Type != "Object" && op2Type != "Object"){
                if(op1Type != "int" && op2Type != "int"){
                    handleError(reader->prev, "Operand Types must be integers");
                    return "None";
                }
            }

            if(op1 == CONSTANT_OP && op2 == CONSTANT_OP){
                Expr* value2 = getCode()->exprs[index2];
                Expr* value1 = getCode()->exprs[index1];
                if(value1->type == NUMBER_EXPR && value2->type == NUMBER_EXPR){
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    if(floor(value1->number) != value1->number || floor(value2->number) != value2->number){
                        handleError(reader->prev, "Operand Types must be integers");
                        return "None";
                    }
                    double result = ((int)value1->number) >> ((int)value2->number);
                    generateConstant(new Expr(result));
                    break;
                }
            }

            getCode()->generateCode(RSHIFT_OP, reader->prev->line); 
            break;
        }
        case BIT_OR: {
            if(op1Type != "Object" && op2Type != "Object"){
                if(op1Type != "int" && op2Type != "int"){
                    handleError(reader->prev, "Operand Types must be integers");
                    return "None";
                }
            }
            if(op1 == CONSTANT_OP && op2 == CONSTANT_OP){
                Expr* value2 = getCode()->exprs[index2];
                Expr* value1 = getCode()->exprs[index1];
                if(value1->type == NUMBER_EXPR && value2->type == NUMBER_EXPR){
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    if(floor(value1->number) != value1->number || floor(value2->number) != value2->number){
                        handleError(reader->prev, "Operand Types must be integers");
                        return "None";
                    }
                    double result = ((int)value1->number) | ((int)value2->number);
                    generateConstant(new Expr(result));
                    break;
                }
            }

            getCode()->generateCode(BIT_OR_OP, reader->prev->line); 
            break;
        }
        case BIT_XOR: {
            if(op1Type != "Object" && op2Type != "Object"){
                if(op1Type != "int" && op2Type != "int"){
                    handleError(reader->prev, "Operand Types must be integers");
                    return "None";
                }
            }
            if(op1 == CONSTANT_OP && op2 == CONSTANT_OP){
                Expr* value2 = getCode()->exprs[index2];
                Expr* value1 = getCode()->exprs[index1];
                if(value1->type == NUMBER_EXPR && value2->type == NUMBER_EXPR){
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    if(floor(value1->number) != value1->number || floor(value2->number) != value2->number){
                        handleError(reader->prev, "Operand Types must be integers");
                        return "None";
                    }
                    double result = ((int)value1->number) ^ ((int)value2->number);
                    generateConstant(new Expr(result));
                    break;
                }
            }

            getCode()->generateCode(BIT_XOR_OP, reader->prev->line); 
            break;
        }
        case BIT_AND: {
            if(op1Type != "Object" && op2Type != "Object"){
                if(op1Type != "int" && op2Type != "int"){
                    handleError(reader->prev, "Operand Types must be integers");
                    return "None";
                }
            }
            if(op1 == CONSTANT_OP && op2 == CONSTANT_OP){
                Expr* value2 = getCode()->exprs[index2];
                Expr* value1 = getCode()->exprs[index1];
                if(value1->type == NUMBER_EXPR && value2->type == NUMBER_EXPR){
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    getCode()->code.pop_back();
                    if(floor(value1->number) != value1->number || floor(value2->number) != value2->number){
                        handleError(reader->prev, "Operand Types must be integers");
                        return "None";
                    }
                    double result = ((int)value1->number) & ((int)value2->number);
                    generateConstant(new Expr(result));
                    break;
                }
            }

            getCode()->generateCode(BIT_AND_OP, reader->prev->line); 
            break;
        }
    }

    if(op1Type == "Object" || op2Type == "Object") return "Object";
    if(op1Type == "float" || op2Type == "float") return "float";
    return op1Type;
}

string parseCall(bool assign){
    string funcIdentifier = lastCall;
    lastCallIndex = getCode()->code.size()-1;
    int funcIndex = functions.back()->loadName(lastCallLexem);
    int argCount;
    if(funcIndex != -1 && functions.back()->names[funcIndex]->argTypes.size() > 0) argCount = parseFuncArgs(funcIndex);
    else if(classTypes.find(lastCall) != classTypes.end()){
        ClassParser* classParser = classMap[lastCall];
        bool found = false;
        for(Name* name : classParser->names){
            if(name->id->value == "__init__") found = true;
        }
        if(found) argCount = parseInitArgs();
        else argCount = parseArgs();
    }
    else argCount = parseArgs();
    getCode()->generateCode(CALL_OP, reader->prev->line);
    getCode()->generateCode(argCount, reader->prev->line);
    if(funcIndex != -1) return functions.back()->names[funcIndex]->type;

    return globals[funcIdentifier];
}
Lexem* lastInstanceLexem;
string parseDot(bool assign){
    read(IDENTIFIER, "Expected property name after '.'");
    Lexem* attrLexem = reader->prev;
    int attrIndex = getCode()->storeExpr(new Expr(reader->prev->value));

    int instanceIndex = functions.back()->loadName(lastInstanceLexem);
    int methodIndex;
    ClassParser* classParser;
    string instanceType;
    if(instanceIndex == -1){
        methodIndex = -1;
    }
    else{
        instanceType = functions.back()->names[instanceIndex]->type;
        if(classMap.find(instanceType) == classMap.end()) {
            handleError(reader->prev, "variable is not an instance of a class");
            return "None";
        }
        classParser = classMap[instanceType];
        methodIndex = classParser->loadName(attrLexem);

        while(methodIndex == -1){
            if(superMap.find(instanceType) != superMap.end()){
                instanceType = superMap[instanceType];
                classParser = classMap[instanceType];
                methodIndex = classParser->loadName(attrLexem);
            }
            else break;
        } 
    }  

    if(assign && peek(EQUAL)){
        string type = parseExpr(ASSIGN_ORDER);
        if(methodIndex == -1){
            getCode()->generateCode(STORE_ATTRIBUTE_OP, reader->prev->line);
            getCode()->generateCode(attrIndex, reader->prev->line);
            return "Object";
        }

        string dataType = classParser->names[methodIndex]->type;

        if(dataType != "Object"){
            if(dataType != type){
                if(dataType== "float"){
                    if(type != "int"){
                        handleError(reader->prev, "Type mismatch");
                        return "None";
                    }
                }
                else if(superMap.find(type) != superMap.end()){
                    if(dataType != superMap[type]){
                        handleError(reader->prev, "Type mismatch");
                        return "None";
                    }
                }
                else{
                    handleError(reader->prev, "Type mismatch");
                    return "None";
                }
            }
        }
        getCode()->generateCode(STORE_ATTRIBUTE_OP, reader->prev->line);
        getCode()->generateCode(attrIndex, reader->prev->line);

        return dataType;
    }
    else if(peek(LEFT_PAREN)){
        if(methodIndex == -1){
            int argCount = parseArgs();
            getCode()->generateCode(METHOD_CALL_OP, reader->prev->line);
            getCode()->generateCode(attrIndex, reader->prev->line);
            getCode()->generateCode(argCount, reader->prev->line);
            return "Object";
        }

        Name* methodVar = classParser->names[methodIndex];
        int argCount = parseMethodArgs(methodVar);
        getCode()->generateCode(METHOD_CALL_OP, reader->prev->line);
        getCode()->generateCode(attrIndex, reader->prev->line);
        getCode()->generateCode(argCount, reader->prev->line);
        return methodVar->type;
    }
    else{
        getCode()->generateCode(LOAD_ATTRIBUTE_OP, reader->prev->line);
        getCode()->generateCode(attrIndex, reader->prev->line);
        
        if(methodIndex == -1) return "Object";

        return classParser->names[methodIndex]->type;
    }
}
string parseLiteral(bool assign){
    switch(reader->prev->type){
        case FALSE: getCode()->generateCode(FALSE_OP, reader->prev->line); return "bool";
        case NONE: getCode()->generateCode(NONE_OP, reader->prev->line); return "None";
        case TRUE: getCode()->generateCode(TRUE_OP, reader->prev->line); return "bool";
        default: return "None";
    }
}
string parseIfExp(bool assign){
    string first = priorType;
    parseExpr(ASSIGN_ORDER);
    read(ELSE, "Expected 'else' in if expression");
    string second = parseExpr(ASSIGN_ORDER);

    if(first != "Object" && second != "Object") {
        if(first != second){
            if(first == "float"){
                if(second != "int"){
                    handleError(reader->prev, "Type mismatch");
                    return "None";
                }
            }
            else if(second == "float"){
                if(first != "int"){
                    handleError(reader->prev, "Type mismatch");
                    return "None";
                }
            }
            else{
                handleError(reader->prev, "Type mismatch");
                return "None";
            }
        }
    }

    getCode()->generateCode(IF_EXP_OP, reader->prev->line);
    
    if(first == "Object" || second == "Object") return "Object";
    if(first == "float" || second == "float") return "float";
    return first;
}
string parseParen(bool assign){
    int size = 0;
    string type;
    bool isTuple = false;
    if(reader->next->type != RIGHT_PAREN){
        size++;
        type = parseExpr(ASSIGN_ORDER);
        while(peek(COMMA)){
            isTuple = true;
            if(reader->next->type == RIGHT_PAREN) break;
            type = parseExpr(ASSIGN_ORDER);
            size++;
        } 
    }
    read(RIGHT_PAREN, "Expected ')' after expression.");

    if(isTuple){
        getCode()->generateCode(TUPLE_OP, reader->prev->line);
        getCode()->generateCode(size, reader->prev->line); 
        return "tuple";
    }

    return type;
}
string parseList(bool assign){
    int size = 0;
    if(reader->next->type != RIGHT_BRACKET){
        do{
            parseExpr(ASSIGN_ORDER);
            size++;
        } while(peek(COMMA));
    }
    read(RIGHT_BRACKET, "Expected ']' after list elements");

    getCode()->generateCode(LIST_OP, reader->prev->line);
    getCode()->generateCode(size, reader->prev->line);   
    return "list"; 
}
string parseSet(bool assign){
    int size = 0;
    bool isDict = false;
    if(reader->next->type != RIGHT_BRACE){
        parseExpr(ASSIGN_ORDER);
        if(peek(COLON)){
            isDict = true;
            parseExpr(ASSIGN_ORDER);
        }
        size++;
        while(peek(COMMA)){
            parseExpr(ASSIGN_ORDER);
            if(isDict){
                read(COLON, "Expected ':' after key");
                parseExpr(ASSIGN_ORDER);
            }
            size++;
        }
    }
    read(RIGHT_BRACE, "Expected '}' after set elements");

    if(isDict){
        getCode()->generateCode(DICT_OP, reader->prev->line);
        getCode()->generateCode(size*2, reader->prev->line); 
        return "dict";
    }
    else{
        getCode()->generateCode(SET_OP, reader->prev->line);
        getCode()->generateCode(size, reader->prev->line); 
        return "set";
    } 
}
string parseSubscript(bool assign){
    string type1 = parseExpr(ASSIGN_ORDER);
    bool isSlice = false;
    if(peek(COLON)){
        isSlice = true;
        if(type1 != "Object" && type1 != "int"){
            handleError(reader->prev, "Indexes must be integers");
            return "None";
        }
        string type2 = parseExpr(ASSIGN_ORDER);
        if(type2 != "Object" && type2 != "int"){
            handleError(reader->prev, "Indexes must be integers");
            return "None";
        }
    }
    read(RIGHT_BRACKET, "expected ']' at end of subscript");

    if(isSlice){
        if(assign && peek(EQUAL)){
            string type3 = parseExpr(ASSIGN_ORDER);
            if(type3 != "Object" && type3 != "list"){
                handleError(reader->prev, "Expected List");
                return "None";
            }
            getCode()->generateCode(STORE_SLICE_OP, reader->prev->line);
        }
        else{
            getCode()->generateCode(LOAD_SLICE_OP, reader->prev->line);
        }
        return "list";
    }
    else{
        if(assign && peek(EQUAL)){
            parseExpr(ASSIGN_ORDER);
            getCode()->generateCode(STORE_SUBSCRIPT_OP, reader->prev->line);
        }
        else{
            getCode()->generateCode(LOAD_SUBSCRIPT_OP, reader->prev->line);
        }
        return "Object";
    }
}
string parseNumber(bool assign){
    double number = stod(reader->prev->value);
    generateConstant(new Expr(number));
    if(floor(number) == number){
        return "int";
    }
    return "float";
}
string parseOr(bool assign){
    int elseJump = generateJump(JUMP_IF_FALSE_OP);
    int endJump = generateJump(JUMP_OP);

    generateJumpOffset(elseJump);
    getCode()->generateCode(POP_OP, reader->prev->line);

    parseExpr(OR_ORDER);
    generateJumpOffset(endJump);
    return "bool";
}
string parseString(bool assign){
    generateConstant(new Expr(reader->prev->value));
    return "str";
}
string parseFString(bool assign){
    generateConstant(new Expr(reader->prev->value));
    while(peek(LEFT_BRACE) || peek(FORMATTED_STRING)){
        if(reader->prev->type == LEFT_BRACE){
            parseExpr(ASSIGN_ORDER);
            read(RIGHT_BRACE, "Expected '}' after string formatting");
            getCode()->generateCode(ADD_OP, reader->prev->line);
        }
        else if(reader->prev->type == FORMATTED_STRING){
            generateConstant(new Expr(reader->prev->value));
            getCode()->generateCode(ADD_OP, reader->prev->line);
        }
    }
    return "str";
}
string parseIDLexem(Lexem* nameLexem, bool assign){
    int loadOp, storeOp;
    int argIndex = functions.back()->loadName(nameLexem);
    string type;
    if(argIndex != -1){
        loadOp = LOAD_NAME_OP;
        storeOp = STORE_NAME_OP;
        type = functions.back()->names[argIndex]->type;
    }
    else if((argIndex = loadNonlocal(nameLexem, functions.size()-1)) != -1){
        loadOp = LOAD_NONLOCAL_OP;
        storeOp = STORE_NONLOCAL_OP;
        type = functions.back()->nonlocals[argIndex]->type;
    }
    else{
        argIndex = getCode()->storeExpr(new Expr(nameLexem->value));
        loadOp = LOAD_GLOBAL_OP;
        storeOp = STORE_GLOBAL_OP;
        if(globals.find(nameLexem->value) == globals.end()) {
            handleError(reader->prev, "varibale is undefined");
            return "None";
        }
        type = globals[nameLexem->value];
    }

    if(assign && peek(EQUAL)){
        string ExprType = parseExpr(ASSIGN_ORDER);
        
        if(type != "Object"){
            if(type != ExprType){
                if(type == "float"){
                    if(ExprType != "int"){
                        handleError(reader->prev, "Type mismatch");
                        return "None";
                    }
                }
                else if(superMap.find(ExprType) != superMap.end()){
                    if(type != superMap[ExprType]){
                        handleError(reader->prev, "Type mismatch");
                        return "None";
                    }
                }
                else{
                    handleError(reader->prev, "Type mismatch");
                    return "None";
                }
            }
        }
        
        getCode()->generateCode(storeOp, reader->prev->line);
        getCode()->generateCode(argIndex, reader->prev->line);
    }
    else{
        getCode()->generateCode(loadOp, reader->prev->line);
        getCode()->generateCode(argIndex, reader->prev->line);
    }

    return type;
}

string parseIdentifier(bool assign){
    return parseIDLexem(reader->prev, assign);
}
string parseSuper(bool assign){
    if(classes.size() == 0) handleError(reader->prev, "'super' can only be used in a class declaration");
    else if(!classes.back()->hasBase) handleError(reader->prev, "Class does not have a super class");

    read(LEFT_PAREN, "Expected '(' after call to super");
    read(RIGHT_PAREN, "Expected ')' after call to super");
    read(DOT, "Expected '.' after super");
    read(IDENTIFIER, "Expected superclass method name.");
    int classIndex = getCode()->storeExpr(new Expr(reader->prev->value));

    parseIDLexem(new Lexem("self"), false);

    if(peek(LEFT_PAREN)){
        int argCount = parseArgs();
        parseIDLexem(new Lexem("super"), false);
        getCode()->generateCode(SUPER_CALL_OP, reader->prev->line);
        getCode()->generateCode(classIndex, reader->prev->line);
        getCode()->generateCode(argCount, reader->prev->line);
    }
    else{
        parseIDLexem(new Lexem("super"), false);
        getCode()->generateCode(LOAD_SUPER_OP, reader->prev->line);
        getCode()->generateCode(classIndex, reader->prev->line);
    }

    return "Object";
}
string parseSelf(bool assign){
    if(classes.size() == 0) {
        handleError(reader->prev, "self has no refrence");
        return "None";
    }

    return parseIdentifier(false);
}
string parseUnary(bool assign){
    LexemType type = reader->prev->type;

    // Complie the operand
    string dataType = parseExpr(UNARY_ORDER);

    // generate bytecode
    switch(type){
        case NOT: {
            if(dataType != "bool" && dataType != "Object"){
                handleError(reader->prev, "Type must be bool");
                return "None";
            }
            getCode()->generateCode(NOT_OP, reader->prev->line);
            return "bool";
        }
        case MINUS: {
            if(dataType != "int" && dataType != "float" && dataType != "Object"){
                handleError(reader->prev, "Type must be a number");
                return "None";
            }
            getCode()->generateCode(INVERT_OP, reader->prev->line);
            return dataType;
        }
        case BIT_NOT: {
            if(dataType != "int" && dataType != "Object"){
                handleError(reader->prev, "Type must be a int");
                return "None";
            }
            getCode()->generateCode(BIT_NOT_OP, reader->prev->line);
            return "int";
        }
        default: return "None";
    }
}
string parseExpr(Order order){
    nextLexem();
    function<string(bool)> pre = parsers[reader->prev->type].pre;
    if(pre == NULL){
        handleError(reader->prev, "Expected expression.");
        return "None";
    }
    
    bool assign = order <= ASSIGN_ORDER;
    string type = pre(assign);
    priorType = type;

    while(order <= parsers[reader->next->type].order){
        if(reader->next->type == LEFT_PAREN) {
            lastCall = reader->prev->value;
            lastCallLexem = reader->prev;
        }
        else if(reader->next->type == DOT){
            lastInstanceLexem = reader->prev;
        }

        nextLexem();
        function<string(bool)> in = parsers[reader->prev->type].in;
        type = in(assign);
        priorType = type;
    }

    if(assign && peek(EQUAL)) handleError(reader->prev, "Invalied assignment");
    return type;
}
void parseBlock(){
    while(reader->next->type != DEDENT && reader->next->type != TOKEN_EOF) {
        parseStmt();
    }
    read(DEDENT, "Expected 'DEDENT' after block.");
}
void parseFunction(FuncType type, string returnType){
    int funcIndex;
    if(type == FUNC_FUNC){
        funcIndex = functions.back()->loadName(reader->prev);
    }
    else{
        funcIndex = classes.back()->loadName(reader->prev);
    }
    FuncParser* funcParser = new FuncParser(type, returnType);
    functions.back()->scopeCounter++;

    int loopStart = getCode()->code.size();

    read(LEFT_PAREN, "Expected '(' after function name");
    vector<string> argTypes;
    if(reader->next->type != RIGHT_PAREN){
        do {
            functions.back()->func->argCount++;
            readType("Expected data type before argument");
            Lexem* argLexem = reader->next;
            int argIndex = assignName("Expected identifier for argument", reader->prev->value);
            functions.back()->initName();
            argIndex = functions.back()->loadName(argLexem);
            string argtype = functions.back()->names[argIndex]->type;
            argTypes.push_back(argtype);
        } while(peek(COMMA));
    }
    read(RIGHT_PAREN, "Expected ')' after parameters");

    read(COLON, "Expected 'COLON' after class identifier");
    while(reader->next->type == NEWLINE) nextLexem();
    read(INDENT, "Expected 'INDENT' before function body");
    parseBlock();

    if(getCode()->code.size() >= 2 && getCode()->code[getCode()->code.size()-2] == CALL_OP && lastCall == functions.back()->func->name){
        getCode()->code.pop_back(); // arg count
        getCode()->code.pop_back(); // CALL_OP
        vector<int> opsBackStack;

        // remove all the ops up throug the function call
        for(int i=getCode()->code.size()-1; i>lastCallIndex; i--){
            opsBackStack.push_back(getCode()->code.back());
            getCode()->code.pop_back();
        }
        getCode()->code.pop_back(); // function index
        getCode()->code.pop_back(); // LOAD_GLOBAL_OP

        // push all the argument ops back onto the stack
        for(int i=opsBackStack.size()-1; i>=0; i--){
            getCode()->code.push_back(opsBackStack[i]);
        }

        for(int i=functions.back()->func->argCount; i>0; i--){
            getCode()->generateCode(STORE_NAME_OP, reader->prev->line);
            getCode()->generateCode(i, reader->prev->line);
            getCode()->generateCode(POP_OP, reader->prev->line);
        }

        generateLoop(loopStart);
    }

    Function* func = finish();
    if(type == FUNC_FUNC){
        functions.back()->names[funcIndex]->argTypes = argTypes;
    }
    else{
        classes.back()->names[funcIndex]->argTypes = argTypes;
    }
    
    getCode()->generateCode(FUNCTION_DEF_OP, reader->prev->line);
    int valIndex = getCode()->storeExpr(new Expr(func));  
    getCode()->generateCode(valIndex, reader->prev->line);

    for(int i=0; i<func->nonlocalCount; i++){
        int notGlobal = funcParser->nonlocals[i]->notGlobal;
        int index = funcParser->nonlocals[i]->index;
        getCode()->generateCode(notGlobal, reader->prev->line);
        getCode()->generateCode(index, reader->prev->line);
    }
}
void parseField(){
    string dataType = reader->prev->value;
    read(IDENTIFIER, "Expected method identifier");
    classes.back()->assignName(dataType);
    classes.back()->initName();
    int fieldIndex = getCode()->storeExpr(new Expr(reader->prev->value));

    if(peek(EQUAL)){
        string type = parseExpr(ASSIGN_ORDER);
        if(dataType != "Object"){
            if(dataType != type){
                if(dataType== "float"){
                    if(type != "int"){
                        handleError(reader->prev, "Type mismatch");
                        return;
                    }
                }
                else if(superMap.find(type) != superMap.end()){
                    if(dataType != superMap[type]){
                        handleError(reader->prev, "Type mismatch");
                        return;
                    }
                }
                else{
                    handleError(reader->prev, "Type mismatch");
                    return;
                }
            }
        }
    }
    else{
        getCode()->generateCode(NONE_OP, reader->prev->line);
    }
    read(NEWLINE, "Expected 'NEWLINE'");

    getCode()->generateCode(FIELD_DEF_OP, reader->prev->line);
    getCode()->generateCode(fieldIndex, reader->prev->line);
}
void parseMethod(){
    string returnType;
    if(checkType()){
        returnType = reader->prev->value;
    }
    else{
        returnType = "None";
    }

    read(IDENTIFIER, "Expected method identifier");
    classes.back()->assignName(returnType);
    classes.back()->initName();
    int methodIndex = getCode()->storeExpr(new Expr(reader->prev->value));

    FuncType type = METHOD_FUNC;
    if(reader->prev->value == "__init__"){
        if(returnType != "None") handleError(reader->prev, "'__init__' should not have a return type");
        type = INIT_FUNC;
    }

    parseFunction(type, returnType);

    getCode()->generateCode(METHOD_DEF_OP, reader->prev->line);
    getCode()->generateCode(methodIndex, reader->prev->line);
}
void errorHandeling(){
    reader->handelingError = false;

    while(reader->next->type != TOKEN_EOF){
        if(reader->prev->type == NEWLINE) return;

        switch(reader->next->type){
            case CLASS:
            case DEF:
            //case LET:
            case DATA_TYPE:
            case FOR:
            case IF:
            case WHILE:
            case RETURN:
                return;

            default: break;
        }

        nextLexem();
    }
}
void parseStmt(){
    if(peek(IMPORT)){
        read(STRING, "Expected math to source file to import");
        string fileName = reader->prev->value;
        if(reader->next->type != NEWLINE) handleError(reader->next, "Expected 'NEWLINE' after import statement");

        ifstream file(fileName);
        if(!file.is_open()){
            handleError(reader->prev, "Could not import module");
        }

        ostringstream stream;
        string source;

        stream << file.rdbuf();
        source = stream.str();

        lexers.push_back(new Lexer(source));
        sourceCodeNames.push_back(fileName);
        nextLexem();
        while(!peek(TOKEN_EOF)){
            parseStmt();
        }
        lexers.pop_back();
        sourceCodeNames.pop_back();
        nextLexem();
    }
    else if(peek(CLASS)){
        read(IDENTIFIER, "Expected class identifier");
        Lexem* className = reader->prev;
        classTypes.insert(className->value);

        int classIndex = getCode()->storeExpr(new Expr(reader->prev->value));
        functions.back()->assignName(className->value);

        getCode()->generateCode(CLASS_DEF_OP, reader->prev->line);
        getCode()->generateCode(classIndex, reader->prev->line);
        functions.back()->initName();
        //getCode()->generateCode(ASSIGN_GLOBAL_OP, reader->prev->line);
        //getCode()->generateCode(classIndex, reader->prev->line);

        ClassParser* classParser = new ClassParser(className);
        classes.push_back(classParser);
        classMap[className->value] = classes.back();

        if(peek(LEFT_PAREN)){
            read(IDENTIFIER, "Expected superclass name");
            Lexem* baseClass = reader->prev;
            parseIdentifier(false);

            if(className->value == reader->prev->value) handleError(reader->prev, "Super class can not be same as class being defined");

            functions.back()->scopeCounter++;
            if(classTypes.find(baseClass->value) == classTypes.end()) handleError(reader->prev, "Super class is undefined");
            functions.back()->storeName(new Lexem("super"), baseClass->value);
            functions.back()->initName();
            //getCode()->generateCode(ASSIGN_GLOBAL_OP, reader->prev->line);
            //getCode()->generateCode(0, reader->prev->line);

            parseIDLexem(className, false);
            getCode()->generateCode(BASE_OP, reader->prev->line);
            classParser->hasBase = true;
            read(RIGHT_PAREN, "Expected ')' at end of superclass");

            superMap[className->value] = baseClass->value;
        }

        parseIDLexem(className, false);

        read(COLON, "Expected ':' after class declaration");
        read(NEWLINE, "Expected 'NEWLINE'");
        read(INDENT, "Expected 'INDENT' before class body");
        while(reader->next->type != DEDENT && reader->next->type != TOKEN_EOF) {
            if(checkType()) parseField();
            else if(peek(DEF)) parseMethod();
            else if(peek(PASS)) nextLexem();
            else if(peek(NEWLINE)) nextLexem();
            else {
                handleError(reader->prev, "Expected either a field or method definition");
                break;
            }
            //while(reader->next->type == NEWLINE) nextLexem();
        }
        read(DEDENT, "Expected 'DEDENT' after class body");
        
        getCode()->generateCode(POP_OP, reader->prev->line);

        if(classParser->hasBase) endBlock();

        classes.pop_back();
    }
    else if(peek(DEF)){
        int functionIndex;
        Lexem* funcLexem;
        if(checkType()){
            funcLexem = reader->next;
            functionIndex = assignName("Expected function identifer", reader->prev->value);
        }
        else{
            funcLexem = reader->next;
            functionIndex = assignName("Expected function identifer", "None");
        }
        functions.back()->initName();
        functionIndex = functions.back()->loadName(funcLexem);
        parseFunction(FUNC_FUNC, functions.back()->names[functionIndex]->type);
    }
    else if(checkType()){
        string dataType = reader->prev->value;
        int name = assignName("Expected varibale Identifier", reader->prev->value);
        Lexem* nameLexem = reader->prev;
        if(peek(EQUAL)){
            string type = parseExpr(ASSIGN_ORDER);
            if(dataType != "Object"){
                if(dataType != type){
                    if(dataType== "float"){
                        if(type != "int"){
                            handleError(reader->prev, "Type mismatch");
                            return;
                        }
                    }
                    else if(superMap.find(type) != superMap.end()){
                        if(dataType != superMap[type]){
                            handleError(reader->prev, "Type mismatch");
                            return;
                        }
                    }
                    else{
                        handleError(reader->prev, "Type mismatch");
                        return;
                    }
                }
            }
        }
        else{
            getCode()->generateCode(NONE_OP, reader->prev->line);
        }

        functions.back()->initName();

        read(NEWLINE, "Expected 'NEWLINE'");
    }
    else if(peek(GLOBAL)){
        readType("Expected data type for variable");
        string globalType = reader->prev->value;
        
        read(IDENTIFIER, "Expected variable identifier");
        globals[reader->prev->value] = globalType;
    
        int name = getCode()->storeExpr(new Expr(reader->prev->value));

        Lexem* nameLexem = reader->prev;
        if(peek(EQUAL)){
            string type = parseExpr(ASSIGN_ORDER);
            if(globalType != "Object"){
                if(globalType != type){
                    if(globalType == "float"){
                        if(type != "int"){
                            handleError(reader->prev, "Type mismatch");
                        }
                    }
                    else{
                        handleError(reader->prev, "Type mismatch");
                    }
                }
            }
        }
        else{
            getCode()->generateCode(NONE_OP, reader->prev->line);
        }
        read(NEWLINE, "Expected 'NEWLINE'");

        getCode()->generateCode(ASSIGN_GLOBAL_OP, reader->prev->line);
        getCode()->generateCode(name, reader->prev->line);
    }
    else if(lexers.size() > 1 && functions.back()->type == SCRIPT_FUNC){
        nextLexem();
    }
    else if(peek(FOR)){
        functions.back()->scopeCounter++; // start for loop scope;

        // reads loop target
        read(IDENTIFIER, "Expected an identifier");
        Lexem* nameLexem = reader->prev;

        read(IN, "expected 'in' before iterator");

        // reads iterable
        string type = parseExpr(ASSIGN_ORDER);
        if(type != "list" && type != "tuple" && type != "set" && type != "dict"){
            handleError(reader->prev, "Expected iterable over");
        }
        Lexem* listLexem = new Lexem("listLexem");
        functions.back()->storeName(listLexem, type);
        functions.back()->initName();
        int listIndex = functions.back()->names.size()-1;
        getCode()->generateCode(STORE_NAME_OP, reader->prev->line);
        getCode()->generateCode(listIndex, reader->prev->line);

        // the intial name declartator
        Lexem* iterLexem = new Lexem("iterLexem");
        functions.back()->storeName(iterLexem, "Object");
        functions.back()->initName();
        int iterIndex = functions.back()->names.size()-1;
        getCode()->generateCode(LOAD_ITER_START_OP, reader->prev->line);
        getCode()->generateCode(listIndex, reader->prev->line);
        getCode()->generateCode(STORE_NAME_OP, reader->prev->line);
        getCode()->generateCode(iterIndex, reader->prev->line);


        // loop starts here
        int loopStart = getCode()->code.size();
        int exit = -1;

        // loop condition
        getCode()->generateCode(LOAD_NAME_OP, reader->prev->line);
        getCode()->generateCode(iterIndex, reader->prev->line);
        getCode()->generateCode(LOAD_ITER_STOP_OP, reader->prev->line);
        getCode()->generateCode(listIndex, reader->prev->line);
        getCode()->generateCode(EQ_OP, reader->prev->line);
        getCode()->generateCode(NOT_OP, reader->prev->line);
        exit = generateJump(JUMP_IF_FALSE_OP);
        getCode()->generateCode(POP_OP, reader->prev->line);

        int body = generateJump(JUMP_OP);
        int increment = getCode()->code.size();
        getCode()->generateCode(INCREMENT_ITER_OP, reader->prev->line);
        getCode()->generateCode(iterIndex, reader->prev->line);
        //getCode()->generateCode(POP_OP, reader->prev->line);
        generateLoop(loopStart);
        loopStart = increment;
        generateJumpOffset(body);

        read(COLON, "Expected ':' after for loop conditions");
        read(NEWLINE, "Expected 'NEWLINE'");
        read(INDENT, "Expected 'INDENT'");
        functions.back()->scopeCounter++;
        functions.back()->storeName(nameLexem, "Object");
        functions.back()->initName();
        int name = functions.back()->names.size()-1;
        getCode()->generateCode(LOAD_ITER_OP, reader->prev->line);
        getCode()->generateCode(iterIndex, reader->prev->line);
        getCode()->generateCode(STORE_NAME_OP, reader->prev->line);
        getCode()->generateCode(name, reader->prev->line);
        parseBlock();
        endBlock();

        generateLoop(loopStart);

        if(exit != -1){
            generateJumpOffset(exit);
            getCode()->generateCode(POP_OP, reader->prev->line);
        }

        //endBlock();
    }
    else if(peek(IF)){
        //read(LEFT_PAREN, "Expected '(' after 'if'.");
        parseExpr(ASSIGN_ORDER);
        //read(RIGHT_PAREN, "Expected ')' after condition.");

        // if jump
        int ifStart = generateJump(JUMP_IF_FALSE_OP);
        getCode()->generateCode(POP_OP, reader->prev->line); // POP if true

        read(COLON, "Expected ':' after if condition");
        read(NEWLINE, "Expected 'NEWLINE'");
        read(INDENT, "Expected 'INDENT'");
        functions.back()->scopeCounter++;
        parseBlock();
        endBlock();
        //parseStmt(); // if block

        // else jump
        int elseStart = generateJump(JUMP_OP);

        // offset if false
        generateJumpOffset(ifStart);
        getCode()->generateCode(POP_OP, reader->prev->line);

        if(reader->next->type == ELIF){
            reader->next->type = IF;
            parseStmt();
        }
        else if(peek(ELSE)) {
            read(COLON, "Expected a ':' after 'else'");
            read(NEWLINE, "Expected 'NEWLINE'");
            read(INDENT, "Expected 'INDENT'");
            functions.back()->scopeCounter++;
            parseBlock();
            endBlock();
            //parseStmt(); // else block
        }

        // offset for else block;
        generateJumpOffset(elseStart);
    }
    else if(peek(RETURN)){
        if(functions.back()->type == SCRIPT_FUNC) handleError(reader->prev, "cant return from script");
        if(peek(NEWLINE)){
            if(functions.back()->func->returnType != "None") handleError(reader->prev, "expected return type of " + functions.back()->func->returnType);
            generateReturn();
        }
        else{
            if(functions.back()->type == INIT_FUNC) handleError(reader->prev, "cant return a value from an __init__");
            string returnType = parseExpr(ASSIGN_ORDER);
            if(returnType != functions.back()->func->returnType) handleError(reader->prev, "wrong return type: Expected return type of " + functions.back()->func->returnType);
            read(NEWLINE, "Expect 'NEWLINE' after return value");
            getCode()->generateCode(RETURN_OP, reader->prev->line);
        }
    }
    else if(peek(WHILE)){
        int loopStart = getCode()->code.size();

        //read(LEFT_PAREN, "Expected '(' after 'while'");
        parseExpr(ASSIGN_ORDER);
        //read(RIGHT_PAREN, "Expected ')' after condition");

        int jump = generateJump(JUMP_IF_FALSE_OP);
        getCode()->generateCode(POP_OP, reader->prev->line);

        read(COLON, "Expected ':' after while condition");
        read(NEWLINE, "Expected 'NEWLINE'");
        read(INDENT, "Expected 'INDENT'");
        functions.back()->scopeCounter++;
        parseBlock();
        endBlock();
        //parseStmt();

        generateLoop(loopStart);

        generateJumpOffset(jump);
        getCode()->generateCode(POP_OP, reader->prev->line);
    }
    else if(peek(INDENT)){
        functions.back()->scopeCounter++;
        parseBlock();
        endBlock();
    }
    else if(peek(NEWLINE)){
        return;
    }
    else if(peek(DEL)){
        read(IDENTIFIER, "Expected identifier to be deleted");
        parseIdentifier(false);
        read(LEFT_BRACKET, "deletion can only be done on items of an iterator");
        parseExpr(ASSIGN_ORDER);
        bool isSlice = false;
        if(peek(COLON)){
            isSlice = true;
            parseExpr(ASSIGN_ORDER);
        }
        read(RIGHT_BRACKET, "expected ']' at end of subscript");

        if(isSlice){
            getCode()->generateCode(DEL_SLICE_OP, reader->prev->line);
        }
        else{
            getCode()->generateCode(DEL_SUBSCRIPT_OP, reader->prev->line);
        }

        read(NEWLINE, "Expected 'NEWLINE'");
    }
    else if(peek(PASS)){
        //read(NEWLINE, "Expected newline");
        nextLexem();
    }
    else{
        parseExpr(ASSIGN_ORDER);
        read(NEWLINE, "Expected 'NEWLINE'");
        getCode()->generateCode(POP_OP, reader->prev->line);
    }

    if(reader->handelingError) errorHandeling();
}

void defineBuiltIns(){
    globals["len"] = "int";
    globals["input"] = "str";
    globals["open"] = "file";
    globals["read"] = "str";
    globals["write"] = "None";
    globals["close"] = "None";
    globals["Thread"] = "thread";
    globals["start"] = "None";
    globals["join"] = "None";
    globals["Lock"] = "lock";
    globals["getLock"] = "None";
    globals["releaseLock"] = "None";
    globals["Socket"] = "socket";
    globals["bind"] = "None";
    globals["listen"] = "None";
    globals["accept"] = "socket";
    globals["recv"] = "str";
    globals["send"] = "None";
    globals["connect"] = "None";
    globals["toStr"] = "str";
    globals["toInt"] = "int";
    globals["toFloat"] = "float";
    globals["print"] = "None";
    globals["range"] = "list";
    globals["Dict"] = "dict";
}

Function* runCompiler(string fileName, string sourceCode){ 
    lexers.push_back(new Lexer(sourceCode));
    sourceCodeNames.push_back(fileName);
    
    FuncParser* funcParser = new FuncParser(SCRIPT_FUNC, "None");

    defineBuiltIns();

    reader->error = false;
    reader->handelingError = false;

    nextLexem();
    while(!peek(TOKEN_EOF)){
        parseStmt();
    }

    Function* func = finish();
    if(reader->error) return NULL;
    return func;
}