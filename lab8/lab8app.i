# 1 "lab8app.c"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "lab8app.c"

# 1 "clib.h" 1



void print(char *string, int length);
void printNewLine(void);
void printChar(char c);
void printString(char *string);


void printInt(int val);
void printLong(long val);
void printUInt(unsigned val);
void printULong(unsigned long val);


void printByte(char val);
void printWord(int val);
void printDWord(long val);


void exit(unsigned char code);


void signalEOI(void);
# 3 "lab8app.c" 2
# 1 "yakk.h" 1




struct Task {
 int *taskSP;
 unsigned char taskPriority;
 unsigned int taskDelay;
 struct Task* next;
 struct Task* prev;
 int eventWaitMode;
 unsigned eventMask;
};

typedef struct {
 unsigned value;
 struct Task* pendHead;
 struct Task* pendTail;
} YKEVENT;

typedef struct {
 int value;
 struct Task* pendHead;
 char *string;
} YKSEM;

typedef struct {
 int *nextEmpty;
 int *oldest;
 int *qEnd;
 int *qStart;
 int qSize;
 int qCount;
 struct Task* qBlockedHead;
} YKQ;

extern unsigned int YKTickCount;
extern unsigned int YKCtxSwCount;
extern unsigned int YKIdleCount;

YKEVENT *YKEventCreate(unsigned initialValue);

unsigned YKEventPend(YKEVENT *event, unsigned eventMask, int waitMode);

void YKEventSet(YKEVENT *event, unsigned eventMask);

void YKEventReset(YKEVENT *event, unsigned eventMask);

void printYKQ(YKQ *queue);

void printMsgQueue(YKQ *queue);

int YKQPost(YKQ *queue, void *msg);

void *YKQPend(YKQ *queue);

YKQ *YKQCreate(void **start, unsigned size);

void YKsaveSP(void);

void YKDispatcher(int saveContext);

void YKEnterMutex(void);

void YKExitMutex(void);

void YKIdleTask(void);

YKSEM* YKSemCreate(int initialValue, char *string);

void YKInitialize(void);

void YKNewTask(void (*task)(void), int *taskStack, unsigned char priority);

void YKRun(void);

void YKDelayTask(unsigned count);

void YKExitISR(void);

void YKEnterISR(void);

void YKScheduler(int saveContext);

void YKSemPend(YKSEM *semaphore);

void YKSemPost(YKSEM *semaphore);

void YKTickHandler(void);

void YKkeypress(void);

void YKinsertSorted(struct Task* item, struct Task** listHead);

struct Task* YKpopSorted (struct Task** listHead);

void YKremoveUnsorted (struct Task* item, struct Task** listHead, struct Task** listTail);

void YKinsertUnsorted(struct Task* item, struct Task** listHead, struct Task** listTail);
# 4 "lab8app.c" 2
# 1 "yaku.h" 1
# 9 "yaku.h"
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
# 5 "lab8app.c" 2
# 1 "simptris.h" 1


void SlidePiece(int ID, int direction);
void RotatePiece(int ID, int direction);
void SeedSimptris(long seed);
void StartSimptris(void);
# 6 "lab8app.c" 2
# 1 "simdefs.h" 1
# 7 "lab8app.c" 2





unsigned slotOr[10] = {0,0,0,3,1,2,1,3,0,2};
unsigned slotColumn[10] = {1,4,0,0,5,2,2,3,3,5};
unsigned slotLrow[10]= {0x38, 0x07, 0x30, 0x20, 0x03, 0x08, 0x18, 0x04, 0x06, 0x01};
unsigned slotUrow[10]= {0x00 ,0x00, 0x20, 0x30, 0x01, 0x18, 0x08, 0x06, 0x04, 0x03};

struct SMpiece pieceArray[10];
struct SMcmd cmdArray[10];
void *pieceQ[10];
void *cmdQ[10];
YKQ *pieceQPtr;
YKQ *cmdQPtr;

YKSEM *SemPtr;


int SMcmdTaskStk[512];
int SMpieceTaskStk[512];
int SMStatTaskStk[512];

unsigned rightBlock;
unsigned leftBlock;
unsigned upperRow;
unsigned lowerRow;
unsigned pieceNext;
unsigned cmdNext;
int score;

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
 else if (++pieceNext >= 10)
  pieceNext = 0;
}

void SMrecievedCmdHdlr(void){
 YKSemPost(SemPtr);
}


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

    tmpCmd.slide2 = tmpCmd.slide1;
    tmpCmd.slide1 = 0;
   }
  }
 }

 return cmdNum;
}


void SMpieceTask(void) {
 struct SMpiece* ptemp;
 unsigned destSlot;
 int minSlot;
 int minCmdNum;
 int tCmdNum;
 int tSlot;
 int k;
 SeedSimptris(10947);
 StartSimptris();
     while(1) {
  ptemp = (struct SMpiece*) YKQPend(pieceQPtr);


  if (ptemp->type == 1) {
   if (lowerRow == 0) {
    tCmdNum = getCmds(ptemp, 0);
    if (getCmds(ptemp, 1) < tCmdNum){
     destSlot = 1;
    }
    else {
     destSlot = 0;
    }
   }
   else if ((lowerRow & 0x38) == 0) {destSlot = 0;}
   else if ((lowerRow & 0x07) == 0) {destSlot = 1;}
   else if ((lowerRow & 0x38) == 0x38) {destSlot = 0;}
   else if ((lowerRow & 0x07) == 0x07) {destSlot = 1;}
   else {
    printString("\n\rDefault hit (straight piece)\n\r");
   }

   tCmdNum = getCmds(ptemp, destSlot);


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


  else {
   if (lowerRow == 0) {
    minCmdNum = 20;
    k = 2;
    while (k < 5){
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
    destSlot = 4;
   }
   else if (lowerRow == 0x07) {
    tCmdNum = getCmds(ptemp, 2);
    destSlot = 2;
   }
   else if ((lowerRow & 0x38) == 0x30) {destSlot = 5;}
   else if ((lowerRow & 0x07) == 0x03) {destSlot = 7;}
   else if ((lowerRow & 0x38) == 0x18) {destSlot = 3;}
   else if ((lowerRow & 0x07) == 0x06) {destSlot = 9;}
   else { printString("\n\rDefault hit (corner piece)\n\r");}

   tCmdNum = getCmds(ptemp, destSlot);

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
    else if (leftBlock > 0) {
     leftBlock--;
     upperRow = 0x38;
    }
   }
  }
  YKEnterMutex();
# 250 "lab8app.c"
  cmdArray[cmdNext].pieceID = ptemp->pieceID;
  cmdArray[cmdNext].slide1 = tmpCmd.slide1;
  cmdArray[cmdNext].rotate = tmpCmd.rotate;
  cmdArray[cmdNext].slide2 = tmpCmd.slide2;
  if (YKQPost(cmdQPtr, (void *) &(cmdArray[cmdNext])) == 0)
   printString("\n\rcmdQ overflow!\n\r");
  else if (++cmdNext >= 10)
   cmdNext = 0;
  YKExitMutex();

 }
}


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


void SMStatTask(void)
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

    YKNewTask(SMcmdTask, (void *) &SMcmdTaskStk[512], 1);
    YKNewTask(SMpieceTask, (void *) &SMpieceTaskStk[512], 2);

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
 score = 0;
 pieceQPtr = YKQCreate(pieceQ, 10);
 cmdQPtr = YKQCreate(cmdQ, 10);
 SemPtr = YKSemCreate(1, "PSem");
 YKNewTask(SMStatTask, (void *) &SMStatTaskStk[512], 0);
 YKRun();
}
