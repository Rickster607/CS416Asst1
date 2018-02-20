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
int priorityArr[23] = {0,1,2,3,0,1,2,0,1,0,4,5,0,1,2,3,0,1,2,0,1,0,4};
static int queueLevels = 4;
static int init = 1;
static int tid = 0;
static int threadsCreated = 0;
static int threadsFinished = 0;

void initialize() {
	int i;
	sched = (threadScheduler*)malloc(sizeof(threadScheduler));
	sched->running = malloc(sizeof(queue) * queueLevels);
	for (i = 0; i < queueLevels; i++){
		makeQueue(&(sched->running[i]));
	}
	sched->waiting = malloc(sizeof(queue));
	makeQueue(sched->waiting);
	sched->finished = malloc(sizeof(queue));
	makeQueue(sched->finished);
	sched->currentThread = NULL;
	init = 0;

	mainThread = malloc(sizeof(my_pthread_t));
	getcontext(&(mainThread->ucp));
	mainThread->ucp.uc_link = 0;
	mainThread->ucp.uc_stack.ss_sp = malloc(threadStackSize*2);
	mainThread->ucp.uc_stack.ss_size = threadStackSize*2;
	mainThread->ucp.uc_stack.ss_flags = 0;
	mainThread->tid = tid++;
	mainThread->next = NULL;
	mainThread->priority = 0;
	mainThread->done = 0;
	mainThread->yielding = 0;

	schedTimer.it_value.tv_sec = 0;
	schedTimer.it_value.tv_usec = 50000;
	schedTimer.it_interval.tv_sec = 0;
	schedTimer.it_interval.tv_usec = 50000;

	sched->currentThread = mainThread;
	//sched->running[0].head = mainThread;
	///sched->running[0].tail = mainThread;
	//sched->running[0].numThreads = 1;

	//scheduleThread(mainThread);
	signal(SIGALRM, myScheduler);
	setitimer(ITIMER_REAL, &schedTimer, NULL);

	//myScheduler();
	//scheduleThread(mainThread, scheduler(), NULL);
	//makecontext(&(mainThread->ucp), (void*)scheduleThread, 3, mainThread, (void*) scheduler(), NULL);

	//return;
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
////printf("in the scheduler, curr: %d\n", sched->currentThread);
	gettimeofday(&systemTime, NULL);
	long int priority = priorityArr[((systemTime.tv_sec + systemTime.tv_usec) % 23)];

	if (priority == 4 || priority == 5){
		if (priority == 4){
			for (i = 1; i < 4; i++){
				while (sched->running[i].numThreads > 0){
					////printf("numthread: %d\n", sched->running[i].numThreads);
					my_pthread_t* tmp;
					tmp = dequeue(&(sched->running[i]));
					enqueue(&(sched->running[0]), tmp);
				}
			}
			//myScheduler();
			//resetTimer();
			//return;
		}
		if (priority == 5){
			while (sched->waiting->numThreads > 0){
				////printf("dequeueing waiting\n");
				my_pthread_t* tmp;
				tmp = dequeue(sched->waiting);
				enqueue(&(sched->running[0]), tmp);
			}
			if (sched->running[0].numThreads > 0){
				my_pthread_t* tmp = sched->running[0].head;
				for (i = 0; i < sched->running[0].numThreads; i++){
					tmp->yielding = 0;
					tmp = tmp->next;
				}
			}
			//myScheduler();
			//resetTimer();
			//return;
		}
	}
	my_pthread_t* curr;
	////printf("berfore assign\n");
	curr = sched->currentThread;
	////printf("i hope we don't get here multiple times in a row\n");
	if (curr->done == 0){						//make this a check for finished threads
		if (curr->yielding == 0){				//only add to running queue if not yielding
			if (curr->priority < 3)
				curr->priority++;
			enqueue(&(sched->running[curr->priority]), curr);
		}
		else {
			////printf("scheduled waiting\n");
			enqueue(sched->waiting, curr);
		}
	}
	else{
		enqueue(sched->finished, curr);
	}
	if (sched->running[priority].numThreads != 0){
		my_pthread_t* tmp;
		tmp = dequeue(&(sched->running[priority]));
		sched->currentThread = tmp;
	}
	else{
		for (priority = 0; priority < 4; priority++){
			if (sched->running[priority].numThreads != 0){
				my_pthread_t* tmp;
				tmp = dequeue(&(sched->running[priority]));
				sched->currentThread = tmp;
				break;
			}
			else{
				if (sched->running[3].numThreads == 0 && priority == 3){
					////printf("no more threads in running queue\n");
					while (sched->waiting->numThreads > 0){
						////printf("dequeueing waiting\n");
						my_pthread_t* tmp;
						tmp = dequeue(sched->waiting);
						enqueue(&(sched->running[0]), tmp);
					}
					myScheduler();
					if ((threadsCreated - threadsFinished) == 0){
						////printf ("really, no more threads! :) \n");
						if (curr->done == 1){
							//printf("NO, REALLY!\n");
							//printf("curr->done: %d", curr->done);
							return;
						}
						//return;
					}
				}
			}
		}
	}
	//////printf("swapping context\n")
	////printf("curr: %d, sched: %d\n", curr->tid, sched->currentThread->tid);
	resetTimer();
	swapcontext(&(curr->ucp), &(sched->currentThread->ucp));
	return;
}

