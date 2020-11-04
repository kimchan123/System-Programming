#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
typedef enum {
    ADD = 0x18,
    ADDF = 0x58,
    ADDR = 0x90,
    AND = 0x40,
    CLEAR = 0xB4,
    COMP = 0x28,
    COMPF = 0x88,
    COMPR = 0xA0,
    DIV = 0x24,
    DIVF = 0x64,
    DIVR = 0x9C,
    FIX = 0xC4,
    FLOAT = 0xC0,
    HIO = 0xF4,
    J = 0x3C,
    JEQ = 0x30,
    JGT = 0x34,
    JLT = 0x38,
    JSUB = 0x48,
    LDA = 0x00,
    LDB = 0x68,
    LDCH = 0x50,
    LDF = 0x70,
    LDL = 0x08,
    LDS = 0x6C,
    LDT = 0x74,
    LDX = 0x04,
    LPS = 0xD0,
    MUL = 0x20,
    MULF = 0x60,
    MULR = 0x98,
    NORM = 0xC8,
    OR = 0x44,
    RD = 0xD8,
    RMO = 0xAC,
    RSUB = 0x4C,
    SHIFTL = 0xA4,
    SIO = 0xF0,
    SSK = 0xEC,
    STA = 0x0C,
    STB = 0x78,
    STCH = 0x54,
    STF = 0x80,
    STI = 0xD4,
    STL = 0x14,
    STS = 0x7C,
    STSW = 0xE8,
    STT = 0x84,
    STX = 0x10,
    SUB = 0x1C,
    SUBF = 0x5C,
    SUBR = 0x94,
    SVC = 0xB0,
    TD = 0xE0,
    TIO = 0xF8,
    TIX = 0x2C,
    TIXR = 0xB8,
    WD = 0xDC
}Mnemonic;





 enum {
    lt, eq, gt
} CCstatus;

typedef struct {
    Mnemonic mnemonic;
    int format;
    int address;
    union {
        int tar;
        int immediate;
        int reg[2];
    }operand;
    int index;
} obj;
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

typedef struct Estab {
	char esymbol[20];
	int loc;
	int len;
	int flag;
	struct Estab* nextEstab;
}Estab;

typedef struct BP {
	int address;
	struct BP* nextBP;
}BP;
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
void link(FILE* fp);
void makeEstab(Estab* TempEstab);
void loader(FILE* fp);
void printload();
void transstring(int i, char* s, int len);
void printbp();
void clearbp();
int hextodec(char* s);
void runobj();
int getTarAdd(int cur_addr, int format);
int getMemo(int add, int h);
void SetMemo(int add, int b, int val);
void DumpRegs();
int immediateAddress(int add, int format);
int simpleAddress(int add, int format);
int SICAddress(int add);
int indirectAddress(int add, int format);
int bpfind(int i);