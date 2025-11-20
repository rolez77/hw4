/*
Assignment:
HW4 - Complete Parser and Code Generator for PL/0
(with Procedures, Call, and Else)
Author(s): Jovany Lopez and Andrew Irimie
Language: C (only)
To Compile:
Scanner:
gcc -O2 -std=c11 -o lex lex.c
Parser/Code Generator:
gcc -O2 -std=c11 -o parsercodegen_complete parsercodegen_complete.c
Virtual Machine:
gcc -O2 -std=c11 -o vm vm.c
To Execute (on Eustis):
./lex <input_file.txt>
./parsercodegen_complete
./vm elf.txt
where:
<input_file.txt> is the path to the PL/0 source program
Notes:
- lex.c accepts ONE command-line argument (input PL/0 source file)
- parsercodegen_complete.c accepts NO command-line arguments
- Input filename is hard-coded in parsercodegen_complete.c
- Implements recursive-descent parser for extended PL/0 grammar
- Supports procedures, call statements, and if-then-else
- Generates PM/0 assembly code (see Appendix A for ISA)
- VM must support EVEN instruction (OPR 0 11)
- All development and testing performed on Eustis
Class: COP3402 - System Software - Fall 2025
Instructor: Dr. Jie Lin
Due Date: Friday, November 21, 2025 at 11:59 PM ET
*/

/*
Assignment:
HW3 - Parser and Code Generator for PL/0
Author(s): Jovany Lopez and Andrew Irimie
Language: C (only)
To Compile:
Scanner:
gcc -O2 -std=c11 -o lex lex.c
Parser/Code Generator:
gcc -O2 -std=c11 -o parsercodegen parsercodegen.c
To Execute (on Eustis):
./lex <input_file.txt>
./parsercodegen
where:
<input_file.txt> is the path to the PL/0 source program
Notes:
- lex.c accepts ONE command-line argument (input PL/0 source file)
- parsercodegen.c accepts NO command-line arguments
- Input filename is hard-coded in parsercodegen.c
- Implements recursive-descent parser for PL/0 grammar
- Generates PM/0 assembly code (see Appendix A for ISA)
- All development and testing performed on Eustis
Class: COP3402 - System Software - Fall 2025
Instructor: Dr. Jie Lin
Due Date: Friday, October 31, 2025 at 11:59 PM ET
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SYMBOL_TABLE_SIZE 500

typedef enum {
    skipsym = 1 , // Skip / ignore token
    identsym =2, // Identifier
    numbersym = 3 , // Number
    plussym = 4, // +
    minussym = 5, // -
    multsym = 6, // *
    slashsym = 7, // /
    eqsym =8, // =
    neqsym =9, // <>
    lessym =10, // <
    leqsym =11, // <=
    gtrsym =12, // >
    geqsym =13, // >=
    lparentsym =14, // (
    rparentsym =15, // )
    commasym =16, // ,
    semicolonsym = 17, // ;
    periodsym =18, // .
    becomessym =19, // :=
    beginsym = 20, // begin
    endsym = 21, // end
    ifsym = 22, // if
    fisym = 23, // fi
    thensym = 24, // then
    whilesym = 25, // while
    dosym =26, // do
    callsym =27, // call
    constsym =28, // const
    varsym =29, // var
    procsym =30, // procedure
    writesym =31, // write
    readsym =32, // read
    elsesym =33, // else
    evensym =34 // even
} TokenType ;

typedef enum{
    LIT =1,
    OPR =2,
    LOD =3,
    STO =4,
    CAL =5,
    INC =6,
    JMP =7,
    JPC =8,
    SYS =9
}OpCode;

typedef enum{
   RTN = 0,
   ADD = 1,
   SUB = 2,
   MUL = 3,
   DIV = 4,
   EQUL = 5,
   NEQ = 6,
   LSS = 7,
   LEQ = 8,
   GTR = 9,
   GEQ = 10,
   EVEN = 11
}oprcodes;


typedef struct ir{
    int op;
    int l;
    int m;
} InstructionRegister;

typedef struct{
    int kind;
    char name[12];
    int val;
    int level;
    int addr;
    int mark;
}symbol;

//globals
int tokenList[MAX_SYMBOL_TABLE_SIZE];
char identifierList[MAX_SYMBOL_TABLE_SIZE][12];
int numList[MAX_SYMBOL_TABLE_SIZE];
int tokenInd =0;
int tokenCount = 0;
int codeIdx = 0;
int symbolInd = 1;
int numVars = 0;
int numConsts = 0;
int currentLevel =0;
InstructionRegister code[MAX_SYMBOL_TABLE_SIZE];

// list of error messages each corresponding to an index
const char* errorMessages[] = {
    "Scanning error detected by lexer (skipsym present)", //0
    "program must end with period", //1
    "const, var, and read keywords must be followed by identifier",//2
    "symbol name has already been declared",//3
    "constants must be assigned with =",//4
    "constants must be assigned an integer value",//5
    "constant and variable declarations must be followed by a semicolon",//6
    "undeclared identifier",//7
    "only variable values may be altered",//8
    "assignment statements must use :=",//9
    "begin must be followed by end",//10
    "if must be followed by then",//11
    "while must be followed by do",//12
    "condition must contain comparison operator",//13
    "right parenthesis must follow left parenthesis",//14
    "arithmetic equations must contain operands, parentheses, numbers, or symbols"//15
};


symbol symbol_table[MAX_SYMBOL_TABLE_SIZE];
//prototypes
int symbolTableCheck(const char*name);
void addToSymbolTable(int kind, const char* name, int val, int level, int addr);
void program();
void block();
void constDeclaration();
int varDeclaration();
void procedureDeclaration();
void statement();
void condition();
void expression();
void term();
void factor();
void getNextToken();
void emit(int op, int l, int m);
void readFile();
void printCode();
void printTable();
void printFile();

// linear search of the table, takes in a name as a parameter, if found it will return the index, otherwise returns -1 if not found
int symbolTableCheck(const char*name){
    for(int i =0; i<symbolInd;i++){
        if(strcmp(symbol_table[i].name,name) ==0 && symbol_table[i].mark ==0){
            return i;
        }
    }
    return -1;
}

/*
Function to add to the symbol table
Takes in kind,name,val,level,addr
// level will always be 0 for this project
*/
void addToSymbolTable(int kind, const char* name, int val, int level, int addr){
    int len = strlen(name);
    if(symbolInd >= MAX_SYMBOL_TABLE_SIZE){
        printf("Overflow");
        return;
    }
    symbol_table[symbolInd].kind = kind;
    strncpy(symbol_table[symbolInd].name, name, len);
    symbol_table[symbolInd].name[len] = '\0';
    symbol_table[symbolInd].val = val;
    symbol_table[symbolInd].level = level;
    symbol_table[symbolInd].addr = addr;
    symbol_table[symbolInd].mark = 0; // unmarked
    symbolInd++;

}

