#include <stdio.h>
#include <errno.h>
#include<stdlib.h>
#include <time.h>
#include "threadpool.h"


void *thread_runtime(void *arg)
{
	printf("thread 0x%x is starting...\n",(int)pthread_self());
	threadpool_t *pool=(threadpool_t *)arg;
	
	struct timespec abstime;
	int timeout;
	
	while(1)
	{
		timeout=0;
		condition_lock(&pool->ready);
		pool->idle++;

		while(pool->first==NULL && !pool->quit)
		{
			printf("thread 0x%x is waiting...\n",(int)pthread_self());
			//condition_wait(&pool->ready);
			
			clock_gettime(CLOCK_REALTIME,&abstime);
			abstime.tv_sec+=2;
			int status=condition_timedwait(&pool->ready,&abstime);
			if(status==ETIMEDOUT)
			{
				printf("thread 0x%x is out of time...\n",(int)pthread_self());
				timeout=1;
				break;
			}
		
			
		}
		
		pool->idle--;

		if(pool->first!=NULL)
		{
			task_t *task=pool->first;
			pool->first=pool->first->next;

			condition_unlock(&pool->ready);
			task->run(task->arg);
			free(task);
			condition_lock(&pool->ready);
		}
		if(pool->quit&&pool->first==NULL)
		{
			pool->counter--;
			if(pool->counter==0)
			{
				condition_signal(&pool->ready);
			}
			condition_unlock(&pool->ready);
			break;
		}
		if(timeout==1 && pool->first==NULL)
		{
			pool->counter--;
			if(pool->counter==0)
			{
				condition_signal(&pool->ready);
			}
			condition_unlock(&pool->ready);
			break;
		}
		
		condition_unlock(&pool->ready);
	}
	printf("thread 0x%x is exiting...\n",(int)pthread_self());
	return NULL;
}

void threadpool_init(threadpool_t *pool,int threads)
{
	condition_init(&pool->ready);
	pool->first=NULL;
	pool->last=NULL;
	pool->counter=0;
	pool->idle=0;
	pool->max_threads=threads;
	pool->quit=0;

}

void threadpool_add_task(threadpool_t *pool,void *(*run)(void *arg),void *arg)
{
	task_t *newtask=(task_t*)malloc(sizeof(task_t));
	newtask->run=run;
	newtask->arg=arg;
	newtask->next=NULL;

	condition_lock(&pool->ready);

	//将任务添加到队列中
	if(pool->first==NULL)
	{
		pool->first=newtask;
		pool->last=newtask;
	}
	else 
	{
		pool->last->next=newtask;
		pool->last=newtask;
	}
	
	if(pool->idle>0)
	{
		condition_signal(&pool->ready);
	}
	else if(pool->counter<pool->max_threads)
	{
		pthread_t pid;
		pthread_create(&pid,NULL,thread_runtime,pool);
		pool->counter++;
	}
	condition_unlock(&pool->ready);
	
}

void threadpool_destroy(threadpool_t *pool)
{
	if(pool->quit)
	{
		return;
	}
	condition_lock(&pool->ready);
	pool->quit=1;
	if(pool->counter>0)
	{
		if(pool->idle>0)
		{
			condition_broadcast(&pool->ready);
		}
			
			//处于执行任务的线程不会受到广播,需要等待线程执行
		while(pool->counter>0)
		{
			condition_wait(&pool->ready);
		}
	}
	condition_unlock(&pool->ready);
	condition_destroy(&pool->ready);
}

