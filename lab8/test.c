#include "stdio.h"

unsigned slotOr[10]     = {1,1,0,3,1,2,1,3,0,2};
unsigned slotColumn[10] = {1,4,0,0,5,2,2,3,3,5};
unsigned slotRow[10]    = {0b111000, 0b000111, 0b110000, 0b100000, 0b000011,
			   0b001000, 0b011000, 0b000100, 0b000110, 0b000001};

struct SMpiece 
{
	unsigned pieceID;
	unsigned type;
	unsigned orientation;
	unsigned column;	
};

struct SMcmd 
{
	unsigned pieceID;
	int slide1;
	int rotate;
	int slide2;
};

struct SMpiece pieceArray[10];
struct SMcmd tmpCmd[4];

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

void main (void) {
	int cmd = 0;
	pieceArray[0].column = 4;
	pieceArray[0].orientation = 3;
	cmd = getCmds(&pieceArray[0], 3, 0);
	printf("\n\rSlide1: %d, Rotate: %d, Slide2: %d", tmpCmd[0].slide1, tmpCmd[0].rotate, tmpCmd[0].slide2);
	printf("\n\r%d\n\r", cmd);
	
}
