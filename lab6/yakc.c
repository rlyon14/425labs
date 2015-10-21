#include "clib.h"
#include "yakk.h"
#include "yaku.h"

#define NULL 0
#define IDLE_STACKSIZE 256

int* YKsave;
int* YKrestore;

int next;
int data;

extern YKQ *MsgQPtr; 
extern struct msg MsgArray[];
extern int GlobalFlag;

extern YKSEM *NSemPtr;

extern int KeyBuffer;
int YKqueueCount;
int YKsemCount;
int YKtaskCount;
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

YKSEM YKsemaphores[MAXSEM];
struct Task YKtasks[MAXTASK];
YKQ YKqueues[MAXQUEUES];

int YKIdleStk[IDLE_STACKSIZE];

void YKIdleTask(void){
	while(1){
		YKIdleCount++;
		if(YKIdleVar){
			YKIdleVar++;
		}
	}
}

YKQ *YKQCreate(void **start, unsigned size){
	YKQ *qtemp;
	YKEnterMutex();
	qtemp = &YKqueues[YKqueueCount];
	qtemp->nextEmpty = (int*)start;
	qtemp->oldest = NULL;
	qtemp->qSize = size;
	qtemp->qCount = 0;
	qtemp->qStart = (int*)start;
	qtemp->qEnd = ((int*)start)+(size-1);
	qtemp->qBlockedHead = NULL;
	YKqueueCount++;
	//printYKQ(qtemp);
	if (YKRunFlag == 1){
		YKExitMutex();
	}
	return qtemp;
}

void *YKQPend(YKQ *queue){
	void *retMSG;
	struct Task* item;
	YKEnterMutex();
	if (queue->oldest == NULL){
		item = YKpopSorted(&readyHead);
		YKinsertSorted(item, &(queue->qBlockedHead));
		YKScheduler(1); 
	}
	retMSG = (void*) *(queue->oldest);
	(queue->oldest)++;
	queue->qCount--;
	if (queue->qCount <= 0){
		queue->oldest = NULL;
	}
	else if ((queue->oldest) > (queue->qEnd)){
		queue->oldest = queue->qStart;
	}
	//printString("Pend\n\r");
	//printMsgQueue(queue);
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
	//printString("Post\n\r");
	//printMsgQueue(queue);
	YKExitMutex();
	return 1;
}

void printYKQ(YKQ *queue){
	YKEnterMutex();
	printString("Printing YKQ:\n\r");
	printString("NextEmpty: "); 
	printWord((int)queue->nextEmpty);
	printString(", oldest: ");
	printWord((int)queue->oldest);
	printString(", size: ");
	printInt(queue->qSize);
	printString(", count: ");
	printInt(queue->qCount);
	printString(", qStart: "); 
	printWord((int)queue->qStart);
	printString(", qEnd: ");
	printWord((int)queue->qEnd);
	printNewLine();
	YKExitMutex();
}

void printMsgQueue(YKQ *queue){
	int *tempMsg;
	int k;
	YKEnterMutex();
	printString("PrintingQueue:\n\r");
	k = 0;
	tempMsg = (int*)queue->oldest;
	if (tempMsg != NULL) {
		for (k; k < (queue->qCount); k++){
			printString("[");
			printWord((int) tempMsg);
			printString("]: "); 
			printInt((int)*tempMsg);
			printString(": ");
			printInt(*((int*)*tempMsg));
			printString(", ");
			tempMsg++;
			if (((int*) tempMsg) > (queue->qEnd)){
				tempMsg = queue->qStart;
			}
		}
	printNewLine();
	}
	YKExitMutex();
}

YKSEM* YKSemCreate(int initialValue, char *string){
	YKSEM *tempSem;
	YKEnterMutex();
	if (initialValue < 0){
		return NULL;
	}
	tempSem = &YKsemaphores[YKsemCount];	
	tempSem->value = initialValue;
	tempSem->pendHead = NULL;
	tempSem->string = string;
	YKsemCount++;
	if (YKRunFlag == 1){
		YKExitMutex();
	}
	return tempSem;
}

void YKInitialize(void){
	YKEnterMutex();
	YKsave = NULL;
	YKrestore = NULL;
	YKTickCount = 0;
	YKCtxSwCount = 0;
	YKIdleCount = 0;
	YKRunFlag = 0;
	YKtaskCount = 0;
	readyHead = NULL;
	blockedHead = NULL;
	blockedTail = NULL;
	YKRunningTask = NULL;
	YKsemCount = 0;
	YKIsrDepth = 0;
	YKsemCount = 0;
	YKqueueCount = 0;
	next = 0;
	data = 0;
	YKNewTask(&YKIdleTask, &YKIdleStk[IDLE_STACKSIZE], 100);
}

void YKNewTask(void (*task)(void), int *taskStack, unsigned char priority){
	struct Task* tempTask;
	int *tempSP;
	int k;
	YKEnterMutex();
	//set up inital stack frame
	tempSP = taskStack-1;
	*tempSP = 0x0200; 	//initial value for flags (0x0200)
	--tempSP;
	*tempSP = 0; 		//initial value for CS
	--tempSP;
	*tempSP = (int)task; 	//initial value for IP
	--tempSP;
	*tempSP = (int)(taskStack-1);	//initial value for BP
	--tempSP;
	for(k = 0; k <=7; k++){
		*tempSP = 0;
		--tempSP;
	}
	
	//initialize TCB
	tempTask = &YKtasks[YKtaskCount];
	tempTask->taskSP = tempSP+1;
	tempTask->taskPriority = priority;
	tempTask->taskDelay = 0;
	
	YKtaskCount++;
	//insert into ready list 
	YKinsertSorted(tempTask, &readyHead);
	//printStack(tempTask);
	//printLists();
	if (YKRunFlag == 1){
		YKScheduler(1);
		YKExitMutex();
	}
}

