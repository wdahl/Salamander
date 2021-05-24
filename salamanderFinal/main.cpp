#include "virtualMachine.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
using namespace std;

int main(int argc, char** argv) {
    VirtualMachine* vm = new VirtualMachine();

    if(argc != 2){
        cerr << "Usage: salamander [file]" << endl;
        exit(64);
    }
 
    ifstream file(argv[1]);
    if(!file.is_open()){
        cerr << "error opening file" << endl;
        exit(74);
    }

    ostringstream stream;
    string source;

    stream << file.rdbuf();
    source = stream.str();
    
    file.close();
    
    ExitCode result = vm->readByteCode(argv[1], source);
    if(result == COMPILE_ERROR) exit(65);
    if(result == RUNTIME_ERROR) exit (70);

    vm->clear();

    return 0;
}