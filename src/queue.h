/*
 * DESCRIPTION
 *
 */

#ifndef _QUEUE_H
#define _QUEUE_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>


/* ==[ STRUCTS ]== */
/* MessageQueue:
 *  DESCRIPTION
 */
typedef struct MessageQueue
{
    int    sockfd;
    fd_set write_fds;

    size_t start,
           end,
           size;
    char **queue;

    pthread_mutex_t mutex;
} MessageQueue;


/* ==[ FUNCTIONS ]== */
MessageQueue queue_new (int fd);
void queue_delete (MessageQueue *q);

void  queue_push (MessageQueue *q, char const *const msg);
char *queue_next (MessageQueue *q);
bool  queue_empty (MessageQueue const *const restrict q);


#endif

