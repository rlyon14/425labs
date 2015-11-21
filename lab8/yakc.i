# 1 "yakc.c"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "yakc.c"
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
# 2 "yakc.c" 2
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
# 3 "yakc.c" 2
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
# 4 "yakc.c" 2





int* YKsave;
int* YKrestore;

int YKqueueCount;
int YKsemCount;
int YKtaskCount;
int YKeventCount;
int YKRunFlag;
int YKIsrDepth;
volatile int YKIdleVar;
unsigned int YKTickCount;
unsigned int YKCtxSwCount;
unsigned int YKIdleCount;
struct Task* readyHead;
struct Task* blockedHead;
struct Task* blockedTail;
struct Task* YKRunningTask;


struct Task YKtasks[10];
YKQ YKqueues[3];
YKEVENT YKevents[6];
YKSEM YKsemaphores[10];
int YKIdleStk[256];


void YKTickHandler(void){
 struct Task* temp;
 struct Task* tempNext;
 YKTickCount++;

 temp = blockedHead;
 while(temp != 0){
  tempNext = temp->next;
  (temp->taskDelay)--;
  if (temp->taskDelay <= 0){
   YKremoveUnsorted(temp, &blockedHead, &blockedTail);
   YKinsertSorted(temp, &readyHead);
  }
  if (tempNext != 0){
   temp = tempNext;
  }
  else break;
 }
}

void YKkeypress(void){

}


YKEVENT *YKEventCreate(unsigned initialValue){
 YKEVENT *etemp;
 YKEnterMutex();
 etemp = &YKevents[YKeventCount];
 etemp->value = initialValue;
 etemp->pendHead = 0;
 etemp->pendTail = 0;
 YKeventCount++;
 if (YKRunFlag == 1){
  YKExitMutex();
 }
 return etemp;
}

unsigned YKEventPend(YKEVENT *event, unsigned eventMask, int waitMode){
 unsigned tempMask;
 struct Task* item;
 YKEnterMutex();
 tempMask = eventMask & (event->value);
 if (waitMode == 1){
  if (tempMask > 0) {
   YKExitMutex();
   return event->value;
  }
 }
 else {
  if (tempMask == eventMask) {
   YKExitMutex();
   return event->value;
  }
 }

 if (readyHead != 0){
  item = YKpopSorted(&readyHead);
  YKinsertUnsorted(item, &(event->pendHead), &(event->pendTail));
  item->eventWaitMode = waitMode;
  item->eventMask = eventMask;
  YKScheduler(1);
 }
 YKExitMutex();
 return event->value;
}

void YKEventSet(YKEVENT *event, unsigned eventMask){
 struct Task* temp;
 struct Task* tempNext;
 YKEnterMutex();
 event->value = event->value | eventMask;
 temp = event->pendHead;
 while (temp != 0){
  if (temp->eventWaitMode == 0){
   if (temp->eventMask != (temp->eventMask & event->value)){
    temp = temp->next;
    continue;
   }
  }
  else {
   if ((temp->eventMask & event->value) == 0){
    temp = temp->next;
    continue;
   }
  }
  tempNext = temp->next;
  YKremoveUnsorted(temp, &(event->pendHead), &(event->pendTail));
  YKinsertSorted(temp, &readyHead);
  temp = tempNext;
 }
 if (YKIsrDepth == 0){
  YKScheduler(1);
 }
 YKExitMutex();
}

void YKEventReset(YKEVENT *event, unsigned eventMask){
 YKEnterMutex();
 event->value = (~eventMask & event->value);
 YKExitMutex();
}


YKQ *YKQCreate(void **start, unsigned size){
 YKQ *qtemp;
 YKEnterMutex();
 qtemp = &YKqueues[YKqueueCount];
 qtemp->nextEmpty = (int*)start;
 qtemp->oldest = 0;
 qtemp->qSize = size;
 qtemp->qCount = 0;
 qtemp->qStart = (int*)start;
 qtemp->qEnd = ((int*)start)+(size-1);
 qtemp->qBlockedHead = 0;
 YKqueueCount++;
 if (YKRunFlag == 1){
  YKExitMutex();
 }
 return qtemp;
}

