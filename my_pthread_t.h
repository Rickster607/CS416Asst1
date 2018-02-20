// File:	my_pthread_t.h
// Author:	Yujie REN
// Date:	09/23/2017

// name: Richard Stoltzfus, William He
// username of iLab: ras480, wh250
// iLab Server: vi.cs.rutgers.edu

#ifndef MY_PTHREAD_T_H
#define MY_PTHREAD_T_H

#define _GNU_SOURCE

/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <stddef.h>

//typedef uint my_pthread_t;

typedef struct my_pthread_t {
	int tid;
	struct ucontext_t ucp;
	void* retVal;
	int priority;
	int yielding;
	int done;
	struct my_pthread_t* next;
} my_pthread_t;

typedef struct threadQueue {
	int numThreads;
	my_pthread_t* head;
	my_pthread_t* tail;
} queue;

/* mutex struct definition */
typedef struct my_pthread_mutex_t {
	int locked;
} my_pthread_mutex_t;

typedef struct threadScheduler {
	queue* running;
	queue* waiting;
	queue* finished;
	my_pthread_t* currentThread;
} threadScheduler;


/* Function Declarations: */

void initialize();

void myScheduler();

void scheduleThread(my_pthread_t* newThread);

void threadHandler(my_pthread_t* runningThread, void* (*function) (void*), void* arg);

int isDone(int tid);

void makeQueue(queue* newQueue);

void enqueue(queue* myQueue, my_pthread_t* newThread);

my_pthread_t* dequeue(queue* myQueue);

/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg);

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield();

/* terminate a thread */
void my_pthread_exit(void *value_ptr);

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr);

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex);

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex);

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex);


#define USE_MY_PTHREAD 1
#ifdef USE_MY_PTHREAD
#define pthread_t my_pthread_t
#define pthread_mutex_t my_pthread_mutex_t
#define pthread_create my_pthread_create
#define pthread_exit my_pthread_exit
#define pthread_join my_pthread_join
#define pthread_mutex_init my_pthread_mutex_init
#define pthread_mutex_lock my_pthread_mutex_lock
#define pthread_mutex_unlock my_pthread_mutex_unlock
#define pthread_mutex_destroy my_pthread_mutex_destroy
#endif

#endif