void program(){
    emit(JMP,0,0); // jump to main block
    block();
    if(tokenList[tokenInd] != periodsym){
        printf("%s",errorMessages[1]);
        return;
    }
    emit(SYS,0,3); // halt
}

void block(){
    constDeclaration();
    int numVars = varDeclaration();
    code[0].m = 3;
    emit(INC,0,3+numVars);
    statement();

}

void constDeclaration(){
    if(tokenList[tokenInd] == constsym){
        do{
            numConsts++;
            getNextToken();
            if(tokenList[tokenInd]!=identsym){
                printf("%s",errorMessages[2]);
                return;
            }
            if(symbolTableCheck(identifierList[tokenInd]) != -1){
                printf("%s",errorMessages[7]);
                return;
            }
            char identifierName[12];
            strcpy(identifierName, identifierList[tokenInd]);
            getNextToken();
            if(tokenList[tokenInd] != eqsym){
                printf("%s",errorMessages[4]);
                return;
            }
            getNextToken();
            if(tokenList[tokenInd] != numbersym){
                printf("%s",errorMessages[5]);
                return;
            }
            addToSymbolTable(1,identifierName, numList[tokenInd], 0, 0);
            getNextToken();
        }while(tokenList[tokenInd] == commasym);
        if(tokenList[tokenInd] != semicolonsym){
            printf("%s",errorMessages[6]);
            return;
        }
        getNextToken();
    }

}

int varDeclaration(){
    if(tokenList[tokenInd] == varsym){
        do{
            numVars++;
            getNextToken();
            if(tokenList[tokenInd]!=identsym){
                printf("%s",errorMessages[2]);
                return 0;
            }
            if(symbolTableCheck(identifierList[tokenInd]) != -1){
                printf("%s",errorMessages[7]);
                return 0;
            }
            addToSymbolTable(2, identifierList[tokenInd], 0, 0, 3+numVars-1);
            getNextToken();
        }while(tokenList[tokenInd] == commasym);
        if(tokenList[tokenInd] != semicolonsym){
            printf("%s",errorMessages[6]);
            return 0;
        }
        getNextToken();
    }
    return numVars;
}

