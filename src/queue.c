/* See LICENSE file for copyright and license details.
 * queue.c
 *
 *  DESCRIPTION
 *
 */

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"
#include "logging.h"



/* Queue */
/* queue_push: add an item to the queue */
void queue_push (Queue *q, char const *const msg)
{
    pthread_mutex_lock (&q->mutex);

    size_t msg_len = strlen (msg);

    char *q_new = malloc (msg_len + 1);
    memcpy (q_new, msg, msg_len);
    q_new[msg_len] = '\0';

    q->queue[q->end] = q_new;
    q->end = (q->end + 1) % q->size;

    /* the queue is full; we have to alloc more space */
    if (q->end == q->start)
    {
        size_t new_size = q->size * 2;
        q->queue = realloc (q->queue,
                            sizeof(*q->queue) * new_size);

        size_t distance  = q->size - q->start,
               new_start = new_size - distance;

        /* move the data from the old list to the correct
         * position in the new list */
        memmove (q->queue + new_start, q->queue + q->start,
                 sizeof(*q->queue) * distance);

        /* zero the empty newly allocated memory */
        memset (q->queue + q->start, 0,
                sizeof(*q->queue) * (new_size - q->size));

        q->size  = new_size;
        q->start = new_start;
    }

    pthread_mutex_unlock (&q->mutex);
}

/* queue_next: get the next item from the queue
 *  - returns NULL if the queue is empty */
char *queue_next (Queue *q)
{
    pthread_mutex_lock (&q->mutex);

    char *out = NULL;

    if (!queue_empty (q))
    {
        size_t out_len = strlen (q->queue[q->start]);

        out = malloc (out_len + 1);
        memcpy (out, q->queue[q->start], out_len);
        out[out_len] = '\0';

        free (q->queue[q->start]);
        q->queue[q->start] = NULL;
        q->start = (q->start + 1) % q->size;
    }

    pthread_mutex_unlock (&q->mutex);
    return out;
}

/* queue_empty: check if the queue is empty */
/* TODO: (does this need to be mutex'd?) */
bool queue_empty (Queue const *const restrict q)
{
    return q->start == q->end;;
}


/* queue_new: allocate a new queue */
Queue queue_new(void)
{
    static size_t const QUEUE_INITIAL_SIZE = 4;

    Queue out = { 0 };

    out.start = 0;
    out.end   = 0;
    out.size  = QUEUE_INITIAL_SIZE;

    out.queue = calloc (sizeof(*out.queue), out.size);

    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init (&attr) == -1)
    {
        fatal ("pthread_mutexattr_init failed");
    }
    if (pthread_mutex_init (&out.mutex, &attr) == -1)
    {
        fatal ("pthread_mutex_init failed");
    }

    return out;
}

/* queue_delete: free memory used by the queue */
void queue_delete (Queue *q)
{
    pthread_mutex_lock (&q->mutex);

    for (size_t i = 0; i < q->size; ++i)
    {
        free (q->queue[i]);
        q->queue[i] = NULL;
    }
    free (q->queue);
    q->queue = NULL;

    q->size  = 0;
    q->end   = 0;
    q->start = 0;

    pthread_mutex_unlock (&q->mutex);
}
/* END Queue */



/* MessageQueue */
/* queue_push: add an item to the queue */
void msg_queue_push (MessageQueue *q, char const *const msg)
{
    pthread_mutex_lock (&q->mutex);

    /* start listening for write events if we weren't already */
    FD_SET(q->sockfd, &q->write_fds);

    queue_push (&q->queue, msg);

    pthread_mutex_unlock (&q->mutex);
}

/* queue_next: get the next item from the queue
 *  - returns NULL if the queue is empty */
char *msg_queue_next (MessageQueue *q)
{
    pthread_mutex_lock (&q->mutex);

    char *out = queue_next (&q->queue);

    /* stop listening for write events if the queue is empty */
    if (msg_queue_empty (q))
    {
        FD_CLR(q->sockfd, &q->write_fds);
    }

    pthread_mutex_unlock (&q->mutex);
    return out;
}

/* queue_empty: check if the queue is empty */
/* TODO: (does this need to be mutex'd?) */
bool msg_queue_empty (MessageQueue const *const restrict q)
{
    return q->queue.start == q->queue.end;
}


/* queue_new: allocate a new queue */
MessageQueue msg_queue_new (int fd)
{
    MessageQueue out = { 0 };

    out.sockfd = fd;
    FD_ZERO (&out.write_fds);

    out.queue = queue_new();

    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init (&attr) == -1)
    {
        fatal ("pthread_mutexattr_init failed");
    }
    if (pthread_mutex_init (&out.mutex, &attr) == -1)
    {
        fatal ("pthread_mutex_init failed");
    }

    return out;
}

/* queue_delete: free memory used by the queue */
void msg_queue_delete (MessageQueue *q)
{
    pthread_mutex_lock (&q->mutex);

    queue_delete (&q->queue);
    q->sockfd = -1;

    pthread_mutex_unlock (&q->mutex);
}
/* END MessageQueue */

