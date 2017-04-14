#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include "condition.h"

typedef struct task 
{
	void *(*run)(void *arg);
	void *arg;
	struct task *next;
}task_t;


typedef struct threadpool 
{
	condition_t ready;
	task_t *first;			//任务队列头指针
	task_t *last;
	int counter;			//当前线程数
	int idle;				//当前正在等待的线程个数
	int max_threads;
	int quit;
}threadpool_t;

void threadpool_init(threadpool_t *pool,int num);

void threadpool_add_task(threadpool_t *pool,void *(*run)(void *arg),void *arg);

void threadpool_destroy(threadpool_t *pool);


#endif