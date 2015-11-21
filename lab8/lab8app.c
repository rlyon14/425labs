
#include "clib.h"
#include "yakk.h" 
#include "yaku.h"                    
#include "simptris.h"

#define TASK_STACK_SIZE   512  
#define MSGQSIZE 10  
#define MSGARRAYSIZE 10

unsigned board[8] = {0,0,0,0,0,0,0,0};

unsigned cornerRow1[4]= {CNR0ROW1, CNR1ROW1, CNR2ROW1, CNR3ROW1};
unsigned cornerRow0[4]=	{CNR0ROW0, CNR1ROW0, CNR2ROW0, CNR3ROW0};
unsigned strRow2[2]= 	{STR0ROW2, STR1ROW2};
unsigned strRow1[2]= 	{STR0ROW1, STR1ROW1};
unsigned strRow0[2]= 	{STR0ROW0, STR1ROW0};
unsigned UsedSlots[4];
unsigned pieceNext;
unsigned cmdNext;

struct SMPiece pieceArray[MSGARRAYSIZE];
struct SMPiece cmdArray[MSGARRAYSIZE];
void *pieceQ[MSGQSIZE];
void *cmdQ[MSGQSIZE];        
YKQ *pieceQPtr; 
YKQ *cmdQPtr;

YKSEM *SemPtr;

/*a stack for each task */
int SMcmdTaskStk[TASK_STACK_SIZE];
int SMpieceTaskStk[TASK_STACK_SIZE];
int SMStatTaskStk[TASK_STACK_SIZE];

struct SMPiece failSlot;
struct SMPiece tempSlot;

void SMgameOverHdlr(void){
	printString("\n\rGame Over!\n\r");
	exit(0);
}

void SMnewpieceHdlr(void){
	pieceArray[pieceNext].pieceID = NewPieceID;
	pieceArray[pieceNext].type = NewPieceType;
	pieceArray[pieceNext].rotation = NewPieceOrientation;
	pieceArray[pieceNext].column = NewPieceColumn;
	
	if (YKQPost(pieceQPtr, (void *) &(pieceArray[pieceNext])) == 0)
		printString("\n\rPieceQ overflow!\n\r");
	else if (++pieceNext >= MSGARRAYSIZE)
		pieceNext = 0;
}

void SMrecievedCmdHdlr(void){
	YKSemPost(SemPtr); 
}

struct SMPiece fitPiece(unsigned type, int rotation, int column, int depth, unsigned commands) {
	unsigned k;
	unsigned pieceRow2;
	unsigned pieceRow0;
	unsigned pieceRow1;
	unsigned arithBase;
	struct SMPiece minSlot;
	failSlot.type = type;
	failSlot.score = 0xFFFF;
	failSlot.rotation = rotation;
	failSlot.column = column;
	failSlot.commands = 0;
	arithBase = (0x1 << column);
	if (((UsedSlots[rotation]) & arithBase) != 0) { return failSlot; }
	
	if (column < 0) { return failSlot; }
	else if (column > 5) { return failSlot; }
	if (type == 1) {
		if ((rotation < 0) || (rotation > 1)) { return failSlot; }
		if (rotation == 0) {
			if ((column == 0) || (column == 5)) {
				return failSlot;
			}
		}
		pieceRow1 = strRow1[rotation] >> column;
		pieceRow0 = strRow0[rotation] >> column;
	}
	else {
		if (column == 0) {
			if ((rotation == 1)||(rotation == 2)) { return failSlot; }
		}
		if (column == 5) {
			if ((rotation == 0)||(rotation == 3)) { return failSlot; }
		}
		if (rotation > 3) { rotation = 0; }
		else if (rotation < 0) { rotation = 3; }

		pieceRow1 = cornerRow1[rotation] >> column;
		pieceRow0 = cornerRow0[rotation] >> column;
	}	

	//printf("Rotation: %d, Column: %d, Depth: %d\n\r", rotation, column, depth);

	UsedSlots[rotation] = ((UsedSlots[rotation]) | arithBase);
	
