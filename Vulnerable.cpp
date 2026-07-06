#include <iostream>
#include <cstring>

using namespace std;

//This function is never actually called by the code. It is a "hidden" area of memory. 
//Goal is to force the computer to jump here by breaking the logic of the stack.
void secretFunction() {
    cout << endl << "******************************************" << endl;
    cout << "[!!!] ATTACK SUCCESSFUL [!!!]" << endl;
    cout << "******************************************" << endl;
}

// This function is vulnerable to a stack overflow.
void vulnerableSubroutine(char *input) { //point to memory address of whats typed in terminal, aka bash
    char buffer[64]; //creates a sort of box in memory
    
    //strcpy is the vulnerability, it doesn't stop at 64
    //strcpy was designed in an era before modern security. It has no size parameter. 
    //it starts at the beginning of input and keeps copying bytes until it sees a null terminator (a zero). 
    //it blindly trusts that the buffer is big enough.
    strcpy(buffer, input); 
    
    cout << "Buffer contains: " << buffer << endl;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        cout << "Error: Provide an argument to the program." << endl;
        return 1;
    }

    vulnerableSubroutine(argv[1]);
    
    cout << "Program exited normally." << endl;
    return 0;
}