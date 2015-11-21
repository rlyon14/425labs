

#include "stdio.h"
#define CNR0ROW1 0x20
#define CNR0ROW0 0x30
#define CNR1ROW1 0x20
#define CNR1ROW0 0x60
#define CNR2ROW1 0x60
#define CNR2ROW0 0x20
#define CNR3ROW1 0x30
#define CNR3ROW0 0x20
#define STR0ROW2 0x0
#define STR0ROW1 0x0
#define STR0ROW0 0x70
#define STR1ROW2 0x20
#define STR1ROW1 0x20
#define STR1ROW0 0x20

#define MAXMOVES 2

struct SMPiece {
	unsigned type;
	int rotation;
	int column;
	unsigned score;
	unsigned commands;
};

unsigned board[8] = {0,0,0,0,0,0,0,0};

unsigned cornerRow1[4]= {CNR0ROW1, CNR1ROW1, CNR2ROW1, CNR3ROW1};
unsigned cornerRow0[4]=	{CNR0ROW0, CNR1ROW0, CNR2ROW0, CNR3ROW0};
unsigned strRow2[2]= 	{STR0ROW2, STR1ROW2};
unsigned strRow1[2]= 	{STR0ROW1, STR1ROW1};
unsigned strRow0[2]= 	{STR0ROW0, STR1ROW0};
unsigned UsedSlots[4];

struct SMPiece failSlot;
struct SMPiece tempSlot;

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

void printBoard(void){
	int i; 
	unsigned h;
	for (i = 7; i >= 0; i--){
		if (board[i] != 0){
			printf("%d ", i);
			for (h = 0; h <= 5; h++){
				if ((board[i] & (0x020 >> h)) != 0) {
					printf("#");
				}
				else {
					printf("_");
				}
			}
			printf("\n\r");
		}
	}

}

//0 is corner, 1 is straight
void playPiece(unsigned type, int rotation, int column){
	struct SMPiece slot;
	int j;
	for (j = 0; j <=3; j++){
		UsedSlots[j] = 0;
	}

	slot = fitPiece(type, rotation, column, 0, 0);

	printf("Score: %#02x, rotation: %d, Column: %d\n\r", slot.score, slot.rotation, slot.column);
	printf("Commands: %#02x\n\r", slot.commands);
	buildBoard(type, slot.rotation, slot.column);
	printBoard();
}

//0 is corner, 1 is straight
void main (void) {
	playPiece(1,1,1);
	playPiece(1,1,1);
	playPiece(1,1,1);
	playPiece(1,1,1);
	playPiece(1,1,2);
	playPiece(1,1,2);
	playPiece(1,1,3);
	
}