void procedureDeclaration(){
    while(tokenList[tokenInd] == procsym){
        getNextToken();
        if(tokenList[tokenInd]!=identsym){
            printf("%s",errorMessages[2]);
            return;
        }
        getNextToken();
        if(tokenList[tokenInd] != semicolonsym){
            printf("%s",errorMessages[6]);
            return;
        }
        getNextToken();
        char identifierName[12];
        strcpy(identifierName, identifierList[tokenInd]);
        addToSymbolTable(3,identifierName, 0, 0, codeIdx);
    }
}

void statement(){
  if (tokenList[tokenInd] == identsym){
    int symIdx = symbolTableCheck(identifierList[tokenInd]);
    if(symIdx == -1){
        printf("%s",errorMessages[7]);
        return;
    }
    getNextToken();
    // not a var
    if(symbol_table[symIdx].kind != 2){
        printf("%s",errorMessages[8]);
        return;
    }
    if(tokenList[tokenInd]!=becomessym){
        printf("%s",errorMessages[9]);
        return;
    }
    getNextToken();
    expression();
    emit(STO,0,symbol_table[symIdx].addr);
    return;
  }
  if(tokenList[tokenInd] ==beginsym){
    do{
        getNextToken();
        statement();
    }while(tokenList[tokenInd]==semicolonsym);
    if(tokenList[tokenInd]!=endsym){
        printf("%s",errorMessages[10]);
        return;
    }
    getNextToken();
    return;
  }

  // if token is ifsym
  if(tokenList[tokenInd]==ifsym){
    getNextToken();
    condition();
    int jpcIdx = codeIdx;
    emit(JPC,0,0);
    if(tokenList[tokenInd]!=thensym){
        printf("%s",errorMessages[11]);
        return;
    }
    getNextToken();
    statement();
    code[jpcIdx].m = codeIdx;
    if(tokenList[tokenInd] != fisym){
        exit(1);

    }
    getNextToken();
    return;
  }

  if(tokenList[tokenInd] == whilesym){
        getNextToken();
        int loopIdx = codeIdx;
        condition();
        if(tokenList[tokenInd]!=dosym){
            printf("%s",errorMessages[12]);
            return;
        }
        getNextToken();
        int jpcIdx = codeIdx;
        emit(JPC,0,0);
        statement();
        emit(JMP,0,loopIdx);
        code[jpcIdx].m = codeIdx;
        return;
    }

    //readsym
    if(tokenList[tokenInd] == readsym){
        getNextToken();
        if(tokenList[tokenInd] != identsym){
            printf("%s",errorMessages[2]);
            return;
        }
        int symIdx = symbolTableCheck(identifierList[tokenInd]);
        if(symIdx == -1){
            printf("%s",errorMessages[7]);
            return;
        }
        if(symbol_table[symIdx].kind != 2){
            printf("%s",errorMessages[8]);
            return;
        }
        getNextToken();
        emit(SYS,0,2); // read
        emit(STO,0,symbol_table[symIdx].addr);
        return;
    }
    //writesym
    if(tokenList[tokenInd] == writesym){
        getNextToken();
        expression();
        emit(SYS,0,1); // write
        return;
    }
}

void condition(){
    if(tokenList[tokenInd] == evensym){
        getNextToken();
        expression();
        emit(OPR,0,EVEN);
    }else{
        expression();
        if(tokenList[tokenInd] == eqsym){
            getNextToken();
            expression();
            emit(OPR,0,EQUL);
        }else if(tokenList[tokenInd] == neqsym){
            getNextToken();
            expression();
            emit(OPR,0,NEQ);
        }else if(tokenList[tokenInd] == lessym){
            getNextToken();
            expression();
            emit(OPR,0,LSS);
        }else if(tokenList[tokenInd] == leqsym){
            getNextToken();
            expression();
            emit(OPR,0,LEQ);
        }else if(tokenList[tokenInd] == gtrsym){
            getNextToken();
            expression();
            emit(OPR,0,GTR);
        }else if(tokenList[tokenInd] == geqsym){
            getNextToken();
            expression();
            emit(OPR,0,GEQ);
        }else{
            printf("%s",errorMessages[13]);
            return;
        }
    }
}

void expression(){
    if(tokenList[tokenInd] == minussym){
        getNextToken();
        term();
        emit(OPR,0,SUB);
        while(tokenList[tokenInd] == plussym || tokenList[tokenInd] == minussym){
            if(tokenList[tokenInd] == plussym){
                getNextToken();
                term();
                emit(OPR,0,ADD);
            }else{
                getNextToken();
                term();
                emit(OPR,0,SUB);
            }
        }
    }else{
        if(tokenList[tokenInd] == plussym){
            getNextToken();
        }
        term();
        while(tokenList[tokenInd] == plussym || tokenList[tokenInd] == minussym){
            if(tokenList[tokenInd] == plussym){
                getNextToken();
                term();
                emit(OPR,0,ADD);
            }else{
                getNextToken();
                term();
                emit(OPR,0,SUB);
            }
        }
    }
}

