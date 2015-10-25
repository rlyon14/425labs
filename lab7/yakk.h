
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

void printSem(YKSEM *semaphore);

void printStack(struct Task* item);

void printList(struct Task* listHead, char *string);
