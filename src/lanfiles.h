#define ACTION_SEND    0b00000001
#define ACTION_RECIVE  0b00000010
#define ACTION_ENCRYPT 0b00000100
char action = 0;
int maxBlockSize = 1024*1024; //1MB
char *filepath = 0;
