
#include "stdio.h"                 
#include "yaku.h"
#include "simdefs.h"

#define MSGQSIZE 10  
#define MSGARRAYSIZE 10

unsigned slotOr[10]     = {0,0,0,3,1,2,1,3,0,2};
unsigned slotColumn[10] = {1,4,0,0,5,2,2,3,3,5};
unsigned slotLrow[10]= 	{SLOTL0, SLOTL1, SLOTL2, SLOTL3, SLOTL4, SLOTL5, SLOTL6, SLOTL7, SLOTL8, SLOTL9};
unsigned slotUrow[10]=	{SLOTU0 ,SLOTU1, SLOTU2, SLOTU3, SLOTU4, SLOTU5, SLOTU6, SLOTU7, SLOTU8, SLOTU9};

unsigned rightBlock;
unsigned leftBlock;
unsigned upperRow;
unsigned lowerRow;
unsigned pieceNext;
unsigned cmdNext;

struct SMcmd tmpCmd;


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
int SMpieceTask(struct SMpiece* ptemp) {
	unsigned destSlot;
	int minSlot;
	int minCmdNum;
	int tCmdNum;
	int tSlot;
	int k;
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
				printf("\n\rDefault hit (straight piece)\n\r"); 
			}		

			getCmds(ptemp, destSlot);

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
			else { printf("\n\rDefault hit (corner piece)\n\r");}
			
			getCmds(ptemp, destSlot);

			lowerRow = lowerRow | slotLrow[destSlot];
			upperRow = upperRow | slotUrow[destSlot];
		}
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

		return destSlot;
}

void main(void)
{
	int slot;
	struct SMpiece piece;
	rightBlock = 0;
	leftBlock = 0;
	upperRow = 0b000000;
	lowerRow = 0b000111;
	piece.pieceID = 1;
	piece.column = 5;
	piece.type = 1;
	piece.orientation = 0;
	slot = SMpieceTask(&piece);
	printf("Slot: %d\n\r", slot);
	printf("Rows:\n\r");
	printf("%#02x\n\r", upperRow);
	printf("%#02x\n\r", lowerRow);
	printf("LeftBlock: %d, RightBlock: %d\n\r", leftBlock, rightBlock);
}
