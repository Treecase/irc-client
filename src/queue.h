/* See LICENSE file for copyright and license details.
 * queue.h
 *
 *  DESCRIPTION
 *
 */

#ifndef _QUEUE_H
#define _QUEUE_H

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>


/* ==[ STRUCTS ]== */
/* Queue:
 *  DESCRIPTION
 */
typedef struct Queue
{
    size_t start,
           end,
           size;
    char **queue;

    pthread_mutex_t mutex;
} Queue;

/* MessageQueue:
 *  DESCRIPTION
 */
typedef struct MessageQueue
{
    int    sockfd;
    fd_set write_fds;

    Queue queue;

    pthread_mutex_t mutex;
} MessageQueue;


/* ==[ FUNCTIONS ]== */
Queue queue_new(void);
void queue_delete (Queue *q);

void  queue_push (Queue *q, char const *const msg);
char *queue_next (Queue *q);
bool  queue_empty (Queue const *const restrict q);


MessageQueue msg_queue_new (int fd);
void msg_queue_delete (MessageQueue *q);

void  msg_queue_push (MessageQueue *q, char const *const msg);
char *msg_queue_next (MessageQueue *q);
bool  msg_queue_empty (MessageQueue const *const restrict q);


#endif

