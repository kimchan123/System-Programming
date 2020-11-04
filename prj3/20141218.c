#include "20141218.h"
int locationflag = 0;
int menuflag; //명령어 값
int edit_address, edit_value; //edit 명령어 시 주소와 value 값
int hash_value; //hash 함수 return 값
int start, end; //dump 명령어 시 시작 주소와 끝 주소 값.
int fill_start, fill_end, fill_value; //fill 명령어 시 시작, 끝 주소와 value 값
int last_memory; //dump 입력시 마지막 주소 값
int basevalue; //base 값
int locstart, loclength; //시작 주소값
int location[100], line[100];//location, line 저장 값
char memory[65536][16];	//메모리
LinkHistory *Head, *Tail; //History Linked List
LinkOpcode *OpHead[20], *OpTail[20]; //Opcode Hash Linked List
Symbol *SymHead, *SymTail; //Symbol Linked List
Modify* ModifyHead, * ModifyTail; //Modification Linked List
//main 함수
int main() {
	// 초기화
	last_memory = -1;
	Head = NULL, Tail = NULL;
	for (int i = 0; i < 20; i++) {
		OpHead[i] = NULL, OpTail[i] = NULL;
	}
	makehash();
	while (1) {
		menuflag = 0;
		menu();
		if (menuflag == 1) menu_help();
		else if (menuflag == 2) menu_dir();
		else if (menuflag == 3) break;
		else if (menuflag == 4) menu_history();
		else if (menuflag == 5) menu_dump();
		else if (menuflag == 6) menu_edit();
		else if (menuflag == 7) menu_fill();
		else if (menuflag == 8) menu_reset();
		else if (menuflag == 9) menu_opcode();
		else if (menuflag == 10) menu_opcodelist();
		else if (menuflag == 13) menu_symbol();
	}

	return 0;
}
//16진수를 10진수로 바꾸는 함수
int makedecimal(char* address) {
	int result = 0;
	int len = strlen(address);
	for (int i = 0; i < len; i++) {
		result *= 16;
		if (address[i] >= '0' && address[i] <= '9') {
			result += address[i] - '0';
		}
		//16진수 대소문자 모두 가능
		else if (address[i] >= 'A' && address[i] <= 'F') {
			result += (address[i] - 'A') + 10;
		}
		else if (address[i] >= 'a' && address[i] <= 'f') {
			result += (address[i] - 'a') + 10;
		}
		else {
			result = -1;
			break;
		}
	}

	return result;
}
//명령어를 입력받고 올바른 명령어인지 확인 후 맞다면 history에 추가 그렇지 않다면 에러 체크.
void menu() {
	LinkHistory* TempHistory;
	LinkOpcode* TempHashHead;
	char instr[100], newinstr1[100], typecontext[100], asminstr[100];
	char* newinstr;
	int len, error = 0, hashindex;

	printf("sicsim> ");
	fgets(instr, sizeof(instr), stdin);

	len = strlen(instr);
	instr[len - 1] = '\0';
	strcpy(newinstr1, instr);

	newinstr = strtok(newinstr1, " ");

	if (newinstr != NULL) {
		if (strcmp(newinstr, "h") == 0 || strcmp(newinstr, "help") == 0) {
			menuflag = 1;
		}
		else if (strcmp(newinstr, "d") == 0 || strcmp(newinstr, "dir") == 0) {
			menuflag = 2;
		}
		else if (strcmp(newinstr, "q") == 0 || strcmp(newinstr, "quit") == 0) {
			menuflag = 3;
		}
		else if (strcmp(newinstr, "hi") == 0 || strcmp(newinstr, "history") == 0) {
			menuflag = 4;
		}
		else if (strcmp(newinstr, "du") == 0 || strcmp(newinstr, "dump") == 0) {
			start = -1, end = -1;
			newinstr = strtok(NULL, " ,");
			if (newinstr == NULL) menuflag = 5; //명령어에 dump 입력 시
			else {
				start = makedecimal(newinstr);
				newinstr = strtok(NULL, " ");
				if (!newinstr) { //명령어에 dump start 입력 시
					if (start >= 0 && start < 65536 * 16) menuflag = 5; //start주소가 0과 2^20사이면 올바른 명령어
					else error = 1;
				}
				else { //명령어에 dump start, end 입력 시
					end = makedecimal(newinstr);
					if (start >= 0 && start < 65536 * 16 && end >= 0 && end < 65536 * 16) { //start, end 주소가 0과 2^20사이이고
						if (start <= end) menuflag = 5; //start주소가 end주소 이하면 올바른 명령어
						else error = 2;
					}
					else
						error = 1;
				}
			}

		}
		else if (strcmp(newinstr, "e") == 0 || strcmp(newinstr, "edit") == 0) {
			newinstr = strtok(NULL, " ,");
			edit_address = makedecimal(newinstr);
			newinstr = strtok(NULL, " ");
			edit_value = makedecimal(newinstr);
			if (edit_address >= 0 && edit_address < 16 * 65536 && edit_value >= 0 && edit_value < 256) menuflag = 6;
			else error = 1;
		}

		else if (strcmp(newinstr, "f") == 0 || strcmp(newinstr, "fill") == 0) {
			newinstr = strtok(NULL, " ,");
			fill_start = makedecimal(newinstr);
			newinstr = strtok(NULL, " ,");
			fill_end = makedecimal(newinstr);
			newinstr = strtok(NULL, " ");
			fill_value = makedecimal(newinstr);
			if (fill_start >= 0 && fill_start < 16 * 65536 && fill_end >= 0 && fill_end < 16 * 65536 && fill_value >= 0 && fill_value < 256) {
				if (fill_start <= fill_end) menuflag = 7;
				else error = 2;
			}
			else
				error = 1;
		}
		else if (strcmp(newinstr, "reset") == 0) menuflag = 8;
		else if (strcmp(newinstr, "opcode") == 0) {
			hash_value = -1;
			newinstr = strtok(NULL, " ");
			hashindex = hash(newinstr);
			for (TempHashHead = OpHead[hashindex]; TempHashHead != NULL; TempHashHead = TempHashHead->nextOpcode) {
				//해당 hashindex값이 hashtable안에 있으면 올바른 명령어
				if (strcmp(newinstr, TempHashHead->Opcode) == 0) {
					hash_value = TempHashHead->Value;
					break;
				}
			}
			if (hash_value != -1) menuflag = 9;
			else error = 3;
		}
		else if (strcmp(newinstr, "opcodelist") == 0) menuflag = 10;
		else if (strcmp(newinstr, "assemble") == 0) {
			newinstr = strtok(NULL, " ");
			FILE* fileasm = fopen(newinstr, "r");
			if (fileasm) {
				if (!assemble(fileasm)) {
					fclose(fileasm);
					fileasm = fopen(newinstr, "r");
					assemble1(fileasm);
					menuflag = 11;
				}
			}
		}
		else if (strcmp(newinstr, "type") == 0) {
			newinstr = strtok(NULL, " ");
			FILE* typefile = fopen(newinstr, "r");
			if (typefile) {
				while (fgets(typecontext, sizeof(typecontext), typefile) != NULL) {
					len = strlen(typecontext);
					for (int i = 0; i < len; i++) printf("%c", typecontext[i]);
					menuflag = 12;
				}
			}
			else error = 4;
		}
		else if (strcmp(newinstr, "symbol") == 0) {
			menuflag = 13;
		}
	}
	
	//명령어가 유효하면 history linked list에 삽입.
	if (menuflag) {
		TempHistory = (LinkHistory*)malloc(sizeof(LinkHistory));
		TempHistory->nextHistory = NULL;
		for (int i = 0; i < len; i++) TempHistory->Historyins[i] = instr[i];
		if (!Head) {
			Head = TempHistory;
			Tail = TempHistory;
		}
		else {
			Tail->nextHistory = TempHistory;
			Tail = TempHistory;
		}
	}

	//그렇지 않다면 에러표시
	else {
		if (error == 0) printf("ERROR : WRONG INSTRUCTION!!\n");
		else if (error == 1) printf("ERROR : OUT OF RANGE!!\n");
		else if (error == 2) printf("ERROR : START IS GREATER THAN END!!\n");
		else if (error == 3) printf("ERROR : MNEMONIC DOESN'T EXIST!!\n");
		else if (error == 4) printf("ERROR : FILE DOESN'T EXIST!!\n");
	}
}
//명령어에 help 입력시 명령어 list 출력
void menu_help() {
	printf("h[elp]\n");
	printf("d[ir]\n");
	printf("q[uit]\n");
	printf("hi[story]\n");
	printf("du[mp] [start, end]\n");
	printf("e[dit] address, value\n");
	printf("f[ill] start, end, value\n");
	printf("reset\n");
	printf("opcode mnemonic\n");
	printf("opcodelist\n");
	printf("assemble filename");
	printf("type filename");
	printf("symbol");
}
//명령어에 dir입력시 디렉토리 내 파일들 출력
void menu_dir() {
	DIR* dd = opendir(".");
	struct dirent* dp;
	struct stat info;
	for (dp = readdir(dd); dp != NULL; dp = readdir(dd)) {
		lstat(dp->d_name, &info);
		printf("%s", dp->d_name);
		if (S_ISDIR(info.st_mode))printf("/");
		else if (info.st_mode & S_IXUSR)printf("*");
		printf("  ");
	}
	printf("\n");
}
//명령어에 history입력시 현재까지 입력한 명령어 출력
void menu_history() {
	int num = 0;
	LinkHistory* Temp;
	for (Temp = Head; Temp != NULL; Temp = Temp->nextHistory) {
		num++;
		printf("%d\t%s\n", num, Temp->Historyins);
	}
}
//명령어에 dump or dump start or dump start, end 입력 시 주소 출력
void menu_dump() {
	int address_start, address_end;
	int row, column;
	int unit;
	if (start == -1 && end == -1) {
		address_start = last_memory + 1;
		if (address_start >= 16 * 65536) address_start = 0;
		address_end = address_start + 159;
		last_memory = address_end;
	}
	else if (end == -1) {
		address_start = start;
		address_end = start + 159;
	}
	else {
		address_start = start;
		address_end = end;
	}
	if (address_end >= 16 * 65536) address_end = 16 * 65536 - 1;
	row = address_start / 16 * 16, column = (address_end+16) / 16 * 16;
	
	for (int i = row; i < column ; i+=16) {
		{
			printf("%05X ", i);
			for (int j = 0; j < 16; j++) {
				unit = i + j;
				if (unit <address_start || unit > address_end)
					printf("   ");
				else
					printf("%02X ", (int)memory[i/16][j]);
			}
			printf("; ");

			for (int j = 0; j < 16; j++) {
				unit = i + j;
				if (unit<address_start || unit>address_end)
					printf(".");
				else {
					if (0x20 <= (int)memory[i/16][j] && (int)memory[i/16][j] <= 0x7E)
						printf("%c", memory[i/16][j]);
					else
						printf(".");
				}
			}
			printf("\n");
		}

	}
}
//명령어에 edit address, value입력 시 해당 address 주소 값을 value로 변경
void menu_edit() {
	int row, col;
	row = edit_address / 16;
	col = edit_address % 16;
	memory[row][col] = edit_value;
}
//명령어에 fill start, end, value입력 시 시작 주소부터 끝 주소까지의 주소 값을 value로 변경
void menu_fill() {
	for (int i = fill_start; i <= fill_end; i++) {
		memory[i / 16][i % 16] = fill_value;
	}
}
//memory값을 0으로 초기화
void menu_reset() {
	for (int i = 0; i < 65536; i++)
		for (int j = 0; j < 16; j++)
			memory[i][j] = 0;
}
//hash table을 만드는 함수
void makehash() {
	LinkOpcode* TempHash;
	char line[100], opcode[20], format[20];
	int value, index;
	FILE* fp = fopen("opcode.txt", "r");
	
	while ((fgets(line, sizeof(line), fp) != NULL)){
		sscanf(line, "%X %s %s", &value, opcode, format);
		TempHash = (LinkOpcode*)malloc(sizeof(LinkOpcode));
		TempHash->Value = value;
		strcpy(TempHash->Opcode, opcode);
		strcpy(TempHash->Format, format);
		TempHash->nextOpcode = NULL;
		index = hash(opcode);
		if (!OpHead[index])
		{
			OpHead[index] = TempHash;
			OpTail[index] = TempHash;
		}
		else {
			OpTail[index]->nextOpcode = TempHash;
			OpTail[index] = TempHash;
		}
	}
}
//hash_function
int hash(char* s) {
	int len = strlen(s);
	int sum = 0;
	for (int i = 0; i < len; i++) {
		sum += s[i];
	}

	return sum % 20;
}	
//명령어에 입력 받은 값의 해당 opcode를 출력
void menu_opcode() {
	printf("opcode is %X\n", hash_value);
}
//hash_Table을 출력
void menu_opcodelist() {
	LinkOpcode* Temp;
	for (int i = 0; i < 20; i++) {
		printf("%d : ", i);
		for (Temp = OpHead[i]; Temp != NULL; Temp = Temp->nextOpcode) {
			printf("[%s, %X]", Temp->Opcode, Temp->Value);
			if(Temp->nextOpcode)
				printf(" -> ");
		}
		printf("\n");
	}
}

