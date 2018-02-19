// File:	my_pthread.c
// Author:	Yujie REN
// Date:	09/23/2017

// name: Richard Stoltzfus, William He
// username of iLab: ras480, wh250
// iLab Server: vi.cs.rutgers.edu

#include "my_pthread_t.h"
#define threadStackSize 64000

static threadScheduler* sched;
static struct itimerval schedTimer;
static struct timeval systemTime;
static my_pthread_t* mainThread;
int priorityArr[20] = {0,1,2,3,0,1,2,0,1,0,0,1,2,3,0,1,2,0,1,0};
static int queueLevels = 4;
static int init = 1;
static int tid = 0;
int test[4];

void initialize() {
	int i;
	for (i = 0; i < 4; i++)
		test[i] = 0;
	sched = malloc(sizeof(threadScheduler));
	sched->running = malloc(sizeof(queue) * queueLevels);
	for (i = 0; i < queueLevels; i++){
		makeQueue(&(sched->running[i]));
	}
	sched->waiting = malloc(sizeof(queue));
	makeQueue(sched->waiting);
	sched->currentThread = NULL;
	init = 0;

	mainThread = malloc(sizeof(my_pthread_t));
	getcontext(&(mainThread->ucp));
	mainThread->ucp.uc_link = 0;
	mainThread->ucp.uc_stack.ss_sp = malloc(threadStackSize*2);
	mainThread->ucp.uc_stack.ss_size = threadStackSize;
	mainThread->ucp.uc_stack.ss_flags = 0;
	mainThread->tid = tid++;
	mainThread->next = NULL;
	mainThread->priority = 0;
	mainThread->done = 0;

	schedTimer.it_value.tv_sec = 0;
	schedTimer.it_value.tv_usec = 50000;
	schedTimer.it_interval.tv_sec = 0;
	schedTimer.it_interval.tv_usec = 50000;

	sched->currentThread = mainThread;
	sched->running[0].head = mainThread;
	sched->running[0].tail = mainThread;
	sched->running[0].numThreads = 1;

	//scheduleThread(mainThread, scheduler(), NULL);
	/*
	//makecontext(&(mainThread->ucp), (void*)scheduleThread, 3, mainThread, (void*) scheduler(), NULL);

	scheduler();
	*/

	signal(SIGALRM, myScheduler);
	setitimer(ITIMER_REAL, &schedTimer, NULL);

	return;
}

void resetTimer(){
	schedTimer.it_value.tv_sec = 0;
	schedTimer.it_value.tv_usec = 50000;
	schedTimer.it_interval.tv_sec = 0;
	schedTimer.it_interval.tv_usec = 50000;
	setitimer(ITIMER_REAL, &schedTimer, NULL);
	return;
}

void stopTimer(){
	schedTimer.it_value.tv_sec = 0;
	schedTimer.it_value.tv_usec = 0;
	schedTimer.it_interval.tv_sec = 0;
	schedTimer.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &schedTimer, NULL);
	return;
}

void myScheduler() {
	stopTimer();
	int i;
//printf("in the scheduler\n");
	gettimeofday(&systemTime, NULL);
	long int priority = priorityArr[((systemTime.tv_sec + systemTime.tv_usec) % 20)];
	if (sched->running[priority].numThreads != 0){
		resetTimer();
		swapcontext(&(sched->currentThread), )
	}
	resetTimer();
	return;
}

void scheduleThread(my_pthread_t* newThread){
	printf("gotta schedule a thread\n");
	if (sched->running[newThread->priority].numThreads == 0)
		sched->running[newThread->priority].head = newThread;
	else
		sched->running[newThread->priority].head->next = newThread;
	myScheduler();
	return;
}

/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
	if (init == 1){
		printf("before init\n");
		initialize();
		printf("after init\n");
	}
	getcontext(&(thread->ucp));
	thread->ucp.uc_link = &(mainThread->ucp);
	thread->ucp.uc_stack.ss_sp = malloc(threadStackSize);
	thread->ucp.uc_stack.ss_size = threadStackSize;
	thread->ucp.uc_stack.ss_flags = 0;
	thread->tid = tid++;
	thread->next = NULL;
	thread->priority = 0;
	thread->done = 0;
	makecontext(&(thread->ucp), (void*)function, 1, arg); //do I need to catch this with the scheduler? no.
	//scheduleThread(newThread);
	swapcontext(&(mainThread->ucp), &thread->ucp);	//this is what needs to be done by the scheduler.
	thread->done = 1;
	int j;
	for (j = 0; j < 4; j++)
		printf("arr[%d] = %d\n", j, test[j]);
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

