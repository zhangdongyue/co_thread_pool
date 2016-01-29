/*************************************************
 *Copyright (c) Dongyue.Zippy All right reservied. 
 *************************************************/

/*
 * @author dongyue.zippy(zhangdy1986@gmail.com)
 * @date 2016-01-29
 * @brief co_thread_pool.h
 * **/

#ifndef __CO_THREADPOOL__
#define __CO_THREADPOOL__

#include <pthread.h>

typedef void (*CB_FUN)(void *);

/* Task */
typedef struct task
{
	void		*_argv; 				/* parameters of handler */	
	CB_FUN		_handler; 				/* task call back handler,Must reture 0.*/
	struct task *_next; 				/* point to next task */	

}co_task_t;

/* Task Queue */
typedef struct task_queue
{
	co_task_t*	_head;  					
	co_task_t**	_tail;				
	unsigned int _max_task_num; 
	unsigned int _cur_task_num; 

}co_task_queue_t;

/* Thread Pool */
typedef struct thread_pool
{
	pthread_mutex_t    		_mutex;  
	pthread_cond_t     		_cond;
	co_task_queue_t       	_tasks;

	unsigned int       _thread_num; 			/* number of thread */	
	unsigned int       _thread_stack_size; 		/* size of thread stack */ 

}co_thread_pool_t;

/* Configure */
typedef struct thread_pool_conf
{
	unsigned int _thread_num;   			/* number of thread */ 
	unsigned int _thread_stack_size;		/* size of thread stack */
	unsigned int _max_task_num;				/* max task num */

}co_thread_pool_conf_t;

/* Init Thread Pool */
co_thread_pool_t* co_thread_pool_init(co_thread_pool_conf_t *conf);

/* Add task to thread pool */
int co_thread_pool_add_task(co_thread_pool_t *pool, CB_FUN handler, void* argv);

/* Destroy the thread pool */
void co_thread_pool_destroy(co_thread_pool_t *pool);

/* Add a thread pool */
int co_thread_add(co_thread_pool_t *pool);

/* Set the max task number */
void co_set_max_task_num(co_thread_pool_t *pool,unsigned int num);

#endif
