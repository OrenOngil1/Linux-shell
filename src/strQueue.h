#define HISTLEN 20

/* Definition of a link in an strQueue  data structure which keeps the given commands*/
/* the link is doubly-linked for easier traversal */
typedef struct strLink{
    char *line;             /* pointer to a command given by user */
    struct strLink *next;   /* pointer to the next strLink in the strQueue if there's any, otherwise NULL */
    struct strLink *prev;   /* pointer to the previous strLink in the strQueue */
} strLink;

/* Definition of the strQueue itself */
typedef struct strQueue{
    strLink *first;     /* pointer to the first link that was added i.e. the first to be removed when more that HISTLEN strLinks exist */
    strLink *last;      /* pointer to the last strLink, representin the last command given by user */
    int size;           /* how many links are in queue, can be HISTLEN at most */
} strQueue;

/* Returns a pointer to the string in first in queue and removes it from the queue */
char* dequeue(strQueue *queue);

/* Puts a duplicate of line last in queue. */
/* If there are more than HISTLEN elements in queue, removes the first one */
void enqueue(strQueue *queue, char* line);

/* Frees the memory allocated to queue */
void freeStrQueue(strQueue *queue);

/* Prints the contents of queue to STDIN */
void printStrQueue(strQueue *queue);