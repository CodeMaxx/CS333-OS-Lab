#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int items[10];
pthread_spinlock_t locks[10];

void* add(void* i);

int main() {
    int val[10];

    for(int i = 0; i < 10; i++){
        items[i] = 0;
        pthread_spin_init(&locks[i], PTHREAD_PROCESS_PRIVATE);
        val[i] = i;
    }

    pthread_t threads[10];
    int irets[10];

    for(int i = 0; i < 10; i++) {
        irets[i] = pthread_create( &threads[i], NULL, add, (void*)&val[i]);
        if(irets[i])
        {
            fprintf(stderr,"Error - pthread_create() return code: %d\n",irets[i]);
            exit(EXIT_FAILURE);
        }
    }

    for(int i = 0; i < 1000; i++){
        for(int k = 0; k < 10; k++){
           pthread_spin_lock(&locks[k]);
           items[k]++;
           pthread_spin_unlock(&locks[k]);
        }
    }

    for(int i = 0; i < 10; i++) {
        pthread_join(threads[i], NULL);
        pthread_spin_destroy(&locks[i]);
    }

    for(int i = 0; i < 10; i++) {
        printf("%d: %d\n", i, items[i]);
    }

    exit(EXIT_SUCCESS);

}

void* add(void* i) {
    int j = *(int*)i;
    for(int k = 0; k < 1000; k++){
       pthread_spin_lock(&locks[j]);
       items[j]++;
       pthread_spin_unlock(&locks[j]);
    }
}