//강의시간 pass1에 해당하는 함수
int assemble(FILE* fileasm) {
	
	int loc_start=0, flag = 0, symbolflag = 0, error = 0, endflag = 0, baseflag = 0, basevalue = 0;
	char asminstr1[100], asm1[20], asm2[20], asm3[20], asm4[20], asm5[20], base[10];
	int i = 0, lvalue = 0, len = 0;
	LinkOpcode* FindOpcode;
	Symbol* TempSymbol=NULL;
	while (fgets(asminstr1, sizeof(asminstr1), fileasm) != NULL) {
		
		if (endflag || error)
			break;
		lvalue += 5;
		location[i] = loc_start;
		line[i] = lvalue;
		flag = 0;
		symbolflag = 0;
		sscanf(asminstr1, "%s %s %s %s", asm1, asm2, asm3, asm4);
		if (asm2[strlen(asm2) - 2] == ',') asm2[strlen(asm2) - 2] == NULL;
		if (strcmp(asm2, "start") == 0) {
			loc_start = makedecimal(asm3);
			locstart = loc_start;
			
		}
		else if (asm1[0] != '.') {
			FindOpcode = NULL;
			if (strcmp(asm1, "BASE") == 0 || strcmp(asm2, "START") == 0 || strcmp(asm1, "END") == 0) flag = 1;

			if (asm1[0] == '+')
				FindOpcode = FindOp(asm1 + 1);
			else
				FindOpcode = FindOp(asm1);
			
			if (!FindOpcode && flag != 1) { //symbol인 경우 symbol linked list에 추가
				makeSymbol(asm1, loc_start);
				symbolflag = 1;
			}
			if (!symbolflag) { //symbol이 아닌경우 symbol인 경우와 자리를 맞춰준다.
				for (int i = 0; i < 20; i++) {
					
					
					asm4[i] = asm3[i];
					asm3[i] = asm2[i];
					asm2[i] = asm1[i];
				}
				
			}

			FindOpcode = NULL;
			flag = 0;

			if (strcmp(asm2, "BASE") == 0) flag = 1;
			else if (strcmp(asm2, "BYTE") == 0) flag = 2;
			else if (strcmp(asm2, "RESW") == 0) flag = 3;
			else if (strcmp(asm2, "RESB") == 0) flag = 4;
			else if (strcmp(asm2, "END") == 0) flag = 5;

			if (asm2[0] == '+')
				FindOpcode = FindOp(asm2 + 1);
			else
				FindOpcode = FindOp(asm2);

			if (FindOpcode) { //opcode이면
				if (asm2[0] == '+' && strcmp(FindOpcode->Format, "3/4") == 0) // Format 4
				{
					if (strcmp(FindOpcode->Opcode, "JSUB") == 0)
						loc_start = loc_start + 4;
					else if (strcmp(FindOpcode->Opcode, "LDT") == 0)
						loc_start = loc_start + 4;
				}

				else if (asm2[0] != '+' && strcmp(FindOpcode->Format, "3/4") == 0) // Format 3
				{
					if (strcmp(FindOpcode->Opcode, "RSUB") == 0) loc_start = loc_start + 3;
					else if (strcmp(FindOpcode->Opcode, "RSUB") != 0)
						loc_start = loc_start + 3;

				}

				else if (asm2[0] != '+' && strcmp(FindOpcode->Format, "2") == 0) //Format2
				{

					loc_start = loc_start + 2;
				}
				else if (asm2[0] != '+' && strcmp(FindOpcode->Format, "1") == 0) //Format1
					loc_start = loc_start + 1;
				else error = 1;
			}
			else if (!FindOpcode && flag) { //directive
				if (flag == 1) { //BASE
					strcpy(base, asm3);
					baseflag = 1;
				}

				else if (flag == 2) { //BYTE
					len = strlen(asm3);
					if (asm3[0] == 'C') loc_start = loc_start + (len - 3);
					else if (asm3[0] == 'X') loc_start = loc_start + (len - 3) / 2;
					else error = 1;
				}

				else if (flag == 3) { //RESW
					locationflag = atoi(asm3);
					loc_start = loc_start + (3 * locationflag);
				}

				else if (flag == 4) { //RESB
					locationflag = atoi(asm3);
					loc_start = loc_start + locationflag;
				}

				else if (flag == 5) { // END
					endflag = 1;
				}
			}
		}
		i++;
	}
	if (baseflag)
	{
		for (TempSymbol = SymHead; TempSymbol != NULL; TempSymbol = TempSymbol->nextSymbol) {
			if (strcmp(TempSymbol->sym, base) == 0) {
				basevalue = TempSymbol->loc;
				break;
			}
		}
	}
	loclength = loc_start - locstart;
	if (error) printf("ERROR : %d LINE", lvalue);
	return error;
}

