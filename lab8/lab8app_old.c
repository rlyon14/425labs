
#include "clib.h"
#include "yakk.h" 
#include "yaku.h"                    
#include "simptris.h"
#include "simdefs.h"

#define TASK_STACK_SIZE   512  
#define MSGQSIZE 10  
#define MSGARRAYSIZE 10

unsigned cornerRow1[4]= {CNR0ROW1, CNR1ROW1, CNR2ROW1, CNR3ROW1}
unsigned cornerRow0[4]=	{CNR0ROW0, CNR1ROW0, CNR2ROW0, CNR3ROW0}
unsigned strRow0[2]= {STR0ROW0, STR1ROW0}

struct SMpiece pieceArray[MSGARRAYSIZE];
struct SMcmd cmdArray[MSGARRAYSIZE];
void *pieceQ[MSGQSIZE];
void *cmdQ[MSGQSIZE];        
YKQ *pieceQPtr; 
YKQ *cmdQPtr;

YKSEM *SemPtr;

/*a stack for each task */
int SMcmdTaskStk[TASK_STACK_SIZE];
int SMpieceTaskStk[TASK_STACK_SIZE];
int SMStatTaskStk[TASK_STACK_SIZE];

unsigned rightBlock;
unsigned leftBlock;
unsigned upperRow;
unsigned lowerRow;
unsigned pieceNext;
unsigned cmdNext;

struct SMcmd tmpCmd;

void SMgameOverHdlr(void){
	printString("\n\rGame Over!\n\r");
	exit(0);
}

void SMnewpieceHdlr(void){
	pieceArray[pieceNext].pieceID = NewPieceID;
	pieceArray[pieceNext].type = NewPieceType;
	pieceArray[pieceNext].orientation = NewPieceOrientation;
	pieceArray[pieceNext].column = NewPieceColumn;
	
	if (YKQPost(pieceQPtr, (void *) &(pieceArray[pieceNext])) == 0)
		printString("\n\rPieceQ overflow!\n\r");
	else if (++pieceNext >= MSGARRAYSIZE)
		pieceNext = 0;
}

void SMrecievedCmdHdlr(void){
	YKSemPost(SemPtr); 
}

//writes commands to global variable tmpCmd
int getCmds(struct SMpiece* piece, unsigned slotNum){
	unsigned cmdNum;
	int column;
	int rotation;
	rotation = piece->orientation;
	column = piece->column;
	cmdNum = 0;
	tmpCmd.slide1 = 0;
	tmpCmd.slide2 = 0;
	
	while (rotation != slotOr[slotNum]){
		cmdNum++;	
		if (rotation == 0) { rotation = 3;}
		else {rotation--;}
	}
	if (cmdNum > 2){
		cmdNum = 1;
		tmpCmd.rotate = -1;
	}
	else {
		tmpCmd.rotate = cmdNum;
	}
	
	tmpCmd.slide2 = 0;
	tmpCmd.slide1 = (slotColumn[slotNum])-column;
	if (tmpCmd.slide1 < 0){
		cmdNum = cmdNum + (-1*(tmpCmd.slide1));
	}
	else {
		cmdNum = cmdNum + tmpCmd.slide1;
	}
	
	if (tmpCmd.rotate != 0){
		if (slotColumn[slotNum] == 0) { 
			if (column == 0) {
				tmpCmd.slide1 = 1;
				tmpCmd.slide2 = -1;
				cmdNum = cmdNum +2;
			}
			else if (column == 5) {
				tmpCmd.slide1 = -1;
				tmpCmd.slide2 = -4;	
			}
			else {
				//forgot this else block
				tmpCmd.slide2 = tmpCmd.slide1;
				tmpCmd.slide1 = 0;
			}
		}
		if (slotColumn[slotNum] == 5) {
			if (column == 0) {
				tmpCmd.slide1 = 1;
				tmpCmd.slide2 = 4;
			}
			else if (column == 5) {
				tmpCmd.slide1 = -1;
				tmpCmd.slide2 = 1;
				cmdNum = cmdNum +2;	
			}
			else {
				//forgot this else block
				tmpCmd.slide2 = tmpCmd.slide1;
				tmpCmd.slide1 = 0;
			}
		}
	}
	return cmdNum;	
} 

