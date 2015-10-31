
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

/*a stack for each task */
int SMcdmTaskStk[TASK_STACK_SIZE];
int SMpieceTaskStk[TASK_STACK_SIZE];
int SMStatTaskStk[TASK_STACK_SIZE];

unsigned SMupperRow;
unsigned SMlowerRow;
unsigned newPieceNext;

void SMgameOverHdlr(void){
	printString("\n\rGameOver\n\r");
}

void SMnewpieceHdlr(void){
	pieceArray[newPieceNext].pieceID = NewPieceID;
	pieceArray[newPieceNext].type = NewPieceType;
	pieceArray[newPieceNext].orientation = NewPieceOrientation;
	pieceArray[newPieceNext].column = NewPieceColumn;
	
	if (YKQPost(pieceQPtr, (void *) &(pieceArray[next])) == 0)
		printString("\n\rnewPieceQ overflow!\n\r");
	else if (++next >= MSGARRAYSIZE)
		next = 0;
}

/* pulls pieces from SMpieceQueue and puts commands in SMcmdQueue*/
void SMpieceTask(void)
	//tmp = (struct msg *) YKQPend(MsgQPtr); /* get next msg */
    while(1) {

    }
}

/* sends commands to simptris from SMcmdQueue*/
void SMcmdTask(void)    
{

    while(1) {

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
	newPieceNext = 0;
	SMreceivedFlag = 1;
	pieceQPtr = YKQCreate(pieceQ, MSGQSIZE);
	cmdQPtr = YKQCreate(cmdQ, MSGQSIZE);
    YKNewTask(SMStatTask, (void *) &SMStatTaskStk[TASK_STACK_SIZE], 0);
    
    YKRun();
}
