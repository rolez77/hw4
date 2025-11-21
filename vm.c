/*
Assignment :
vm . c - Implement a P - machine virtual machine
Authors : Jovany Lopez, Andrew Irimie
Language : C ( only )
To Compile :
gcc -O2 -Wall -std=c11 -o vm vm.c
To Execute ( on Eustis ) :
./ vm input . txt
where :
input . txt is the name of the file containing PM /0 instructions ;
each line has three integers ( OP L M )
Notes :
- Implements the PM /0 virtual machine described in the homework
instructions .
- No dynamic memory allocation or pointer arithmetic .
- Does not implement any VM instruction using a separate function .
- Runs on Eustis .
Class : COP 3402 - Systems Software - Fall 2025
Instructor : Dr . Jie Lin
Due Date : Friday , September 12 th , 2025
*/



#include <stdio.h>
#include <stdlib.h>


int pas[500] = {0};
void printHelper(int bp, int sp);
int base(int,int);

typedef struct ir{
    int op;
    int l;
    int m;
} InstructionRegister;

int main(int argc, char *argv[]) {

    //Check arg count
    if (argc != 2) {
        fprintf(stderr, "Usage: %s elf.txt\n", argv[0]);
        return 1;
    }
    //initalize variables
    int op, l, m;
    // adress
    int addy = 499;
    // program counter
    int pc = 499;
    // stack pointer
    int sp;
    // base pointer
    int bp;
    // halt flag
    int halt = 0;
    // instruction register
    InstructionRegister ir;

    // file handling
    FILE* f = fopen(argv[1],"r");


    if(f == NULL){
        printf("Error opening file\n");
        return 1;
    }


    // read file into pas array
    while(fscanf(f, "%d %d %d", &op, &l, &m) ==3){
        pas[addy] = op;
        pas[addy-1] = l;
        pas[addy-2] = m;
        addy-=3;
    }
    fclose(f);
    
    sp = addy+1;
    bp = sp-1;
    //fetch loop
    printf("\t\t\tL\tM\t     PC   BP   SP   \tstack\n");
    printf("Inital values: \t\t\t\t     %d  %d  %d \n", pc, bp, sp);
    while(halt!=1){
        
        ir.op = pas[pc];
        ir.l = pas[pc-1];
        ir.m = pas[pc-2];
        pc-=3;

        switch(ir.op){
            case 1: // LIT
            sp--;
            pas[sp] = ir.m;
            printf("LIT: \t\t\t%d\t%d\t     %d  %d  %d", ir.l, ir.m, pc, bp, sp);
            printHelper(bp,sp);
            break;
            case 2: // OPR
                //RTN
                if(ir.m == 0){
                    sp =bp +1;
                    bp = pas[sp-2];
                    pc = pas[sp-3];
                }
                // ADD
                else if(ir.m == 1){
                    pas[sp+1] =  pas[sp+1] + pas[sp];
                    sp++;
                }
                //sub
                else if(ir.m == 2){
                    pas[sp+1] =  pas[sp+1] - pas[sp];
                    sp++;
                }
                //mul
                else if(ir.m == 3){
                    pas[sp+1] =  pas[sp+1] * pas[sp];
                    sp++;
                }
                //div
                else if(ir.m == 4){
                    pas[sp+1] =  pas[sp+1] / pas[sp];
                    sp++;
                }
                //eql
                else if(ir.m == 5){
                    pas[sp+1] =  pas[sp+1] == pas[sp];
                    sp++;
                }
                //neq
                else if(ir.m == 6){
                    pas[sp+1] =  pas[sp+1] != pas[sp];
                    sp++;
                }
                //lss
                else if(ir.m == 7){
                    pas[sp+1] =  pas[sp+1] < pas[sp];
                    sp++;
                }
                //leq
                else if(ir.m == 8){
                    pas[sp+1] =  pas[sp+1] <= pas[sp];
                    sp++;
                }
                //gtr
                else if(ir.m == 9){
                    pas[sp+1] =  pas[sp+1] > pas[sp];
                    sp++;
                }
                //geq
                else if(ir.m == 10){
                    pas[sp+1] =  pas[sp+1] >= pas[sp];
                    sp++;
                //even
                }else if(ir.m == 11){
                    pas[sp] = (pas[sp] % 2 == 0);
                }
                printf("OPR: \t\t\t%d\t%d\t     %d  %d  %d", ir.l, ir.m, pc, bp, sp);
                printHelper(bp,sp);
                break;
            case 3: // LOD
                sp--;
                pas[sp] = pas[base(bp,ir.l) - ir.m];
                printf("LOD: \t\t\t%d\t%d\t     %d  %d  %d", ir.l, ir.m, pc, bp, sp);
                printHelper(bp,sp);
                break;
            case 4: // STO
                pas[base(bp, ir.l) - ir.m] = pas[sp];
                sp++;
                printf("STO: \t\t\t%d\t%d\t     %d  %d  %d", ir.l, ir.m, pc, bp, sp);
                printHelper(bp,sp);
                break;
            case 5: // CAL
                pas[sp-1] = base(bp, ir.l);
                pas[sp-2] = bp;
                pas[sp-3] = pc;
                bp = sp -1;
                pc = 499-ir.m;
                printf("CAL: \t\t\t%d\t%d\t     %d  %d  %d", ir.l, ir.m, pc, bp, sp);
                printHelper(bp,sp);
                break;
            case 6: // INC
                sp -= ir.m;
                printf("INC: \t\t\t%d\t%d\t     %d  %d  %d", ir.l, ir.m, pc, bp, sp);
                printHelper(bp,sp);
                break;
            case 7: // JMP
                pc  = 499 - ir.m;
                printf("JMP: \t\t\t%d\t%d\t     %d  %d  %d", ir.l, ir.m, pc, bp, sp);
                printHelper(bp,sp);
                break;
            case 8: // JPC
                if(pas[sp] == 0){
                    pc = 499 - ir.m;
                }
                sp++;
                printf("JPC: \t\t\t%d\t%d\t     %d  %d  %d", ir.l, ir.m, pc, bp, sp);
                printHelper(bp,sp);
                break;   
            case 9:
                // m1
                if(ir.m == 1){
                    printf("Output result is: %d\n", pas[sp]);
                    sp++;
                    printf("SYS: \t\t\t%d\t%d\t     %d  %d  %d", ir.l, ir.m, pc, bp, sp);
                    printHelper(bp,sp);
                }
                //m2
                else if(ir.m ==2){
                    int input;
                    printf("Please input an integer: ");
                    scanf("%d", &input);
                    sp--;
                    pas[sp] = input;
                    printf("SYS: \t\t\t%d\t%d\t     %d  %d  %d", ir.l, ir.m, pc, bp, sp);
                    printHelper(bp,sp);
                }else if(ir.m == 3){
                    printf("SYS: \t\t\t%d\t%d\t     %d  %d  %d", ir.l, ir.m, pc, bp, sp);
                    printHelper(bp,sp);
                    halt = 1;
                }
                break;
            default:
                printf("Invalid OP code: %d\n",ir.op);
                halt=1;
                break;
        }
    }
    return 0;   
}


/* Find base L levels down from the current activation record */
int base ( int BP , int L ) {
    int arb = BP ; // activation record base
    while ( L > 0) {
        arb = pas [ arb ]; // follow static link
        L--;
    }
    return arb ;
}

void printHelper(int bp, int sp){
    printf("\t");
    if (sp >= 434) {
        // No local variables allocated yet - print everything together
        for (int i = 439; i >= sp; i--) {
            printf("%d ", pas[i]);
        }
    } else {
        // Local variables allocated - show main stack | procedure stack
        for (int i = 439; i >= 434; i--) {
            printf("%d ", pas[i]);
        }
        printf("| ");
        for (int i = 433; i >= sp; i--) {
            printf("%d ", pas[i]);
        }
    }
    printf("\n");
}