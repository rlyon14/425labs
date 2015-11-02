
#include "clib.h"
#include "yakk.h" 
#include "yaku.h"                    
#include "simptris.h"

#define TASK_STACK_SIZE   512  
#define MSGQSIZE 10  
#define MSGARRAYSIZE 10

int SMreceivedFlag;  
unsigned slotOr[10]     = {1,1,0,3,1,2,1,3,0,2};
unsigned slotColumn[10] = {1,4,0,0,5,2,2,3,3,5};
unsigned slotRow[10]    = {0b111000, 0b000111, 0b110000, 0b100000, 0b000011,
			0b001000, 0b011000, 0b000100, 0b000110, 0b000001};

struct SMpiece pieceArray[MSGARRAYSIZE];
struct SMcmd cmdArray[MSGARRAYSIZE];
void *pieceQ[MSGQSIZE];
void *cmdQ[MSGQSIZE];        
YKQ *pieceQPtr; 
YKQ *cmdQPtr;

YKSEM *SemPtr;

/*a stack for each task */
int SMcdmTaskStk[TASK_STACK_SIZE];
int SMpieceTaskStk[TASK_STACK_SIZE];
int SMStatTaskStk[TASK_STACK_SIZE];

unsigned upperBlock;
unsigned curRow;
unsigned pieceNext;
unsigned cmdNext;

struct SMcmd tmpCmd[4];

void SMgameOverHdlr(void){
	printString("\n\rGameOver\n\r");
}

void SMnewpieceHdlr(void){
	pieceArray[pieceNext].pieceID = NewPieceID;
	pieceArray[pieceNext].type = NewPieceType;
	pieceArray[pieceNext].orientation = NewPieceOrientation;
	pieceArray[pieceNext].column = NewPieceColumn;
	
	if (YKQPost(pieceQPtr, (void *) &(pieceArray[pieceNext])) == 0)
		printString("\n\rPieceQ overflow!\n\r");
	else if (++pieceNext >= MSGARRAYSIZE)
		piecenext = 0;
}

void SMrecievedCmdHdlr(void){
	YKSemPost(SemPtr); 
}

int getCmds(struct SMpiece* piece, unsigned slotNum, int tempIndex){
	unsigned cmdNum;
	int column;
	int rotation;
	rotation = piece->orientation;
	column = piece->column;
	cmdNum = 0;
	tmpCmd[tempIndex].slide1 = 0;
	tmpCmd[tempIndex].slide2 = 0;
	
	while (rotation != slotOr[slotNum]){
		cmdNum++;	
		if (rotation == 0) { rotation = 3;}
		else {rotation--;}
	}
	if (cmdNum > 2){
		cmdNum = 1;
		tmpCmd[tempIndex].rotate = -1;
	}
	else {
		tmpCmd[tempIndex].rotate = cmdNum;
	}
	
	tmpCmd[tempIndex].slide2 = 0;
	tmpCmd[tempIndex].slide1 = (slotColumn[slotNum])-column;
	cmdNum = cmdNum + (-1*(tmpCmd[tempIndex].slide1));
	
	if (tmpCmd[tempIndex].rotate != 0){
		if (slotColumn[slotNum] == 0) { 
			if (column == 0) {
				tmpCmd[tempIndex].slide1 = 1;
				tmpCmd[tempIndex].slide2 = -1;
				cmdNum = cmdNum +2;
			}
			else if (column == 5) {
				tmpCmd[tempIndex].slide1 = -1;
				tmpCmd[tempIndex].slide2 = -4;	
			}
		}
		if (slotColumn[slotNum] == 5) {
			if (column == 0) {
				tmpCmd[tempIndex].slide1 = 1;
				tmpCmd[tempIndex].slide2 = 4;
			}
			else if (column == 5) {
				tmpCmd[tempIndex].slide1 = -1;
				tmpCmd[tempIndex].slide2 = 1;
				cmdNum = cmdNum +2;	
			}
		}
	}
	return cmdNum;	
} 