void YKRun(void) {
	if (readyHead != NULL){
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
		YKinsertBlocked(item);
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
		//printString("\n\rContext Switch\n\r");
		YKCtxSwCount++;
		YKDispatcher(saveContext);
	}
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
	if ((semaphore->pendHead) != NULL) {
		item = YKpopSorted(&(semaphore->pendHead));
		YKinsertSorted(item, &readyHead);
		if (YKIsrDepth == 0){	
			YKScheduler(1);
		}
	}
	YKExitMutex();
}

//assuming tick isr is highest priority
void YKTickHandler(void){
	struct Task* temp;
	struct Task* tempNext;
	//static int next;
	//static int data;
	YKTickCount++;

	/* create a message with tick (sequence #) and pseudo-random data */
	MsgArray[next].tick = YKTickCount;
	data = (data + 89) % 100;
	MsgArray[next].data = data;
	if (YKQPost(MsgQPtr, (void *) &(MsgArray[next])) == 0)
		printString("  TickISR: queue overflow! \n");
	else if (++next >= MSGARRAYSIZE)
		next = 0;

	temp = blockedHead;
	while(temp != NULL){
		tempNext = temp->next;
		(temp->taskDelay)--;
		if (temp->taskDelay <= 0){
			YKremoveBlocked(temp);
			YKinsertSorted(temp, &readyHead);
		}
		if (tempNext != NULL){
			temp = tempNext;
		}
		else break;
	}
}

void YKkeypress(void){
	GlobalFlag = 1;
}

void YKinsertSorted(struct Task* item, struct Task** listHead){
	struct Task* temp;
	struct Task* tempNext;
	if (item != NULL) {
		//add to empty list
		item->prev = NULL;
		if ((*listHead) == NULL){
			item->next = NULL;
			*listHead = item;
		}
		//add before listHead
		else if (((*listHead)->taskPriority) >= (item->taskPriority)){
			item->next = *listHead;
			*listHead = item;
		}
	
		else {
			temp = *listHead;
			tempNext = (*listHead)->next;
			while (tempNext != NULL) {
				if ((tempNext->taskPriority) > (item->taskPriority)) {
					//put in bewtween temp and temp next
					item->next = tempNext;
					temp->next = item;
					return;
				}
			temp = tempNext;
			tempNext = tempNext->next;
			}		
			//add at tail
			temp->next = item;
			item->next = NULL;
		}
	}
}

struct Task* YKpopSorted (struct Task** listHead){	
	struct Task* temp;
	if (*listHead != NULL) {
		temp = *listHead;
		*listHead = (*listHead)->next;
		temp->next = NULL;
		return temp;
	}
	else { 
		return NULL;
	}
}

void YKremoveBlocked (struct Task* item){
	if (item != NULL) {

		if (item->prev != NULL) {
			(item->prev)->next = item->next;
		}
		else {
			blockedHead = item->next;
		}
		if (item->next != NULL) {
			(item->next)->prev = item->prev;
		}
		else {
			blockedTail = item->prev;
		}
	}
}
void YKinsertBlocked(struct Task* item){
	if (item != NULL) {
		if (blockedTail == NULL){
			blockedHead = item;
			blockedTail = item;
			item->next = NULL;
			item->prev = NULL;
		}
		else {
			blockedTail->next = item;
			item->prev = blockedTail;
			blockedTail = item;
			item->next = NULL;
		}
	}
}

void printSem(YKSEM *semaphore){
	struct Task *temp;
	if (semaphore != NULL){
		printNewLine();
		printString(semaphore->string);
		printString(": ");
		printInt(semaphore->value);
		printNewLine();
		printString("PendList: \n\r");
		temp = semaphore->pendHead;
		while (temp != NULL){
			printString("[0x");
			printByte(temp->taskPriority);
			printString(", ");
			printWord(temp->taskDelay);
			printString(", ");
			printWord((int)temp->taskSP);
			printString("] ");
			temp = temp->next;	
		}
		printNewLine();
	}
}

void printStack(struct Task* item){
	int k;
	int *tempSP;
	k = 0;
	if (item != NULL){
		tempSP = item->taskSP;
		printString("\n\rPrinting Stack:\n\r");
		for (k; k<12; k++){
			printWord((int)tempSP);
			printString(": [");
			printWord(*tempSP);
			printString("]\n\r");
			tempSP = tempSP+1;
		}
	}
	else {
		printString("\n\rPrintStack: item is NULL");

	}	
}

void printLists(void){
	struct Task* temp;
	printString("\n\rReady List:\n\r");
	temp = readyHead;
	while(temp != NULL){
		printString("[0x");
		printByte(temp->taskPriority);
		printString(", ");
		printWord(temp->taskDelay);
		printString(", ");
		printWord((int)temp->taskSP);
		printString("] ");
		temp = temp->next;
	}
	printNewLine();
	temp = blockedHead;
	printString("Blocked List:\n\r");
	while(temp != NULL){
		printString("[0x");
		printByte(temp->taskPriority);
		printString(", ");
		printWord(temp->taskDelay);
		printString(", ");
		printWord((int)temp->taskSP);
		printString("] ");
		temp = temp->next;
	}
	printString("\n\r");
}
