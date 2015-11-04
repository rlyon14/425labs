#include "clib.h"
#include "yakk.h"
#include "yaku.h"

#define NULL 0
#define IDLE_STACKSIZE 256

int* YKsave;
int* YKrestore;

extern YKQ *MsgQPtr; 
extern struct msg MsgArray[];
extern int GlobalFlag;

extern YKSEM *NSemPtr;

extern int KeyBuffer;

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

YKSEM YKsemaphores[MAXSEM];
struct Task YKtasks[MAXTASK];
YKQ YKqueues[MAXQUEUES];
YKEVENT YKevents[MAXEVENT];

int YKIdleStk[IDLE_STACKSIZE];

void YKIdleTask(void){
	while(1){
		YKIdleCount++;
		YKIdleVar++;
		YKIdleVar--;
	}
}

YKEVENT *YKEventCreate(unsigned initialValue){
	YKEVENT *etemp;
	YKEnterMutex();
	etemp = &YKevents[YKeventCount];
	etemp->value = initialValue;
	etemp->pendHead = NULL;
	etemp->pendTail = NULL;
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
	if (waitMode == EVENT_WAIT_ANY){
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
	
	if (readyHead != NULL){
		item = YKpopSorted(&readyHead);
		YKinsertUnsorted(item, &(event->pendHead), &(event->pendTail));
		item->eventWaitMode = waitMode;
		item->eventMask = eventMask;
		//printEvent(event);
		YKScheduler(1);
	}
	YKExitMutex();
	return event->value;	//volatile?
}

void YKEventSet(YKEVENT *event, unsigned eventMask){
	struct Task* temp;
	struct Task* tempNext;
	YKEnterMutex();
	//bug was here, set event value to eventMask	
	event->value = event->value | eventMask;
	temp = event->pendHead;
	while (temp != NULL){
		if (temp->eventWaitMode == EVENT_WAIT_ALL){
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
	//printEvent(event);
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
	YKeventCount = 0;
	YKsemCount = 0;
	YKIsrDepth = 0;
	YKsemCount = 0;
	YKqueueCount = 0;
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
	tempTask->eventWaitMode = EVENT_WAIT_ANY;
	tempTask->eventMask = 0;
	
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
	YKTickCount++;

	temp = blockedHead;
	while(temp != NULL){
		tempNext = temp->next;
		(temp->taskDelay)--;
		if (temp->taskDelay <= 0){
			YKremoveUnsorted(temp, &blockedHead, &blockedTail);
			YKinsertSorted(temp, &readyHead);
		}
		if (tempNext != NULL){
			temp = tempNext;
		}
		else break;
	}
}

void YKkeypress(void){

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

void YKremoveUnsorted (struct Task* item, struct Task** listHead, struct Task** listTail){
	if (item != NULL) {

		if (item->prev != NULL) {
			(item->prev)->next = item->next;
		}
		else {
			*listHead = item->next;
		}
		if (item->next != NULL) {
			(item->next)->prev = item->prev;
		}
		else {
			*listTail = item->prev;
		}
	}
}

void YKinsertUnsorted(struct Task* item, struct Task** listHead, struct Task** listTail){
	if (item != NULL) {
		if (*listTail == NULL){
			*listHead = item;
			*listTail = item;
			item->next = NULL;
			item->prev = NULL;
		}
		else {
			(*listTail)->next = item;
			item->prev = *listTail;
			*listTail = item;
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

void printList(struct Task* listHead, char *string){
	struct Task* temp;
	printNewLine();
	printString(string);
	printNewLine();
	temp = listHead;
	while(temp != NULL){
		printString("[0x");
		printByte(temp->taskPriority);
		printString(", ");
		printWord(temp->taskDelay);
		printString(", 0x");
		printWord(temp->eventMask);
		printString(", ");
		printInt(temp->eventWaitMode);
		printString("] ");
		temp = temp->next;
	}
	printNewLine();
}

void printEvent(YKEVENT *event){
	printString("Printing Event:\n\r");
	printString("Value: 0x");
	printWord(event->value);
	printList(event->pendHead, "EventList:");
}
