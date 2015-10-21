
struct Task {
	int *taskSP;
	unsigned char taskPriority;
	unsigned int taskDelay;
	struct Task* next;
	struct Task* prev;
};

typedef struct {
	int value;
	struct Task* pendHead;
	char *string;
} YKSEM;

extern unsigned int YKTickCount;
extern unsigned int YKCtxSwCount;
extern unsigned int YKIdleCount;

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

void YKremoveBlocked (struct Task* item);

void YKinsertBlocked(struct Task* item);

void printSem(YKSEM *semaphore);

void printStack(struct Task* item);

void printLists(void);
