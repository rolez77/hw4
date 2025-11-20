/*
Assignment :
lex - Lexical Analyzer for PL /0
Author : Jovany Lopez and Andrew Irimie
Language : C ( only )
To Compile :
gcc - O2 - std = c11 -o lex lex . c
To Execute ( on Eustis ):
./ lex < input file >
where :
< input file > is the path to the PL /0 source program
Notes :
- Implement a lexical analyser for the PL /0 language .
- The program must detect errors such as
- numbers longer than five digits
- identifiers longer than eleven characters
- invalid characters .
- The output format must exactly match the specification .
- Tested on Eustis .
Class : COP 3402 - System Software - Fall 2025
Instructor : Dr . Jie Lin
Due Date : Friday , October 3 , 2025 at 11:59 PM ET
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_DIG 5
#define MAX_CHAR_LEN 11
#define MAX_SYMBOLS 500

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

typedef struct{
    TokenType type;
    char lexeme[MAX_CHAR_LEN];
    int value; 
}Token;

Token tokenList[MAX_SYMBOLS];
int tokenCount = 0;

TokenType reserveWord(const char* str){
    if(strcmp(str, "begin") == 0) return beginsym;
    if(strcmp(str, "end") == 0) return endsym;
    if(strcmp(str, "if") == 0) return ifsym;
    if(strcmp(str, "fi") == 0) return fisym;
    if(strcmp(str, "then") == 0) return thensym;
    if(strcmp(str, "while") == 0) return whilesym;
    if(strcmp(str, "do") == 0) return dosym;
    if(strcmp(str, "call") == 0) return callsym;
    if(strcmp(str, "const") == 0) return constsym;
    if(strcmp(str, "var") == 0) return varsym;
    if(strcmp(str, "procedure") == 0) return procsym;
    if(strcmp(str, "write") == 0) return writesym;
    if(strcmp(str, "read") == 0) return readsym;
    if(strcmp(str, "else") == 0) return elsesym;
    if(strcmp(str, "even") == 0) return evensym;
    return identsym;
}


TokenType specialSymbols(const char* str){
    if(strcmp(str, "+") == 0) return plussym;
    if(strcmp(str, "-") == 0) return minussym;
    if(strcmp(str, "*") == 0) return multsym;
    if(strcmp(str, "/") == 0) return slashsym;
    if(strcmp(str, "=") == 0) return eqsym;
    if(strcmp(str, "<>") == 0) return neqsym;
    if(strcmp(str, "<") == 0) return lessym;
    if(strcmp(str, "<=") == 0) return leqsym;
    if(strcmp(str, ">") == 0) return gtrsym;
    if(strcmp(str, ">=") == 0) return geqsym;
    if(strcmp(str, "(") == 0) return lparentsym;
    if(strcmp(str, ")") == 0) return rparentsym;
    if(strcmp(str, ",") == 0) return commasym;
    if(strcmp(str, ";") == 0) return semicolonsym;
    if(strcmp(str, ".") == 0) return periodsym;
    if(strcmp(str, ":=") == 0) return becomessym;
    return skipsym;
}

//Skip empty space
int emptySpace(char c){
    return c == ' ' || c == '\n' || c == '\t';
}

//Check if character is a letter
int isLetter(int c){
    return (c>='a'&&c<='z') || (c>='A'&&c<='Z');
}

//Check if a character is a digit
int isDigit(int c){
    return (c>='0'&&c<='9');
}

// function to add token to the list, takes in type, lexeme and value
void addToken(TokenType type, const char* lexeme, int val){
    // only add if we have space
    if(tokenCount < MAX_SYMBOLS){
        tokenList[tokenCount].type = type;
        strcpy(tokenList[tokenCount].lexeme, lexeme);
        tokenList[tokenCount].value = val;
        tokenCount++;
    }
}

int main(int argc, char *argv[]) {
    if(argc != 2){
        return 1;
    }
    //file handling
    FILE* f = fopen(argv[1],"r");
    if(f == NULL){
        printf("Error opening file\n");
        return 1;
    }
    // read file content
    char characters[1000];
    int index = 0;
    int character;
    while((character = fgetc(f)) != EOF){
        characters[index++] = character;
    }
    characters[index] = '\0';
    fclose(f);
    // print out the source program that was read from the file
    //printf("Source Program :\n");
    //printf("%s\n", characters);

    int i = 0;
    // loop until we read the terminating null character
    while(characters[i]!='\0'){
        
        char c = characters[i];
        // continue on empty
        if(emptySpace(c)){
            i++;
            continue;
        }

        //This will check for comments
        if(c == '/' && characters[i+1] == '*'){
            i += 2; // skip /*
            while(characters[i] != '\0' && !(characters[i] == '*' && characters[i+1] == '/')){
                i++;
            }
            if(characters[i] == '\0'){
                i+=2;
            };
            continue;
        }
        // identifier check
        if(isLetter(c)){
            char buffer[50];
            int j = 0;
            // read while we have letters or digits
            while(isLetter(characters[i]) || isDigit(characters[i])){
                if(j < MAX_CHAR_LEN ){
                    buffer[j++] = characters[i];
                }
                i++;
            }
            buffer[j] = '\0';
            // if the string is too long return error
            if(j > MAX_CHAR_LEN){
                addToken(identsym, "Too long", 0);
            }
            //otherwise, add the token, checking if it's a reserved word
            TokenType t = reserveWord(buffer);
            addToken(t, buffer, 0);
            continue;
        }
        // number check
        if(isDigit(c)){
            char buffer[50];
            int j = 0;
            // read while we have digits
            while(isDigit(characters[i])){
                if(j < MAX_CHAR_LEN ){
                    buffer[j++] = characters[i];
                }
                i++;
            }
            buffer[j] = '\0';
            // if the number is too long return error
            if(j > MAX_DIG){
                addToken(numbersym, "# too long", 0);
            }
            //add
            addToken(numbersym, buffer, atoi(buffer));
            continue;   
        }

        // special symbol check
        char buf[3] = {0};
        buf[0] = characters[i]; buf[1] = '\0';
        // checks for <>, <=, >= and :=
        if((c == '<' && (characters[i+1]=='>'||characters[i+1]=='=')) ||
           (c == '>' && characters[i+1]=='=') ||
           (c == ':' && characters[i+1]=='=')){
            buf[1] = characters[i+1];
            buf[2] = '\0';
            i += 2;
        } else {
            i++;
        }
        // get the token type
        TokenType t = specialSymbols(buf);
        if(t == skipsym){
            addToken(skipsym, "Invalid symbol", 0);
        } else {
            addToken(t, buf, 0);
        }
    }

    // print out the tables 
    //printf("Lexeme Table :\n");
    //printf("\n");
    //printf("lexeme\ttoken type\n");
    for(int j=0;j<tokenCount;j++){
        //printf("%s\t%d\n", tokenList[j].lexeme, tokenList[j].type);
    }
    //printf("\nToken List:\n\n");

    FILE *tf = fopen("tokens.txt", "w");
    if (tf == NULL) {
        fprintf(stderr, "Could not open tokens.txt for writing\n");
        return 1;
    }

    for (int j = 0; j < tokenCount; j++) {
        if (tokenList[j].type == identsym || tokenList[j].type == numbersym) {
            //printf("%d %s ", tokenList[j].type, tokenList[j].lexeme);
            fprintf(tf, "%d %s ", tokenList[j].type, tokenList[j].lexeme);
        } else if (tokenList[j].type == skipsym) {
            continue;
        } else {
            // print to console
            //printf("%d ", tokenList[j].type);
            // mirror to tokens.txt
            fprintf(tf, "%d ", tokenList[j].type);
        }
    }
    //printf("\n");
    fprintf(tf, "\n");
    fclose(tf);


    
    return 0;
}