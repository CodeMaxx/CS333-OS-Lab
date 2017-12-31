#include<stdio.h>
#include<stdlib.h>
#include<semaphore.h>
#include<pthread.h>

void* printme(void*);
sem_t sem1, sem2;
int main() {
    sem_init(&sem1, 0, 1);
    sem_init(&sem2, 0, 0);

    int index[] = {0, 1};
    pthread_t threads[2];

    pthread_create(&threads[0], NULL, printme, (void*)&index[0]);
    pthread_create(&threads[1], NULL, printme, (void*)&index[1]);
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
    sem_destroy(&sem1);
    sem_destroy(&sem2);
}

void* printme(void* i) {
    int j = *(int*)i;

    if(j == 0){
        for(int i = 0; i < 10; i++) {
            sem_wait(&sem1);
            printf("Hello I am the THREAD %d!\n", j);
            sem_post(&sem2);
        }
    }
    else
    {
        for(int i = 0; i < 10; i++) {
            sem_wait(&sem2);
            printf("Hello I am the THREAD %d!\n", j);
            sem_post(&sem1);
        }
    }
}