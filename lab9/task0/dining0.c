#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define DOWN 0
#define UP 1

void* philosopher(void*);


typedef struct philosophers_lock {
    pthread_mutex_t mutex;
    pthread_cond_t* forks;
    int* state;
} philosophers_lock;

philosophers_lock pl;
int n;

int leftFork(i) { int j; j = i > 0 ? (i-1)%n : (i+n-1)%n; return j;}
int rightFork(i) {return i%n;}

void InitializeLock(philosophers_lock* pl) {
    pl->forks = calloc(n, sizeof(pthread_cond_t));
    pl->state = calloc(n, sizeof(int));

    pthread_mutex_init(&pl->mutex, NULL);

    for(int i = 0; i < n; i++) {
        pthread_cond_init(&pl->forks[i], NULL);
        pl->state[i] = DOWN;
    }
}


void pickUp(int i) {
    pthread_mutex_lock(&pl.mutex);
    while(pl.state[leftFork(i)] == UP)
        pthread_cond_wait(&pl.forks[leftFork(i)], &pl.mutex);
    pl.state[leftFork(i)] = UP;
    printf("Philosopher %d picked up left fork\n", i);
    pthread_mutex_unlock(&pl.mutex);


    pthread_mutex_lock(&pl.mutex);
    while(pl.state[rightFork(i)] == UP)
        pthread_cond_wait(&pl.forks[i], &pl.mutex);
    pl.state[rightFork(i)] = UP;
    printf("Philosopher %d picked up right fork\n", i);
    printf("Philosopher %d eating\n", i);
    pthread_mutex_unlock(&pl.mutex);
}

void putDown(int i) {
    pthread_mutex_lock(&pl.mutex);
    pl.state[leftFork(i)] = DOWN;
    pl.state[rightFork(i)] = DOWN;
    pthread_cond_signal(&pl.forks[rightFork(i)]);
    pthread_cond_signal(&pl.forks[leftFork(i)]);
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
        // usleep(rand()%1000000 + 1000000); // THINKING
        pickUp(j);
        usleep(rand()%1000000 + 1000000); //  EATING
        putDown(j);
    }

    return NULL;
}