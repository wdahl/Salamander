# William Dahl
# ICSI 680 - Masters Project

# Salamander

## Compilation
The salamander enviroment can be compiled using the provided make file.
Just run "make" in the command prompt and the executable program called "salamander" will be made.

## Usage
Salamander source code can be compiled and executued by running the following command:
"./salamander [path to the source code file]"

For example, to run the import.sal file within the ExampleCodes directory youe woled exucte the follwong command:
"./salamander ./ExampleCodes/import.sal"

The code will then be compiled to bytecode using the salamander compiler and imedeitally executed by the salamander virtual machine.

## Notes
The salamander compiler and virtual machine using some feature of C++11, therefore please ensure that the c++ compiler you are using can compile C++11 code. I was using the g++ compiler on ubuntu and everyhting worked fine. If you run into any isseu please let me know.

Example codes are provied in the "ExampleCode" directory. These files demenstrate the major features of salamander and how to use them.