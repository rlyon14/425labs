
#include "stdio.h"

#define CNR0ROW1 0x20
#define CNR0ROW0 0x30
#define CNR1ROW1 0x20
#define CNR1ROW0 0x60
#define CNR2ROW1 0x60
#define CNR2ROW0 0x20
#define CNR3ROW1 0x30
#define CNR3ROW0 0x20
#define STR0ROW0 0x38
#define STR1ROW0 0x20

struct PieceSlot{
	unsigned score;
	int rotate;
	int column;
	int commands[3];
};

struct PieceCmd{
	

};

int PieceMoves[3] = {0,0,0};

unsigned board[8] = {0,0,0,0,0,0,0,0};

unsigned cornerRow1[4]= {CNR0ROW1, CNR1ROW1, CNR2ROW1, CNR3ROW1};
unsigned cornerRow0[4]=	{CNR0ROW0, CNR1ROW0, CNR2ROW0, CNR3ROW0};
unsigned strRow0[2]= 	{STR0ROW0, STR1ROW0};

struct PieceSlot failSlot;
struct PieceSlot tempSlot;

struct PieceSlot dropCnrPiece(int rotate, int column, int depth) {
	int k;
	unsigned pieceRow0;
	unsigned pieceRow1;
	struct PieceSlot minSlot;

	failSlot.score = 0xFF;
	failSlot.rotate = 0;
	failSlot.column = 0;

	if (depth > 2) { return failSlot; }
	if (column < 0) { return failSlot; }
	else if (column > 5) { return failSlot; }
	if (column == 0) {
		if ((rotate == 1)||(rotate == 2)) { return failSlot; }
	}
	if (column == 5) {
		if ((rotate == 0)||(rotate == 3)) { return failSlot; }
	}
	if (rotate > 3) { rotate = 0; }
	else if (rotate < 0) { rotate = 3; }
	
	pieceRow1 = cornerRow1[rotate] >> column;
	pieceRow0 = cornerRow0[rotate] >> column;
	minSlot = failSlot;
	for (k = 0; k <= 6; k++){
		//full cover--move up one row and try again
		if ((board[k] & pieceRow0) == pieceRow0) {
			continue;
		}
		//fit --return k
		else if ((board[k] & pieceRow0) == 0) {
			if ((rotate == 2 )|| (rotate == 3)) {
				if ((board[k+1] & pieceRow1) != 0) { break; }
				else if ((board[k] & pieceRow1) == 0) { break; }
			}	
			tempSlot.score = (k<<4)+depth; //?
			tempSlot.rotate = rotate;
			tempSlot.column = column;
			if (k == 0) { return tempSlot; }
			else { 
				minSlot = tempSlot; 
				break;
			}
		}
		//partial cover --return fail
		else { break; }
	}
	
	if (depth < 3) {
		tempSlot = dropCnrPiece(rotate, column-1, depth+1);
		if (tempSlot.score < minSlot.score) { 
			minSlot = tempSlot;
		}
		tempSlot = dropCnrPiece(rotate, column+1, depth+1);
		if (tempSlot.score < minSlot.score) { 
			minSlot = tempSlot;
		}
		tempSlot = dropCnrPiece(rotate-1, column, depth+1);
		if (tempSlot.score < minSlot.score) { 
			minSlot = tempSlot;
		}
		tempSlot = dropCnrPiece(rotate+1, column, depth+1);
		if (tempSlot.score < minSlot.score) { 
			minSlot = tempSlot;
		}
	}
	return minSlot;	
}

void buildBoard(rotate, column){
	unsigned pieceRow0;
	unsigned pieceRow1;
	int k;
	pieceRow1 = cornerRow1[rotate] >> column;
	pieceRow0 = cornerRow0[rotate] >> column;
	for (k = 0; k <= 6; k++){
		//full cover--move up one row and try again
		if ((board[k] & pieceRow0) == pieceRow0) { continue; }
		else if ((board[k] & pieceRow0) == 0) {
			board[k] = board[k] | pieceRow0;
			board[k+1] = board[k+1] | pieceRow1;
			for (k; k <= 6; k++){
				if (board[k] == 0x3F) {
					for (k; k<= 6; k++){
						board[k] = board[k+1];
					}
					board[8] =0;
					k--;
				}
			}
		return;	
		}
	}
}

void main (void) {
	struct PieceSlot slot;
	
	slot = dropCnrPiece(0, 2, 0);
	printf("Score: %#02x, Rotate: %d, Column: %d\n\r", slot.score, slot.rotate, slot.column);
	buildBoard(slot.rotate, slot.column);
	printf("Board1: %#02x\n\rBoard0: %#02x\n\r", board[1], board[0]);
}







