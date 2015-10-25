
#define MAXTASK 10
#define MAXSEM 10
#define MAXQUEUES 3
#define MSGARRAYSIZE 20
#define MAXEVENT 3

#define EVENT_A_KEY  0x1
#define EVENT_B_KEY  0x2
#define EVENT_C_KEY  0x4

#define EVENT_1_KEY  0x1
#define EVENT_2_KEY  0x2
#define EVENT_3_KEY  0x4

extern YKEVENT *charEvent;
extern YKEVENT *numEvent;

struct msg 
{
    int tick;
    int data;
};


