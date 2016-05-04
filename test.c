/*===============================================
 *Copyright (c) dongyue.zdy All Rights Rerservied
 *=============================================== */
/*
 *@date 2016-01-29 19:19:00
 *@brief test thread pool
 * */

#include<stdio.h>
#include<pthread.h>
#include<errno.h>
#include<stdlib.h>
#include "co_thread_pool.h"

FILE * fp = NULL;

int testfun(void *argv)
{
	int *num = (int*)argv;
	printf("testfun threadid = %u  num = %d\n",pthread_self(),*num);
	fprintf(fp,"testfun threadid=%u,num=%d\n",pthread_self(),*num);

	//sleep(3);
	return 0;
}

int main()
{
	fp = fopen("thread.log","wb+");
	if(!fp){
		fprintf(stderr,"[%s:%d]open file error:%s.\n",
				__FILE__,__LINE__,strerror(errno));
		return -1;
	}

	int array[10000] = {0};
	int i = 0;

	co_thread_pool_conf_t conf = {5,0,5}; //实例化启动参数
	co_thread_pool_t *pool = co_thread_pool_init(&conf);//初始化线程池
	if (pool == NULL){
		return 0;
	}

	for (; i < 10000; i++){
		array[i] = i;
		if (i == 80){
			co_thread_add(pool); //增加线程
			co_thread_add(pool);
		}
		
		if (i == 100){
			co_set_max_tasknum(pool, 0); //改变最大任务数   0为不做上限
		}
		while(1){
			if (co_thread_pool_add_task(pool, testfun, &array[i]) == 0){
				break;
			}
			printf("error in i = %d\n",i);
		
		}
	}
	co_thread_pool_destroy(pool);
/*
	while(1){
		sleep(5);
	}
	*/
	return 0;
}/*main*/
