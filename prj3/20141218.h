#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
typedef struct LinkHistory {
	char Historyins[100];
	struct LinkHistory* nextHistory;
}LinkHistory;

typedef struct LinkOpcode {
	char Opcode[20], Format[20];
	int Value;
	struct LinkOpcode* nextOpcode;
}LinkOpcode;

typedef struct Symbol {
	char sym[20];
	int loc;	
	struct Symbol* nextSymbol;
}Symbol;

typedef struct Modify {
	int loc;
	struct Modify* nextModify;
}Modify;
int makedecimal(char* address);
void menu();
void menu_dir();
void menu_help();
void menu_history();
void menu_dump();
void menu_edit();
void menu_fill();
void menu_reset();
void menu_opcode();
void makehash();
void menu_symbol();
int hash(char *s);
void menu_opcodelist();
int assemble(FILE* fp);
void assemble1(FILE* fp);
char regist(char* re);
LinkOpcode* FindOp(char* Code);
Symbol* FindSymbol(char* Code);
void makeSymbol(char* asm1, int loc_start);