
#include "clib.h"
#include "yakk.h" 
#include "yaku.h"                    
#include "simptris.h"

#define TASK_STACK_SIZE   512  
#define MSGQSIZE 10  
#define MSGARRAYSIZE 10  

int SMreceivedFlag;  

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

unsigned SMupperRowLeft;
unsigned SMlowerRowLeft;
unsigned SMupperRowRight;
unsigned SMlowerRowRight;
unsigned pieceNext;
unsigned cmdNext;

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

/* pulls pieces from SMpieceQueue and puts commands in SMcmdQueue*/
void SMpieceTask(void)
	struct SMpiece* ptemp;
    while(1) {
		ptemp = (struct msg *) YKQPend(pieceQPtr); 
		


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
    while(1) {
		ctemp = (struct msg *) YKQPend(cmdQPtr); 
		for (# of commands in ctemp){
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
	SMupperRow = 0;
	SMlowerRow = 0;
	pieceNext = 0;
	cmdNext = 0;
	SMreceivedFlag = 1;
	pieceQPtr = YKQCreate(pieceQ, MSGQSIZE);
	cmdQPtr = YKQCreate(cmdQ, MSGQSIZE);
	SemPtr = YKSemCreate(1, "PSem");
    YKNewTask(SMStatTask, (void *) &SMStatTaskStk[TASK_STACK_SIZE], 0);
    
    YKRun();
}