	minSlot = failSlot;
	for (k = 0; k <= 6; k++){
		//move up one row and try again
		if ((board[k] & pieceRow0) == pieceRow0) {
			continue;
		}
		//fit
		else if ((board[k] & pieceRow0) == 0) {
			if ((rotation == 2 ) || (rotation == 3)) {
				if ((board[k+1] & pieceRow1) != 0) { break; }
				else if ((board[k] & pieceRow1) == 0) { break; }
			}		
			arithBase = k<<8;
			minSlot.type = type;	
			minSlot.score = (arithBase+depth);
			minSlot.rotation = rotation;
			minSlot.column = column;
			minSlot.commands = commands;
			//printf("Score: %#02x\n\r", minSlot.score);
			if (k == 0) { return minSlot; }
			else { break; }
		}
		//partial cover --return fail
		else { break; }
	}
	
	if (depth < MAXMOVES) {
		arithBase = commands;
		if (depth > 0) {arithBase = arithBase << 4; }
		depth++;
		tempSlot = fitPiece(type, rotation, (column-1), depth, (arithBase+1));
		if (tempSlot.score < minSlot.score) { 
			minSlot = tempSlot;
		}
		tempSlot = fitPiece(type, rotation, (column+1), depth, (arithBase+2));
		if (tempSlot.score < minSlot.score) { 
			minSlot = tempSlot;
		}
		tempSlot = fitPiece(type, (rotation-1), column, depth, (arithBase+3));
		if (tempSlot.score < minSlot.score) { 
			minSlot = tempSlot;
		}
		tempSlot = fitPiece(type, (rotation+1), column, depth, (arithBase+4));
		if (tempSlot.score < minSlot.score) { 
			minSlot = tempSlot;
		}
	}
	return minSlot;	
}

void buildBoard(unsigned piecetype, unsigned rotation, unsigned column){
	unsigned pieceRow0;
	unsigned pieceRow1;
	unsigned pieceRow2;
	int k;
	int j;
	if (piecetype == 0){
		pieceRow2 = 0;
		pieceRow1 = cornerRow1[rotation] >> column;
		pieceRow0 = cornerRow0[rotation] >> column;
	}
	else {
		pieceRow2 = strRow2[rotation] >> column;
		pieceRow1 = strRow1[rotation] >> column;
		pieceRow0 = strRow0[rotation] >> column;
	}
	for (k = 0; k <= 5; k++){
		//full cover--move up one row and try again
		if ((board[k] & pieceRow0) == pieceRow0) { continue; }
		else if ((board[k] & pieceRow0) == 0) {
			board[k] = board[k] | pieceRow0;
			board[k+1] = board[k+1] | pieceRow1;
			board[k+2] = board[k+2] | pieceRow2;
			for (k; k <= 6; k++){
				if (board[k] == 0x3F) {
					j = k;
					for (j= k; j<= 6; j++){
						board[j] = board[j+1];
					}
					board[7] =0;
					k--;
				}
				else { break; }
			}
		return;	
		}
	}
}

/* pulls pieces from SMpieceQueue and puts commands in SMcmdQueue*/
void SMpieceTask(void) {
	struct SMPiece* ptemp;
	struct SMPiece  cmdtemp;
	int j;
	SeedSimptris(SIMPSEED);
	StartSimptris();
    	while(1) {
		for (j = 0; j <=3; j++){
			UsedSlots[j] = 0;
		}
		ptemp = (struct SMPiece*) YKQPend(pieceQPtr);
		
		YKEnterMutex();	
		cmdtemp = fitPiece(ptemp->type, ptemp->rotation, ptemp->column, 0, 0);
		buildBoard(ptemp->type, cmdtemp.rotation, cmdtemp.column);
		cmdArray[cmdNext].pieceID = ptemp->pieceID;
		cmdArray[cmdNext].commands = cmdtmp.commands;
		
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
	unsigned moves;
	unsigned pieceID;
	int k;
   	while(1) {
		ctemp = (struct SMcmd*) YKQPend(cmdQPtr);
		moves = ctemp->commands;
		pieceID = ctemp->pieceID;
		for (k = 0; k <= MAXMOVES; k++){
			if (k > 0) { moves = moves >> 4; }
			moves = moves & 0xF;
			if (moves == 0) { break; }
			YKSemPend(SemPtr);
			switch (moves) {
			case 1:
				SlidePiece(pieceID, 0);
				break;
			case 2: 
				SlidePiece(pieceID, 1);
				break;
			case 3: 
				RotatePiece(pieceID, 1);
				break;
			default:
				RotatePiece(pieceID, 0);
				break;
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