/* pulls pieces from SMpieceQueue and puts commands in SMcmdQueue*/
void SMpieceTask(void) {
	struct SMpiece* ptemp;
	unsigned destSlot;
	int minSlot;
	int minCmdNum;
	int tCmdNum;
	int tSlot;
	int k;
	SeedSimptris(SIMPSEED);
	StartSimptris();
    	while(1) {
		ptemp = (struct SMpiece*) YKQPend(pieceQPtr);

		/*straight piece*/
		if (ptemp->type == 1)  { 
			if (lowerRow == 0) {
				tCmdNum = getCmds(ptemp, 0);
				if (getCmds(ptemp, 1) < tCmdNum){
					destSlot = 1;
				}
				else {
					destSlot = 0;
				}
			}
			else if ((lowerRow & LEFTMASK) == 0) {destSlot = 0;}
			else if ((lowerRow & RIGHTMASK) == 0) {destSlot = 1;}
			else if ((lowerRow & LEFTMASK) == 0x38) {destSlot = 0;}
			else if ((lowerRow & RIGHTMASK) == 0x07) {destSlot = 1;}
			else { 
				printString("\n\rDefault hit (straight piece)\n\r"); 
			}		

			tCmdNum = getCmds(ptemp, destSlot);

			//drop peice onto rows
			if ((upperRow | slotLrow[destSlot]) == upperRow) {
				if (destSlot == 0) { leftBlock++; }
				else { rightBlock++; }			
			}
			else if ((lowerRow | slotLrow[destSlot]) == lowerRow) {
				upperRow = upperRow | slotLrow[destSlot];
			}
			else {
				lowerRow = lowerRow | slotLrow[destSlot];
			}			
		}

		/*corner piece*/
		else { 
			if (lowerRow == 0) {
				minCmdNum = 20;
				k = 2;
				while (k < 9){
					tCmdNum = getCmds(ptemp, k);
					if (tCmdNum < minCmdNum){
						minCmdNum = tCmdNum;
						minSlot = k;
					}
					k = k+2;
				}
				destSlot = minSlot;
			}
			else if (lowerRow == 0x38) {
				tCmdNum = getCmds(ptemp, 4);
				if (getCmds(ptemp, 8) < tCmdNum){
					destSlot = 8;
				}
				else {
					destSlot = 4;
				}				
			}
			else if (lowerRow == 0x07) {
				tCmdNum = getCmds(ptemp, 2);
				if (getCmds(ptemp, 6) < tCmdNum){
					destSlot = 6;
				}
				else {
					destSlot = 2;
				}				
			}
			else if ((lowerRow & LEFTMASK)  == 0x30) {destSlot = 5;}
			else if ((lowerRow & RIGHTMASK) == 0x03) {destSlot = 7;}
			else if ((lowerRow & LEFTMASK)  == 0x18) {destSlot = 3;}
			else if ((lowerRow & RIGHTMASK) == 0x06) {destSlot = 9;}
			else { printString("\n\rDefault hit (corner piece)\n\r");}
			
			tCmdNum = getCmds(ptemp, destSlot);

			lowerRow = lowerRow | slotLrow[destSlot];
			upperRow = upperRow | slotUrow[destSlot];
		}
		//clear up to two rows
		for (k = 0; k < 2; k ++) {
			if (lowerRow == 0x3F) {
				lowerRow = upperRow;
				upperRow = 0;
				if (rightBlock > 0) { 
					rightBlock--; 
					upperRow = 0x07;
				}
				else if (leftBlock > 0)  { 
					leftBlock--;
					upperRow = 0x38;
				}
			}
		}		
		YKEnterMutex();

		/*
		printNewLine();
		printString("Slot: ");
		printInt(destSlot);
		printNewLine();
		printWord(upperRow);
		printNewLine();
		printWord(lowerRow);
		printNewLine();
		printString("LeftBlock: ");
		printInt(leftBlock);
		printNewLine();
		printString("RightBlock: ");
		printInt(rightBlock);
		printNewLine();
		*/
		
		cmdArray[cmdNext].pieceID = ptemp->pieceID;
		cmdArray[cmdNext].slide1 = tmpCmd.slide1;
		cmdArray[cmdNext].rotate = tmpCmd.rotate;
		cmdArray[cmdNext].slide2 = tmpCmd.slide2;
		if (YKQPost(cmdQPtr, (void *) &(cmdArray[cmdNext])) == 0)
			printString("\n\rcmdQ overflow!\n\r");
		else if (++cmdNext >= MSGARRAYSIZE)
			cmdNext = 0;
		YKExitMutex();
    }
}

/* sends commands to simptris from SMcmdQueue*/
void SMcmdTask(void) {   
	struct SMcmd* ctemp;
	int k;
	int slide;
	int direction;
	int rotate;
   	while(1) {
		ctemp = (struct SMcmd*) YKQPend(cmdQPtr);
		if (ctemp->slide1 != 0){
			slide = ctemp->slide1;
			if (slide > 0) {direction = 1;}
			else {
				direction = 0;
				slide = -1*slide;
			}
			for (k = 0; k < slide; k++){
				YKSemPend(SemPtr);
				SlidePiece(ctemp->pieceID, direction);			
			}
		}
		if (ctemp->rotate != 0){
			rotate = ctemp->rotate;
			if (rotate > 0) {direction = 1;}
			else {
				direction = 0;
				rotate = -1*rotate;
			}
			for (k = 0; k < rotate; k++){
				YKSemPend(SemPtr);
			   	RotatePiece(ctemp->pieceID, direction);			
			}
		}
		if (ctemp->slide2 != 0){
			slide = ctemp->slide2;
			if (slide > 0) {direction = 1;}
			else {
				direction = 0;
				slide = -1*slide;
			}
			for (k = 0; k < slide; k++){
				YKSemPend(SemPtr);
			   	SlidePiece(ctemp->pieceID, direction);			
			}
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
        
        printString("<CS: ");
        printInt((int)switchCount);
        printString(", CPU: ");
        tmp = (int) (idleCount/max);
        printInt(100-tmp);
        printString(">\r\n");
        
        YKCtxSwCount = 0;
        YKIdleCount = 0;
        YKExitMutex();
    }
}  

void main(void)
{
	YKInitialize();
	pieceNext = 0;
	cmdNext = 0;
	rightBlock = 0;
	leftBlock = 0;
	upperRow = 0;
	lowerRow = 0;
	pieceQPtr = YKQCreate(pieceQ, MSGQSIZE);
	cmdQPtr = YKQCreate(cmdQ, MSGQSIZE);
	SemPtr = YKSemCreate(1, "PSem");
	YKNewTask(SMStatTask, (void *) &SMStatTaskStk[TASK_STACK_SIZE], 0);
	YKRun();
}