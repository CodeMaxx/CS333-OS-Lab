#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

struct barrier_t
{
	sem_t release;
	int held;
	sem_t mutex;
	int threads;
	sem_t previous;
}

barrier_init(struct barrier_t * barrier, uint nthreads)
{
	sem_init(&barrier->release, 0, 0);
	sem_init(&barrier->previous, 0, nthreads);
	sem_init(&barrier->mutex, 0, 1);
	barrier->held = 0;
	barrier->threads = nthreads;
}

void barrier_wait(struct barrier_t *barrier)
{
	sem_wait(&barrier->previous);
	sem_wait(&barrier->mutex);
	
	barrier->held++;
	if(barrier->held == barrier->threads){
		for(int i = 0; i < barrier->threads; i++){
			sem_post(&barrier->release);
		}
		int i;
		sem_getvalue(&barrier->release, &i);
	}
	sem_post(&barrier->mutex);

	sem_wait(&barrier->release);
	sem_wait(&barrier->mutex);

		barrier->held--;
		if(barrier->held == 0){
			for(int i = 0; i < barrier->threads; i++){
				sem_post(&barrier->previous);
			}
		}	
	sem_post(&barrier->mutex);
}