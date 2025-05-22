#include "strQueue.h"

/* contains auxiliary functios to implement history feature of given user commands */
/* for that we use strQueue */

/* Returns a pointer to the last n-th given line by user that is not a history command */
char *getNonHistN(strQueue *queue, int n);

/* Returns a pointer to the last given line by user that is not a history command */
char *getNonHistLast(strQueue *queue);