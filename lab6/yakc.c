#include "clib.h"
#include "yakk.h"
#include "yaku.h"

#define NULL 0
#define IDLE_STACKSIZE 256

//pointers to TCB stack frames
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

//allocate space for kernel data structures
struct Task YKtasks[MAXTASK];
YKQ YKqueues[MAXQUEUES];
YKEVENT YKevents[MAXEVENT];
YKSEM YKsemaphores[MAXSEM];
int YKIdleStk[IDLE_STACKSIZE];

//lab7 variables
int next;
int data;
extern YKQ *MsgQPtr; 
extern struct msg MsgArray[];
extern int GlobalFlag;

//tick handlers
void YKTickHandler(void){
	struct Task* temp;
	struct Task* tempNext;
	YKTickCount++;

	/* lab7 tick handler */
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
	GlobalFlag = 1;
}

//Event functions
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

//Queue functions
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

//Basic Kernel functions
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
	*tempSP = 0x0200; 		//initial value for flags (0x0200)
	--tempSP;
	*tempSP = 0; 			//initial value for CS
	--tempSP;
	*tempSP = (int)task; 		//initial value for IP
	--tempSP;
	*tempSP = (int)(taskStack-1);	//initial value for BP
	--tempSP;
	for(k = 0; k <=7; k++){
		*tempSP = 0;
		tempSP--;
	}
	
	//initialize TCB
	tempTask = &YKtasks[YKtaskCount];
	tempTask->taskSP = tempSP+1;
	tempTask->taskPriority = priority;
	tempTask->taskDelay = 0;
	tempTask->eventWaitMode = EVENT_WAIT_ANY;
	tempTask->eventMask = 0;
	
	YKtaskCount++;
	YKinsertSorted(tempTask, &readyHead);
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
		YKCtxSwCount++;
		YKDispatcher(saveContext);
	}
}

//Semaphore functions
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

//list functions
void YKinsertSorted(struct Task* item, struct Task** listHead){
	struct Task* temp;
	struct Task* tempNext;
	if (item != NULL) {
		item->prev = NULL;
		if ((*listHead) == NULL){
			item->next = NULL;
			*listHead = item;
		}
		else if (((*listHead)->taskPriority) >= (item->taskPriority)){
			item->next = *listHead;
			*listHead = item;
		}
		else {
			temp = *listHead;
			tempNext = (*listHead)->next;
			while (tempNext != NULL) {
				if ((tempNext->taskPriority) > (item->taskPriority)) {
					item->next = tempNext;
					temp->next = item;
					return;
				}
			temp = tempNext;
			tempNext = tempNext->next;
			}		
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