/* pulls pieces from SMpieceQueue and puts commands in SMcmdQueue*/
void SMpieceTask(void)
	struct SMpiece* ptemp;
	unsigned destSlot;
    while(1) {
		ptemp = (struct msg *) YKQPend(pieceQPtr);
		pColumn = ptemp->column;
		pOr = ptemp->orientation; 
		cmdArray[cmdNext].pieceID = ptemp->pieceID;
		/*straight piece*/
		if (ptemp->pieceID == 1)  { 
			if (curRow == 0) {
				//destSlot = 10;
				
			}
			else if ((curRow & 0b111000) == 0) {destSlot = 0;}
			else if ((curRow & 0b000111) == 0) {destSlot = 1;}
			else if ((curRow & 0b111000) == 0b111000) {destSlot = 0;}
			else if ((curRow & 0b000111) == 0b000111) {destSlot = 1;}		 	 	
		}
		/*corner piece*/
		else { 
			if (curRow == 0) {destSlot = 11;}
			else if (curRow == 0b111000) {destSlot = 13;}
			else if (curRow == 0b000111) {destSlot = 12;}
			else if ((curRow & 0b111000) == 0b110000) {destSlot = 5;}
			else if ((curRow & 0b000111) == 0b000011) {destSlot = 7;}
			else if ((curRow & 0b111000) == 0b011000) {destSlot = 3;}
			else if ((curRow & 0b000111) == 0b000110) {destSlot = 9;}
		}
		


		cmdArray[cmdNext].pieceID = 
		cmdArray[cmdNext].slide1 = 
		cmdArray[cmdNext].rotate =
		cmdArray[cmdNext].slide2 =
		if (YKQPost(cmdQPtr, (void *) &(cmdArray[cmdNext])) == 0)
			printString("\n\rcmdQ overflow!\n\r");
		else if (++cmdNext >= MSGARRAYSIZE)
			cmdnext = 0;
    }
}

/* sends commands to simptris from SMcmdQueue*/
void SMcmdTask(void)    
{
	struct SMcmd* ctemp;
	int k;
    while(1) {
		ctemp = (struct msg *) YKQPend(cmdQPtr); 
		for (k = 0; k < numCmds; k++){
			YKSemPend(SemPtr);
	       	sendcmd();			
		}
    }
}


void SMStatTask(void)           /* tracks statistics */
{
    unsigned max, switchCount, idleCount;
    int tmp;

    YKDelayTask(1);
    printString("Starting Simtris\r\n");
    printString("Determining CPU capacity\r\n");
    YKDelayTask(1);
    YKIdleCount = 0;
    YKDelayTask(5);
    max = YKIdleCount / 25;
    YKIdleCount = 0;

    YKNewTask(SMcmdTask, (void *) &SMcmdTaskStk[TASK_STACK_SIZE], 1);
    YKNewTask(SMpieceTask, (void *) &SMpieceTaskStk[TASK_STACK_SIZE], 2);
    
    while (1)
    {
        YKDelayTask(20);
        
        YKEnterMutex();
        switchCount = YKCtxSwCount;
        idleCount = YKIdleCount;
        YKExitMutex();
        
        printString("<CS: ");
        printInt((int)switchCount);
        printString(", CPU: ");
        tmp = (int) (idleCount/max);
        printInt(100-tmp);
        printString(">\r\n");
        
        YKEnterMutex();
        YKCtxSwCount = 0;
        YKIdleCount = 0;
        YKExitMutex();
    }
}  

void main(void)
{
	YKInitialize();
	curRow = 0;
	upperBlock = 0;
	pieceNext = 0;
	cmdNext = 0;
	leftDown = 0;
	rightDown = 0;
	pieceQPtr = YKQCreate(pieceQ, MSGQSIZE);
	cmdQPtr = YKQCreate(cmdQ, MSGQSIZE);
	SemPtr = YKSemCreate(1, "PSem");
    YKNewTask(SMStatTask, (void *) &SMStatTaskStk[TASK_STACK_SIZE], 0);
    
    YKRun();
}
