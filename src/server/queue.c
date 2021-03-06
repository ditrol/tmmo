#include <malloc.h>

#include "queue.h"

/*
 *
 *	Simple thread-safe (??) implementation 
 *	of queue using mutex. 
 *
 */

static void queue_init_empty(queue_t* queue)
{
	queue->count = 0;
	queue->first = NULL;
	queue->last = NULL;
}

queue_t* queue_init()
{
	pthread_mutexattr_t *mutex_attr = malloc( sizeof(pthread_mutexattr_t) );
	queue_t* queue = malloc(sizeof(queue_t));
	if (!queue)
	{
		return NULL;
	}

	pthread_mutexattr_init(mutex_attr);
	pthread_mutexattr_settype(mutex_attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init( &(queue->mutex), mutex_attr);

	pthread_cond_init(&(queue->cond), NULL);
	queue_init_empty(queue);

	return queue;
}

bool queue_is_empty(queue_t* queue)
{
	if (queue->count > 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void queue_enqueue(queue_t* queue, void* data)
{
	queue_node_t* node = malloc(sizeof(queue_node_t));
	if (!node)
	{
		return;
	}

	node->data = data;

	pthread_mutex_lock(&queue->mutex);
	
	if (!queue_is_empty(queue))
	{
		queue->last->next = node;
		queue->last = node;
	}
	else
	{
		queue->last = node;
		queue->first = queue->last;
	}

	queue->count++;
	pthread_mutex_unlock(&queue->mutex);
	pthread_cond_signal(&queue->cond);
}

void queue_dequeue(queue_t* queue, void** data)
{
	queue_node_t* node = queue->first;
	
	if (queue_is_empty(queue))
	{
		*data = NULL;
		return;
	}

	pthread_mutex_lock(&queue->mutex);
	*data = node->data;
	queue->first = node->next;

	queue->count--;
	free(node);
	pthread_mutex_unlock(&queue->mutex);
}

void queue_destroy(queue_t* queue, void callback(void* data))
{
	queue_node_t* node;

	while (queue->count > 0)
	{
		node = queue->first;
		queue->first = node->next;

		if (callback)
		{
			callback(node->data);
		}

		free(node);
		queue->count--;
	}

	pthread_mutex_destroy(&queue->mutex);
	free(queue);
}