
#define MAXTASK 10
#define MAXSEM 10
#define MAXQUEUES 3
#define MAXEVENT 6

extern unsigned NewPieceType;
extern unsigned NewPieceColumn;
extern unsigned NewPieceOrientation;
extern unsigned NewPieceID;

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

