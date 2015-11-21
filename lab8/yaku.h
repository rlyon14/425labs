
//kernel size definitions
#define MAXTASK 10
#define MAXSEM 10
#define MAXQUEUES 3
#define MAXEVENT 6

//lab8 specific
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

extern unsigned NewPieceType;
extern unsigned NewPieceColumn;
extern unsigned NewPieceOrientation;
extern unsigned NewPieceID;

struct SMPiece {
	unsigned peiceID:
	unsigned type;
	int rotation;
	int column;
	unsigned score;
	unsigned commands;
};