//Opcode Linked List에서 해당 코드의 주소를 반환
LinkOpcode* FindOp(char* Code) {
	LinkOpcode* LinkTemp = NULL;
	int index;
	index = hash(Code);
	
	for (LinkTemp = OpHead[index]; LinkTemp != NULL; LinkTemp = LinkTemp->nextOpcode) {
		if (strcmp(LinkTemp->Opcode, Code) == 0)
			break;
	}
	return LinkTemp;
}
//Symbol Linked List에서 해당 코드의 주소를 반환
Symbol* FindSymbol(char* Code) {
	Symbol* TempSymbol = NULL;
	for (TempSymbol = SymHead; TempSymbol != NULL; TempSymbol = TempSymbol->nextSymbol) {
		if (strcmp(TempSymbol->sym, Code) == 0)
			break;
	}
	return TempSymbol;
}
//강의에서 pass2에 해당하는 부분
void assemble1(FILE* fileasm) { //lst와 obj파일을 만든다.
	char asm_instr[100], asm_instr1[20], asm_instr2[20], asm_instr3[20], asm_instr4[20];
	char object[10], obtemp[20], obtemp1[20], objecttemp[200];
	int oblen = 0, len = 0, opvalue = 0, obflag = 0;
	int endflag = 0, error = 0, flag = 0, xvalue = 0, pcflag = 0;
	int locationtemp = 0;
	int count = 0;
	LinkOpcode* FindOpcode;
	Symbol* TempSymbol;
	Modify* ModifyTemp;
	int oblen1 = 0, oblen2 = 0;
	FILE* fp1 = fopen("2_5.obj", "w");
	FILE* fp2 = fopen("2_5.lst", "w");
	while (fgets(asm_instr, sizeof(asm_instr), fileasm) != NULL) {
		flag = 0, pcflag = 0;
		for (int i = 0; i < 10; i++)object[i] = 0;
		for (int i = 0; i < 20; i++)obtemp[i] = 0;
		locationtemp = location[count] - locstart;
		if (endflag || error)
			break;
		sscanf(asm_instr, "%s %s %s %s", asm_instr1, asm_instr2, asm_instr3, asm_instr4);
		if (asm_instr2[strlen(asm_instr2) - 2] == ',') asm_instr2[strlen(asm_instr2) - 2] == NULL;
		if (asm_instr1[0] != '.') {
			FindOpcode = NULL;
			if (asm_instr1[0] == '+')
				FindOpcode = FindOp(asm_instr1 + 1);
			else
				FindOpcode = FindOp(asm_instr1);

			if (strcmp(asm_instr1, "BASE") == 0 || strcmp(asm_instr1, "START") == 0 || strcmp(asm_instr1, "END") == 0) flag = 1;
			if (FindOpcode || flag) {

				for (int i = 0; i < 20; i++) { //symbol이 아닌경우와 맞춰준다.
					asm_instr4[i] = asm_instr3[i];
					asm_instr3[i] = asm_instr2[i];
					asm_instr2[i] = asm_instr1[i];
				}
			}

			FindOpcode = NULL;
			if (asm_instr2[0] == '+')
				FindOpcode = FindOp(asm_instr2 + 1);
			else
				FindOpcode = FindOp(asm_instr2);

			flag = 0;
			obflag = 0;
			if (strcmp(asm_instr2, "BASE") == 0) flag = 1;
			else if (strcmp(asm_instr2, "BYTE") == 0) flag = 2;
			else if (strcmp(asm_instr2, "RESW") == 0) flag = 3;
			else if (strcmp(asm_instr2, "RESB") == 0) flag = 4;
			else if (strcmp(asm_instr2, "END") == 0) flag = 5;
			else if (strcmp(asm_instr2, "START") == 0)flag = 6;
			if (flag) {
				obflag = 3;
				if (flag == 2) {
					len = strlen(asm_instr3);
					if (asm_instr3[0] == 'X') { //BYTE
						oblen = 0;
						for (int i = 2; i < len - 1; i++) {
							object[oblen++] = asm_instr3[i];
						}
						object[oblen] = NULL;
					}
					else {
						oblen = 0;
						for (int i = 2; i < len - 1; i++) {
							sprintf(obtemp, "%02x", (int)asm_instr3[i]);
							object[oblen] = obtemp[0];
							object[oblen + 1] = obtemp[1];
							oblen += 2;
						}
						object[oblen] = 0;
					}
				}

				else if (flag == 3 || flag == 4) { //RESB와 RESW인 경우
					obflag = 1;
				}
				else if (flag == 5) endflag = 1;
				else if (flag == 6) //START인 경우
					obflag = 2;

			}
			else if (FindOpcode) { //Opcode이면
				obflag = 3;
				opvalue = FindOpcode->Value;
				if (asm_instr2[0] != '+' && strcmp(FindOpcode->Format, "1") == 0) //Format1
				{
					sprintf(obtemp, "%02x", opvalue);
					object[0] = obtemp[0];
					object[1] = obtemp[1];
					object[2] = NULL;
				}
				else if (asm_instr2[0] != '+' && strcmp(FindOpcode->Format, "2") == 0) //Format2
				{

					sprintf(obtemp, "%02x", opvalue);
					object[0] = obtemp[0];
					object[1] = obtemp[1];
					object[2] = regist(asm_instr3);
					object[3] = regist(asm_instr4);
					object[4] = NULL;
				}
				else {
					if (asm_instr3[0] == '@') opvalue += 2;//indirect addressing(n=1, i=0)
					else if (asm_instr3[0] == '#') opvalue += 1; //immediate addressing;(n=0, i=1)
					else opvalue += 3; //simple addressing(n=1, i=1)
					sprintf(obtemp, "%02x", opvalue);
					object[0] = obtemp[0];
					object[1] = obtemp[1];
					if (strcmp(asm_instr2, "RSUB") != 0) {
						if (asm_instr[4] == 'X') xvalue = 8;  //x=1
						else xvalue = 0;
						TempSymbol = NULL;
						if (asm_instr3[0] == '#' || asm_instr3[0] == '@')
							TempSymbol = FindSymbol(asm_instr3 + 1);
						else
							TempSymbol = FindSymbol(asm_instr3);

						if (!TempSymbol) {
							if (asm_instr3[0] == '#' || asm_instr3[0] == '@') opvalue = atoi(asm_instr3 + 1);
							else opvalue = atoi(asm_instr3);
						}
						else
							opvalue = TempSymbol->loc;
						if (asm_instr2[0] == '+') {
							xvalue = xvalue + 1; //e=1
							sprintf(obtemp, "%x", xvalue);
							object[2] = obtemp[0];
							sprintf(obtemp, "%05x", opvalue);
							for (int i = 0; i < 5; i++)object[i + 3] = obtemp[i];
							object[8] = NULL;
							if (TempSymbol) //symbol이면 modification linked list에 추가
							{
								ModifyTemp = (Modify*)malloc(sizeof(Modify));
								ModifyTemp->nextModify = NULL;
								ModifyTemp->loc = locationtemp + 1;
								if (ModifyHead == NULL) {
									ModifyHead = ModifyTemp;
									ModifyTail = ModifyTemp;
								}
								else {
									ModifyTail->nextModify = ModifyTemp;
									ModifyTail = ModifyTemp;
								}
							}
						}
						else if (TempSymbol) {
							opvalue = opvalue - location[count];
							if (opvalue >= -2048 && opvalue <= 2047) // pc
							{
								pcflag = 1;
								opvalue = opvalue & 0xFFF;
								sprintf(obtemp, "%03x", opvalue);
								xvalue = xvalue + 2; //p = 1
							}
							else { //base
								opvalue = opvalue + location[count];
								opvalue = opvalue - basevalue;
								if (opvalue >= 0 && opvalue <= 4095) {
									sprintf(obtemp, "%03x", opvalue);
									xvalue = xvalue + 4; //b=1
								}
							}

							if (pcflag) {
								for (int i = 0; i < 3; i++) object[i + 3] = obtemp[i];
								object[6] = NULL;
							}
						}
						else {
							sprintf(obtemp, "%x", opvalue);
							for (int i = 0; i < 3; i++) object[i + 3] = obtemp[i];
							object[6] = NULL;
						}

						sprintf(obtemp, "%x", xvalue);
						object[2] = obtemp[0];
					}
					else {
						for (int i = 2; i < 6; i++) object[i] = '0';
						object[6] = NULL;
					}
					sprintf(obtemp, "%x", xvalue);
					object[2] = obtemp[0];
				}

			}


		}
		//lst파일을 만드는 코드
		asm_instr[strlen(asm_instr) - 1] = NULL;
		fprintf(fp2, "\t%d", line[count]);
		if (object[0] == NULL && (flag != 2 && flag != 3 && flag != 4)) fprintf(fp2, "\t\t\t");
		else {
			sprintf(obtemp1, "\t%04x", location[count]);
			fprintf(fp2, "\t%s", obtemp1);
		}
		fprintf(fp2, "\t%-30s", asm_instr);
		if (object[0] != NULL) fprintf(fp2, "\t%s", object);
		fprintf(fp2, "\n");
		//obj파일을 만드는 코드
		if (obflag == 2) {
			fprintf(fp1, "HCOPY  ", stdin);
			fprintf(fp1, "%06x", locstart);
			fprintf(fp1, "%06x\n", loclength);
		}
		else {
			oblen1 = strlen(object);
			if (!oblen2 && object[0])
			{
				objecttemp[0] = 'T';
				sprintf(obtemp, "%06x", location[count]);
				for (int i = 0; i < 6; i++)objecttemp[i + 1] = obtemp[i];
				oblen2 = 9;
			}
			if (obflag == 1) // RESW, RESB 가 입력 될 경우
			{
				if (oblen2 > 0)
				{
					objecttemp[oblen2] = 0;
					sprintf(obtemp, "%02x", (oblen2 - 9) / 2 + 1);
					objecttemp[7] = obtemp[0];
					objecttemp[8] = obtemp[1];
					fprintf(fp1, "%s\n", objecttemp);
				}
				oblen2 = 0;
			}
			if (obflag == 3 && oblen2 + oblen1 < 69 && object[0]) {
				for (int i = 0; i < oblen1; i++) objecttemp[oblen2 + i] = object[i];
				oblen2 = oblen2 + oblen1;
			}

			else if (obflag == 3 && oblen2 + oblen1 >= 69 && object[0]) {
				objecttemp[oblen2] = 0;
				sprintf(obtemp, "%02x", (oblen2 - 9) / 2 + 1);
				objecttemp[7] = obtemp[0];
				objecttemp[8] = obtemp[1];
				fprintf(fp1, "%s\n", objecttemp);
				objecttemp[0] = 'T';
				oblen2 = 0;
				sprintf(obtemp, "%06x", location[count]);
				for (int i = 0; i < 6; i++) objecttemp[i + 1] = obtemp[i];
				for (int i = 0; i < oblen1; i++) objecttemp[i + 9] = object[i];
				oblen2 = 9 + oblen1;
			}
		}
		count++;
	}
	if(oblen2){
		objecttemp[oblen2] = 0;
		sprintf(obtemp, "%02x", (oblen2 - 9) / 2 + 1);
		objecttemp[7] = obtemp[0];
		objecttemp[8] = obtemp[1];
		fprintf(fp1, "%s\n", objecttemp);
		
	}
	if (ModifyHead != NULL) // Modification Record 출력 
	{
		for (ModifyTemp = ModifyHead; ModifyTemp != NULL; ModifyTemp = ModifyTemp->nextModify)
			fprintf(fp1, "M%06x05\n", ModifyTemp->loc);
	}


	fprintf(fp1, "E%06x\n", locstart);
	fclose(fp2);
	fclose(fp1);
}
//해당 레지스터 값을 반환
char regist(char* str) {
	char flag = '0';
	if (strcmp(str, "A") == 0)flag = '0';
	else if (strcmp(str, "X") == 0)flag = '1';
	else if (strcmp(str, "L") == 0)flag = '2';
	else if (strcmp(str, "PC") == 0)flag = '8';
	else if (strcmp(str, "SW") == 0)flag = '9';
	else if (strcmp(str, "B") == 0)flag = '3';
	else if (strcmp(str, "S") == 0)flag = '4';
	else if (strcmp(str, "T") == 0)flag = '5';
	else if (strcmp(str, "F") == 0)flag = '6';
	
	return flag;
}
//Symbol Linked List를 만드는 함수
void makeSymbol(char* asm1, int loc_start) {
	Symbol* TempSymbol;
	TempSymbol = (Symbol*)malloc(sizeof(Symbol));
	TempSymbol->nextSymbol = NULL;
	strcpy(TempSymbol->sym, asm1);
	TempSymbol->loc = loc_start;
	if (!SymHead) {
		SymHead = TempSymbol;
		SymTail = TempSymbol;
	}
	else {
		SymTail->nextSymbol = TempSymbol;
		SymTail = TempSymbol;
	}
}
//Symbol을 내림차순으로 출력하는 함수
void menu_symbol() {
	char SymTemp[5];
	Symbol* SymbolHead, * SymbolTail, * SymbolTemp1, * SymbolTemp2, * SymbolTemp3, * SymbolTemp4;
	SymbolHead = NULL, SymbolTail = NULL;
	for (SymbolTemp1 = SymHead; SymbolTemp1 != NULL; SymbolTemp1 = SymbolTemp1->nextSymbol) {
		SymbolTemp2 = (Symbol*)malloc(sizeof(Symbol));
		SymbolTemp2->nextSymbol = NULL;
		strcpy(SymbolTemp2->sym, SymbolTemp1->sym);
		SymbolTemp2->loc = SymbolTemp1->loc;
		if (!SymbolHead) {
			SymbolHead = SymbolTemp2;
			SymbolTail = SymbolTemp2;
		}
		else {
			if (strcmp(SymbolTail->sym, SymbolTemp2->sym) < 0) {
				SymbolTail->nextSymbol = SymbolTemp2;
				SymbolTail = SymbolTemp2;
			}
			else {
				SymbolTemp4 = NULL;
				for (SymbolTemp3 = SymbolHead; SymbolTemp3 != NULL; SymbolTemp3 = SymbolTemp3->nextSymbol) {
					if (strcmp(SymbolTemp3->sym, SymbolTemp2) > 0)break;
					SymbolTemp4 = SymbolTemp3;
				}
				if (SymbolTemp4 == NULL) {
					SymbolTemp4 = SymbolTemp2;
					SymbolTemp4->nextSymbol = SymbolTemp3;
					SymbolHead = SymbolTemp4;
				}
				else {
					SymbolTemp4->nextSymbol = SymbolTemp2;
					SymbolTemp2->nextSymbol = SymbolTemp3;
				}
			}
		}
	}
	for (int i = 0; i < 5; i++) SymTemp[i] = 0;
	for (SymbolTemp1 = SymbolHead; SymbolTemp1 != NULL; SymbolTemp1 = SymbolTemp1->nextSymbol) {
		sprintf(SymTemp, "%04x", SymbolTemp1->loc);
		printf("\t%s\t%s\n", SymbolTemp1->sym, SymTemp);
	}
}
