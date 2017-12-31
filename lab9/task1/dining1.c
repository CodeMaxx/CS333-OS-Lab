#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define THINKING 0
#define EATING 1

void* philosopher(void*);


typedef struct philosophers_lock {
    pthread_mutex_t mutex;
    pthread_cond_t* phil;
    int* state;
} philosophers_lock;

philosophers_lock pl;
int n;

int leftNbr(i) { int j; j = i > 0 ? (i-1)%n : (i+2)%n; return j;}
int rightNbr(i) {return (i+1)%n;}

void InitializeLock(philosophers_lock* pl) {
    pl->phil = calloc(n, sizeof(pthread_cond_t));
    pl->state = calloc(n, sizeof(int));

    pthread_mutex_init(&pl->mutex, NULL);

    for(int i = 0; i < n; i++) {
        pthread_cond_init(&pl->phil[i], NULL);
        pl->state[i] = THINKING;
    }
}


bool checkFree(int i) {
    return (pl.state[leftNbr(i)] == THINKING &&
        pl.state[rightNbr(i)] == THINKING);
}

void pickUp(int i) {
    pthread_mutex_lock(&pl.mutex);
    while(!checkFree(i)) {
        pthread_cond_wait(&pl.phil[i], &pl.mutex);
    }
    pl.state[i] = EATING;
    printf("Philosopher %d eating\n", i);
    pthread_mutex_unlock(&pl.mutex);
}

void putDown(int i) {
    pthread_mutex_lock(&pl.mutex);
    pl.state[i] = THINKING;
    pthread_cond_signal(&pl.phil[rightNbr(i)]);
    pthread_cond_signal(&pl.phil[leftNbr(i)]);
    printf("Philosopher %d finished\n", i);
    pthread_mutex_unlock(&pl.mutex);
}


int main(int argc, char** argv) {
    if(argc < 2) {
        printf("Syntax: %s n\n", argv[0]);
        exit(1);
    }
    n = atoi(argv[1]);

    pthread_t* threads = calloc(n, sizeof(pthread_t));

    InitializeLock(&pl);
    srand(time(NULL));

    for(int i = 0; i < n; i++){
        int* k = malloc(sizeof(int));
        *k = i;
        pthread_create(&threads[0], NULL, philosopher, (void*)k);
    }

    for(int i = 0; i < n; i++){
        pthread_join(threads[i], NULL);
    }

    return 0;
}


void* philosopher(void* i){
    int j = *(int*)i;
    while(1) {
        // usleep(rand()%1000000 + 100000); // THINKING
        pickUp(j);
        usleep(rand()%1000000 + 1000000); // EATING
        putDown(j);
    }

    return NULL;
}