// File:	my_pthread.c
// Author:	Yujie REN
// Date:	09/23/2017

// name: Richard Stoltzfus, William He
// username of iLab: ras480, wh250
// iLab Server: vi.cs.rutgers.edu

#include "my_pthread_t.h"
#define threadStackSize 64000
//static my_pthread_mutex_t* m;
static threadScheduler* sched;
my_pthread_t* mainThread;
int init = 1;
int tid = 0;

void initialize() {
	sched = malloc(sizeof(threadScheduler));
	sched->running = malloc(sizeof(queue));
	makeQueue(sched->running);
	sched->waiting = malloc(sizeof(queue));
	makeQueue(sched->waiting);
	sched->currentThread = NULL;
	init = 0;
	/*
	getcontext(&(main->ucp));
	main = malloc(sizeof(my_pthread_t));
	main->ucp.uc_link = 0;
	main->ucp.uc_stack.ss_sp = malloc(threadStackSize*2);
	main->ucp.uc_stack.ss_size = threadStackSize;
	main->ucp.uc_stack.ss_flags = 0;
	main->tid = tid++;
	main->next = NULL;
	main->priority = 0;
	main->done = 0;
	sched->currentThread = main;
	sched->running->head = main;
	sched->running->tail = main;
	sched->running->numThreads = 1;
	makecontext(&(main->ucp), scheduler(), 0);
	signal(SIGALRM, scheduler);
	scheduler();
	*/
	return;
}

void scheduler() {

	return;
}

/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
	ucontext_t mct;
	if (init == 1)
		printf("before init\n");
		//initialize();
		//
	getcontext(&mct);
	mct.uc_stack.ss_sp = malloc(threadStackSize);
	mct.uc_stack.ss_size = threadStackSize;
	getcontext(&(thread->ucp));
	printf("after get\n");
	thread->ucp.uc_link = &mct;
	thread->ucp.uc_stack.ss_sp = malloc(threadStackSize);
	thread->ucp.uc_stack.ss_size = threadStackSize;
	thread->ucp.uc_stack.ss_flags = 0;
	thread->tid = tid++;
	thread->next = NULL;
	thread->priority = 0;
	thread->done = 0;
	makecontext(&(thread->ucp), (void*)function, 1, arg); //do I need to catch this with the scheduler?
	printf("make\n");
	//setcontext(&thread->ucp);
	printf("set\n");
	swapcontext(&mct, &thread->ucp);
	printf("swap\n");
	thread->done = 1;
	return 0;
};

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() {
	printf("yielding\n");
	//scheduler();
	return 0;
};

/* terminate a thread */
void my_pthread_exit(void *value_ptr) {
	printf("in pthread exit\n");
	return;
};

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr) {
	printf("in pthread join\n");
	while (thread.done != 1)
		my_pthread_yield();
	if (value_ptr != NULL)
		thread.retVal = value_ptr;
	return 0;
};

void makeQueue(queue* newQueue) {
	newQueue->head = NULL;
	newQueue->tail = NULL;
	newQueue->numThreads = 0;
	return;
};

void enqueue(queue* myQueue, my_pthread_t* newThread){
	if (myQueue->numThreads == 0) {
		myQueue->head = newThread;
		myQueue->head->next = NULL;
		myQueue->tail = newThread;
		myQueue->tail->next = NULL;
		myQueue->numThreads = 1;
	}
	else {
		myQueue->tail->next = newThread;
		myQueue->tail = newThread;
		myQueue->tail->next = NULL;
		myQueue->numThreads++;
	}
	return;
};

my_pthread_t* dequeue(queue* myQueue) {
	if (myQueue->numThreads == 0) {
		return NULL;
	}
	my_pthread_t* queueThread;
	queueThread = myQueue->head;
	if (myQueue->numThreads == 1) {
		myQueue->head = NULL;
		myQueue->tail = NULL;
	}
	else {
		myQueue->head = myQueue->head->next;
	}
	queueThread->next = NULL;
	myQueue->numThreads--;
	return queueThread;
};

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) {
	mutex->locked = 0;
	return 0;
};

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {
	//while(__sync_lock_test_and_set(&(mutex->locked), 1) == 1) {
		//implement waiting queue
		//printf("we're waiting\n");
		//enqueue(sched->waiting, sched->currentThread);
		//my_pthread_yield();
	///}
	//add to sched->running
	if (mutex->locked == 1)
		//printf("mutex already locked\n");
	//else{
		mutex->locked = 1;
		//printf("mutex locked\n");
	//}
	return 0;
};

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex) {
	//if (sched->waiting->head != NULL) {
		//get thread from waiting queue
		//my_pthread_t* queueThread = dequeue(sched->waiting);
		//scheduling stuff
	//}
	if (mutex->locked == 1)
		//printf("mutex locked in unlock\n");
	//else
		//printf("mutex not locked in unlock\n");
		mutex->locked = 0;
	return 0;
};

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {
	//if (mutex->locked == 1) {
		//error
	//}
	printf("in mutex destroy\n");
	return 0;
};