void *YKQPend(YKQ *queue){
 void *retMSG;
 struct Task* item;
 YKEnterMutex();
 if (queue->oldest == 0){
  item = YKpopSorted(&readyHead);
  YKinsertSorted(item, &(queue->qBlockedHead));
  YKScheduler(1);
 }
 retMSG = (void*) *(queue->oldest);
 (queue->oldest)++;
 queue->qCount--;
 if (queue->qCount <= 0){
  queue->oldest = 0;
 }
 else if ((queue->oldest) > (queue->qEnd)){
  queue->oldest = queue->qStart;
 }
 YKExitMutex();
 return (void*)retMSG;
}

int YKQPost(YKQ *queue, void *msg){
 struct Task* item;
 YKEnterMutex();
 if ((queue->qCount) >= (queue->qSize)){
  YKExitMutex();
  return 0;
 }
 if((queue->qCount) <= 0){
  queue->oldest = queue->nextEmpty;
 }
 *(queue->nextEmpty) = (int) msg;
 queue->qCount++;
 (queue->nextEmpty)++;
 if ((queue->nextEmpty) > (queue->qEnd)){
  queue->nextEmpty = queue->qStart;
 }
 item = YKpopSorted(&(queue->qBlockedHead));
 YKinsertSorted(item, &readyHead);
 if (YKIsrDepth == 0){
  YKScheduler(1);
 }
 YKExitMutex();
 return 1;
}


void YKInitialize(void){
 YKEnterMutex();
 YKsave = 0;
 YKrestore = 0;
 YKTickCount = 0;
 YKCtxSwCount = 0;
 YKIdleCount = 0;
 YKRunFlag = 0;
 YKtaskCount = 0;
 readyHead = 0;
 blockedHead = 0;
 blockedTail = 0;
 YKRunningTask = 0;
 YKeventCount = 0;
 YKsemCount = 0;
 YKIsrDepth = 0;
 YKsemCount = 0;
 YKqueueCount = 0;
 YKNewTask(&YKIdleTask, &YKIdleStk[256], 100);
}

void YKIdleTask(void){
 while(1){
  YKIdleCount++;
  YKIdleVar++;
  YKIdleVar--;
 }
}

void YKNewTask(void (*task)(void), int *taskStack, unsigned char priority){
 struct Task* tempTask;
 int *tempSP;
 int k;
 YKEnterMutex();
 tempSP = taskStack-1;
 *tempSP = 0x0200;
 --tempSP;
 *tempSP = 0;
 --tempSP;
 *tempSP = (int)task;
 --tempSP;
 *tempSP = (int)(taskStack-1);
 --tempSP;
 for(k = 0; k <=7; k++){
  *tempSP = 0;
  tempSP--;
 }


 tempTask = &YKtasks[YKtaskCount];
 tempTask->taskSP = tempSP+1;
 tempTask->taskPriority = priority;
 tempTask->taskDelay = 0;
 tempTask->eventWaitMode = 1;
 tempTask->eventMask = 0;

 YKtaskCount++;
 YKinsertSorted(tempTask, &readyHead);
 if (YKRunFlag == 1){
  YKScheduler(1);
  YKExitMutex();
 }
}

void YKRun(void) {
 if (readyHead != 0){
  YKRunFlag = 1;
  YKrestore = readyHead->taskSP;
  YKRunningTask = readyHead;
  YKCtxSwCount++;
  YKDispatcher(0);
 }
 else {
  printString("YKRun Failed: readyHead is Null");
 }
}