void scheduleThread(my_pthread_t* newThread){
	////printf("gotta schedule a thread\n");
	enqueue(&(sched->running[newThread->priority]), newThread);
	/*
	if (sched->running[newThread->priority].numThreads == 0){
		sched->running[newThread->priority].head = newThread;
		sched->running[newThread->priority].tail = newThread;
	}
	else
		sched->running[newThread->priority].head->next = newThread;
	*/
	//myScheduler();
	return;
}

void threadHandler(my_pthread_t* current, void* (*function) (void*), void* arg){

	current->retVal = function(arg);
	current->done = 1;
	threadsFinished++;
	////printf("thread %d of %d finished, %d remaining\n", current->tid, threadsCreated, (threadsCreated - threadsFinished));
	myScheduler();
}

int isDone(int tid){
	my_pthread_t* tmp;
	int i;
	tmp = sched->finished->head;
	for (i = 0; i < sched->finished->numThreads; i++){
		if (tmp->tid == tid){
			return 1;
		}
		else {
			tmp = tmp->next;
		}
	}
	return 0;

}

/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
	if (init == 1){
		////printf("before init\n");
		initialize();
		////printf("after init\n");
	}
	getcontext(&(thread->ucp));
	//thread->ucp.uc_link = &(mainThread->ucp);		//I think this should be commented out
	thread->ucp.uc_stack.ss_sp = malloc(threadStackSize);
	thread->ucp.uc_stack.ss_size = threadStackSize;
	thread->ucp.uc_stack.ss_flags = 0;
	thread->tid = tid++;
	thread->next = NULL;
	thread->priority = 0;
	thread->done = 0;
	thread->yielding = 0;
	makecontext(&(thread->ucp), (void*) threadHandler, 3, thread, (void*)function, arg); //do I need to catch this with the scheduler? no.
	enqueue(&(sched->running[thread->priority]), thread);
	//scheduleThread(newThread);
	//swapcontext(&(mainThread->ucp), &thread->ucp);	//this is what needs to be done by the scheduler.
	threadsCreated++;
	////printf("thread %d created and scheduled, %d total\n", thread->tid, threadsCreated);
	return 0;
};

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() {
	//////printf("yielding %d\n", sched->currentThread->tid);
	sched->currentThread->yielding = 1;
	myScheduler();
};

/* terminate a thread */
void my_pthread_exit(void *value_ptr) {
	////printf("in pthread exit\n");
	return;
};

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr) {
	////printf("in pthread join %d\n", thread.tid);
	while (isDone(thread.tid) != 1){
		my_pthread_yield();
		//getcontext(&thread.ucp);
	}
	//printf("joined?  !!!!\n");
};

void makeQueue(queue* newQueue) {
	newQueue->head = NULL;
	newQueue->tail = NULL;
	newQueue->numThreads = 0;
	return;
};

void enqueue(queue* myQueue, my_pthread_t* newThread){
	if (myQueue->numThreads < 1) {
		////printf ("adding thread %d to head of queue %d\n", newThread->tid, newThread->priority);
		myQueue->head = newThread;
		myQueue->head->next = NULL;
		myQueue->tail = newThread;
		myQueue->tail->next = NULL;
		myQueue->numThreads = 1;
		////printf("queue head: %d, tail: %d\n", myQueue->head, myQueue->tail);
	}
	else {
		if (newThread->tid != myQueue->tail->tid) {
		////printf ("adding thread %d to tail of queue %d\n", newThread->tid, newThread->priority);
		myQueue->tail->next = newThread;
		myQueue->tail = newThread;
		myQueue->tail->next = NULL;
		myQueue->numThreads++;
		//////printf("numthreads in queue = %d\n", myQueue->numThreads);
		}
	}
	return;
};

my_pthread_t* dequeue(queue* myQueue) {
	//////printf ("trying to dequeue from queue %d\n", myQueue);
	if (myQueue->numThreads == 0) {
		////printf("deq null\n");
		return NULL;
	}
	my_pthread_t* queueThread;

	if (myQueue->numThreads == 1) {
		queueThread = myQueue->head;
		myQueue->head = NULL;
		myQueue->tail = NULL;
		myQueue->numThreads = 0;
	}
	else {
		queueThread = myQueue->head;
		myQueue->head = myQueue->head->next;
		my_pthread_t* tmp;
		tmp = myQueue->head;
		while (tmp != NULL){
			//if (tmp->next != NULL){
				tmp = tmp->next;
			//}
		}
		myQueue->tail = tmp;
		myQueue->numThreads--;
	}
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
		//////printf("we're waiting\n");
		//enqueue(sched->waiting, sched->currentThread);
		//my_pthread_yield();
	///}
	//add to sched->running
	if (mutex->locked == 1)
		//////printf("mutex already locked\n");
	//else{
		mutex->locked = 1;
		//////printf("mutex locked\n");
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
		//////printf("mutex locked in unlock\n");
	//else
		//////printf("mutex not locked in unlock\n");
		mutex->locked = 0;
	return 0;
};

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {
	//if (mutex->locked == 1) {
		//error
	//}
	////printf("in mutex destroy\n");
	return 0;
};