void term(){
    factor();
    while(tokenList[tokenInd] == multsym || tokenList[tokenInd] == slashsym /* || tokenlist[tokenInd] == modsym*/){
        if(tokenList[tokenInd] == multsym){
            getNextToken();
            factor();
            emit(OPR,0,MUL);
        }else if(tokenList[tokenInd] == slashsym){
            getNextToken();
            factor();
            emit(OPR,0,DIV);
        /*}else{
            getNextToken();
            factor();
            emit(OPR,0,MOD);
        */}
    }
}

void factor(){
    if(tokenList[tokenInd] == identsym){
        int symIdx = symbolTableCheck(identifierList[tokenInd]);
        if(symIdx == -1){
            printf("%s",errorMessages[7]);
            return;
        }
        if(symbol_table[symIdx].kind == 1){
            emit(LIT,0,symbol_table[symIdx].val);
        }else if(symbol_table[symIdx].kind == 2){
            emit(LOD,0,symbol_table[symIdx].addr);
        }
        getNextToken();
    }else if(tokenList[tokenInd] == numbersym){
        emit(LIT,0,numList[tokenInd]);
        getNextToken();
    }else if(tokenList[tokenInd] == lparentsym){
        getNextToken();
        expression();
        if(tokenList[tokenInd] != rparentsym){
            printf("%s",errorMessages[14]);
            return;
        }
        getNextToken();
    }else{
        printf("%s",errorMessages[15]);
        return;
    }
}

void getNextToken(){
    if(tokenInd < tokenCount-1)
    {
     tokenInd++;
    }
}

void emit(int op, int l, int m){
    code[codeIdx].op = op;
    code[codeIdx].l = l;
    code[codeIdx].m = m;
    codeIdx++;
}

//function that reads the file. 
void readFile(){
    FILE* f = fopen("tokens.txt", "r");
    if (f == NULL) {
        // Using perror is helpful as it tells you *why* the open failed
        perror("Error opening tokens.txt");
        exit(1); // Exit if the file can't be opened
    }

    int token_value;
    while(fscanf(f, "%d", &token_value) != EOF) {
        tokenList[tokenCount] = token_value;

        if(token_value == identsym){
            fscanf(f, "%s", identifierList[tokenCount]);
        } else if(token_value == numbersym){
            fscanf(f, "%d", &numList[tokenCount]);
        }
        
        tokenCount++;
    }
    fclose(f);
}
//prints the generated code
void printCode(){
    const char* op_names[] = {"", "LIT", "OPR", "LOD", "STO", "CAL", "INC", "JMP", "JPC", "SYS"};
    
    printf("Assembly Code:\n");
    printf("Line\tOP\tL\tM\n");
    for (int i = 0; i < codeIdx; i++) {
        printf("%d\t%s\t%d\t%d\n", i, op_names[code[i].op], code[i].l, code[i].m);
    }
}
// prints the symbol table
void printTable(){
    printf("\nSymbol Table:\n");
    printf("Kind | Name\t\t | Value | Level | Address | Mark\n");
    printf("-----------------------------------------------------\n");
    for (int i = 0; i < symbolInd; i++) {
        printf("%-5d| %-15s| %-6d| %-6d| %-8d| %d\n",
               symbol_table[i].kind,
               symbol_table[i].name,
               symbol_table[i].val,
               symbol_table[i].level,
               symbol_table[i].addr,
               symbol_table[i].mark);
    }
}
//prints out to elf.txt also checks for skipsym error
void printFile(){

    FILE* out = fopen("elf.txt", "w");
    if (out == NULL) {
        perror("Error writing to elf.txt");
        return;
    }
    for(int i = 0; i<tokenCount;i++){
        if(tokenList[i]==skipsym){
            printf("%s",errorMessages[0]);
            return;
        }
    }
    for (int i = 0; i < codeIdx; i++) {
        fprintf(out, "%d %d %d\n", code[i].op, code[i].l, code[i].m);
    }
    fclose(out);
}
int main(){
    readFile();

    for(int i=0;i<tokenCount;i++){
        if(tokenList[i]==skipsym){
            printf("%s",errorMessages[0]);
            return 0;
        }
    }
    program();

    printCode();
    printTable();
    printFile();
    return 0;
}