void YKDelayTask(unsigned count){
 struct Task* item;
 if (count != 0){
  YKEnterMutex();
  item = YKRunningTask;
  item->taskDelay = count;
  YKpopSorted(&readyHead);
  YKinsertUnsorted(item, &blockedHead, &blockedTail);
  YKScheduler(1);
  YKExitMutex();
 }
}

void YKExitISR(void){
 YKIsrDepth--;
 if(YKIsrDepth == 0){
  YKScheduler(0);
 }
}

void YKEnterISR(void){
 YKIsrDepth++;
 if (YKIsrDepth == 1){
  YKsave = (int*)&(YKRunningTask->taskSP);
  YKsaveSP();
 }
}

void YKScheduler(int saveContext){
 if (YKRunningTask != readyHead){
  YKsave = (int*)&(YKRunningTask->taskSP);
  YKrestore = readyHead->taskSP;
  YKRunningTask = readyHead;
  YKCtxSwCount++;
  YKDispatcher(saveContext);
 }
}


YKSEM* YKSemCreate(int initialValue, char *string){
 YKSEM *tempSem;
 YKEnterMutex();
 if (initialValue < 0){
  return 0;
 }
 tempSem = &YKsemaphores[YKsemCount];
 tempSem->value = initialValue;
 tempSem->pendHead = 0;
 tempSem->string = string;
 YKsemCount++;
 if (YKRunFlag == 1){
  YKExitMutex();
 }
 return tempSem;
}

void YKSemPend(YKSEM *semaphore){
 int value;
 struct Task* item;
 YKEnterMutex();
 value = semaphore->value;
 (semaphore->value)--;
 if (value <= 0){
  item = YKpopSorted(&readyHead);
  YKinsertSorted(item, &(semaphore->pendHead));
  YKScheduler(1);
 }
 YKExitMutex();
}

void YKSemPost(YKSEM *semaphore){
 struct Task* item;
 YKEnterMutex();
 (semaphore->value)++;
 if ((semaphore->pendHead) != 0) {
  item = YKpopSorted(&(semaphore->pendHead));
  YKinsertSorted(item, &readyHead);
  if (YKIsrDepth == 0){
   YKScheduler(1);
  }
 }
 YKExitMutex();
}


void YKinsertSorted(struct Task* item, struct Task** listHead){
 struct Task* temp;
 struct Task* tempNext;
 if (item != 0) {
  item->prev = 0;
  if ((*listHead) == 0){
   item->next = 0;
   *listHead = item;
  }
  else if (((*listHead)->taskPriority) >= (item->taskPriority)){
   item->next = *listHead;
   *listHead = item;
  }
  else {
   temp = *listHead;
   tempNext = (*listHead)->next;
   while (tempNext != 0) {
    if ((tempNext->taskPriority) > (item->taskPriority)) {
     item->next = tempNext;
     temp->next = item;
     return;
    }
   temp = tempNext;
   tempNext = tempNext->next;
   }
   temp->next = item;
   item->next = 0;
  }
 }
}

struct Task* YKpopSorted (struct Task** listHead){
 struct Task* temp;
 if (*listHead != 0) {
  temp = *listHead;
  *listHead = (*listHead)->next;
  temp->next = 0;
  return temp;
 }
 else {
  return 0;
 }
}

void YKremoveUnsorted (struct Task* item, struct Task** listHead, struct Task** listTail){
 if (item != 0) {

  if (item->prev != 0) {
   (item->prev)->next = item->next;
  }
  else {
   *listHead = item->next;
  }
  if (item->next != 0) {
   (item->next)->prev = item->prev;
  }
  else {
   *listTail = item->prev;
  }
 }
}

void YKinsertUnsorted(struct Task* item, struct Task** listHead, struct Task** listTail){
 if (item != 0) {
  if (*listTail == 0){
   *listHead = item;
   *listTail = item;
   item->next = 0;
   item->prev = 0;
  }
  else {
   (*listTail)->next = item;
   item->prev = *listTail;
   *listTail = item;
   item->next = 0;
  }
 }
}
