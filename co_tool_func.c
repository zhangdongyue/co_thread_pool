/*================================================
 *Copyright (c) dongyue.zippy All rights Reservied
 *================================================*/


/*
 * @author dongyue.z (zhangdy1986@gmail.com)
 * @date 2016-01-29 18:54:00
 * @file co_tool_func.h 
 * @brief
 *
 * */

#include "co_thread_pool.h"
#include <signal.h>

#define MAX_TASK_SIZE 99999999
static 	pthread_key_t  key;

void 		thread_pool_cycle(void* argv);
void  		thread_pool_exit_cb(void* argv);
int 		conf_check(co_thread_pool_conf_t *conf);
int 		thread_mutex_create(pthread_mutex_t *mutex);
int 		thread_pool_create(co_thread_pool_t *pool);
inline void task_queue_init(co_task_queue_t* task_queue);
inline void thread_mutex_destroy(pthread_mutex_t *mutex);
inline void thread_cond_destroy(pthread_cond_t *cond);
inline void change_maxtask_num(co_thread_pool_t *pool, unsigned int num);
inline void thread_key_destroy();
inline int 	thread_cond_create(pthread_cond_t *cond);
inline int 	thread_add(co_thread_pool_t *pool);
inline int 	thread_key_create();

int conf_check(co_thread_pool_conf_t *conf)
{
	if (conf == NULL){
		return -1;
	}

	if (conf->_thread_num < 1){
		return -1;
	}

	if (conf->_max_task_num < 1){
		conf->_max_task_num = MAX_TASK_SIZE;
	}
	return 0;
}


inline void  task_queue_init(co_task_queue_t* task_queue)
{
	task_queue->_head = NULL;
	task_queue->_tail = &task_queue->_head;
}

int thread_mutex_create(pthread_mutex_t *mutex)
{
	int ret = 0;
	pthread_mutexattr_t attr;

	if (pthread_mutexattr_init(&attr) != 0){
		return -1;
	}

	if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK) != 0){
		pthread_mutexattr_destroy(&attr);
		return -1;
	}

	ret = pthread_mutex_init(mutex,&attr);

	pthread_mutexattr_destroy(&attr);

	return ret;
}

inline  thread_mutex_destroy(pthread_mutex_t *mutex)
{
	pthread_mutex_destroy(mutex);
}

inline int thread_cond_create(pthread_cond_t *cond)
{
	return pthread_cond_init(cond, NULL);
}

inline void thread_cond_destroy(pthread_cond_t *cond)
{
	pthread_cond_destroy(cond);
}


int thread_pool_create(co_thread_pool_t *pool)
{
	int i = 0;
	pthread_t  pid;
	pthread_attr_t attr;

	if (pthread_attr_init(&attr) != 0){
		return -1;
	}

	if (pool->_thread_stack_size != 0)
	{
		if (pthread_attr_setstacksize(&attr, pool->_thread_stack_size) != 0){
			pthread_attr_destroy(&attr);
			return -1;
		}
	}

	/* Create thread pool */
	for (; i < pool->_thread_num; ++i)
	{
		pthread_create(&pid, &attr, thread_pool_cycle,pool);
	}	
	pthread_attr_destroy(&attr);

	return 0;
}



int thread_add(co_thread_pool_t *pool)
{
	pthread_t  pid;
	pthread_attr_t attr;
	int ret = 0;
	if (pthread_attr_init(&attr) != 0){
		return -1;
	}
	if (pool->_thread_stack_size != 0)
	{
		if (pthread_attr_setstacksize(&attr, pool->_thread_stack_size) != 0){
			pthread_attr_destroy(&attr);
			return -1;
		}
	}
	ret = pthread_create(&pid, &attr, thread_pool_cycle,pool);
	if (ret == 0)
	{
		pool->_thread_num++;
	}
	pthread_attr_destroy(&attr);
	return ret;
}


inline void change_maxtask_num(co_thread_pool_t *pool, unsigned int num)
{
	pool->_tasks._max_task_num = num;
	if (pool->_tasks._max_task_num < 1)
	{
		pool->_tasks._max_task_num = MAX_TASK_SIZE;
	}
}

/* Worker thread */
void thread_pool_cycle(void* argv)
{
	unsigned int exit_flag = 0;
	sigset_t set;
	co_task_t *ptask = NULL;
	co_thread_pool_t *pool = (co_thread_pool_t*)argv;

	/* register signal */
	sigfillset(&set);
	sigdelset(&set, SIGILL);
	sigdelset(&set, SIGFPE);
	sigdelset(&set, SIGSEGV);
	sigdelset(&set, SIGBUS);
	
	if (pthread_setspecific(key,(void*)&exit_flag) != 0){/* set exit_flag = 0 */
		return;
	}
	if (pthread_sigmask(SIG_BLOCK, &set, NULL) != 0){
		return;
	}
	while(!exit_flag){         /* thread exit when exit_flag=1 */
		if (pthread_mutex_lock(&pool->_mutex) != 0){ /* lock */ 
			return;
		}

		while(pool->_tasks._head == NULL){
			if (pthread_cond_wait(&pool->_cond, &pool->_mutex) != 0){
				pthread_mutex_unlock(&pool->_mutex);
				return;
			}
		}
		
		ptask = pool->_tasks._head;    /* get a task from task queue */ 
		pool->_tasks._head = ptask->_next;

		pool->_tasks._cur_task_num--;   /* current task number-- */

		if (pool->_tasks._head == NULL){
			pool->_tasks._tail = &pool->_tasks._head;
		}

		if (pthread_mutex_unlock(&pool->_mutex) != 0){ /* unlock */ 
			return;
		}

		ptask->_handler(ptask->_argv); /* run task */ 
		free(ptask);
		ptask = NULL;
	}
	pthread_exit(0);
}

/* The callback funtion for thread pool exit.*/
void thread_pool_exit_cb(void* argv)
{
	unsigned int *lock = argv;
	unsigned int *pexit_flag = NULL;
	pexit_flag = (int *)pthread_getspecific(key);
	*pexit_flag = 1;    /* set exit_flag=1 */
	pthread_setspecific(key, (void*)pexit_flag);
	*lock = 0;
}

inline int thread_key_create()
{
	return pthread_key_create(&key, NULL);
}

inline void thread_key_destroy()
{
	pthread_key_delete(key);
}


