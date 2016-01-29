/*================================================
 *Copyright (c) dongyue.zippy All rights Reservied
 *================================================*/

/*
 * @author dongyue.z(zhangdy1986@gmail.com)
 * @date 2016-01-29 16:55:00
 * @file co_thread_pool.c
 * @brief 
 * 
 * */

#include "co_thread_pool.h"

extern void thread_pool_exit_cb(void* argv);


/* Initialize a thread pool by conf */
co_thread_pool_t* co_thread_pool_init(co_thread_pool_conf_t *conf)
{
	co_thread_pool_t *pool = NULL;
	int error_flag_mutex = 0;
	int error_flag_cond = 0;
	pthread_attr_t attr;
	do{
		if (conf_check(conf) == -1){ 
			break;
		}

		pool = (co_thread_pool_t *)malloc(sizeof(co_thread_pool_t));
		if (pool == NULL){
			break;
		}

		pool->_thread_num 			= conf->_thread_num;
		pool->_thread_stack_size 	= conf->_thread_stack_size;
		pool->_tasks._max_task_num 	= conf->_max_task_num;
		pool->_tasks._cur_task_num 	= 0;

		task_queue_init(&pool->_tasks);
	
		if (thread_key_create() != 0){/* create pthread_key_t,access global variable */
			free(pool);
			break;
		}
		if (thread_mutex_create(&pool->_mutex) != 0){/* init mutex */ 
			thread_key_destroy();
			free(pool);
			break;
		}

		if (thread_cond_create(&pool->_cond) != 0){ /* init cond */ 
			thread_key_destroy();
			thread_mutex_destroy(&pool->_mutex);
			free(pool);
			break;
		}

		if (thread_pool_create(pool) != 0){      /* create thread pool */ 
			thread_key_destroy();
			thread_mutex_destroy(&pool->_mutex);
			thread_cond_destroy(&pool->_cond);
			free(pool);
			break;
		}

		return pool;

	}while(0);

	return NULL;
}

/* Add task to thread pool */
int co_thread_pool_add_task(co_thread_pool_t *pool, CB_FUN handler, void* argv)
{
	co_task_t *task = NULL;

	/* get a task */
	task = (co_task_t *)malloc(sizeof(co_task_t));
	if (task == NULL){
		return -1;
	}

	task->_handler = handler;
	task->_argv = argv;
	task->_next = NULL;

	if (pthread_mutex_lock(&pool->_mutex) != 0){/* lock */ 
		free(task);
		return -1;
	}

	do{

		if (pool->_tasks._cur_task_num >= pool->_tasks._max_task_num){
			break;
		}

		/* insert task to queue tail */
		*(pool->_tasks._tail) = task;
		pool->_tasks._tail = &task->_next;
		pool->_tasks._cur_task_num++;

		/* notify to blocked thread task */
		if (pthread_cond_signal(&pool->_cond) != 0){
			break;
		}

		/* unlock */
		pthread_mutex_unlock(&pool->_mutex);
		return 0;

	}while(0);

	pthread_mutex_unlock(&pool->_mutex);
	free(task);
	return -1;

}

/* Destroy the thread pool */
void co_thread_pool_destroy(co_thread_pool_t *pool)
{
	unsigned int n = 0;
	volatile unsigned int  lock;

	/* thread_pool_exit_cb make it quit */
	for (; n < pool->_thread_num; n++){
		lock = 1;
		if (co_thread_pool_add_task(pool, thread_pool_exit_cb, &lock) != 0){
			return;
		}
		while (lock){
			usleep(1);
		}
	}
	thread_mutex_destroy(&pool->_mutex);
	thread_cond_destroy(&pool->_cond);
	thread_key_destroy();
	free(pool);
}

/* Add a job thread to thread pool */
int co_thread_add(co_thread_pool_t *pool)
{
	int ret = 0;
	if (pthread_mutex_lock(&pool->_mutex) != 0){
		return -1;
	}
	ret = thread_add(pool);
	pthread_mutex_unlock(&pool->_mutex);
	return ret;
}

/* Set the max task number,0 is no limit */
void co_set_max_tasknum(co_thread_pool_t *pool,unsigned int num)
{
	if (pthread_mutex_lock(&pool->_mutex) != 0){
		return -1;
	}
	change_maxtask_num(pool, num);  
	pthread_mutex_unlock(&pool->_mutex);
}

