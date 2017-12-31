#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


#define BSIZE 10
#define PAUSE_COUNT 20


// the buffer structure that holds the producer-consumer buffer
// and other variables related to the producers and consumers
typedef struct {
    int buf[BSIZE];
    
    int prodIndex;
    int consIndex;
    int itemCount;

    pthread_cond_t producers; // producers sleep on this
    pthread_cond_t consumers; // consumers sleep on this

    pthread_mutex_t mutex; // Mutex for buffer_t
    // Add your own variables/locks DONE

} buffer_t;

typedef struct {
	long counter1;
	long counter2;
	long result;
} gpdata_t;

typedef struct {
	long counter1;
	long counter2;
	long result;
} gcdata_t;


int prod_wait = 1;
int cons_wait = 1;
int exec_count = 10;
int consumer_count;
int producer_count;
int cons_exec_count = 10, prod_exec_count = 10;

buffer_t *b;
gpdata_t *gpdata;
gcdata_t *gcdata;

// producers call this function to produce in to the producer-consumer buffer.
// NOT TO BE UPDATED, only called.
// t: thread number
// i: index in buffer for producion
// this function is part of the critical section
void produce(int t, int i) {

	if (gpdata->result != gpdata->counter1 + gpdata->counter2) {
		printf("Something is Wrong! Illegal Access\n");
	}

	if (gcdata->result != gcdata->counter1 + gcdata->counter2) {
		printf("Something is Wrong! Illegal Access\n");
	}

	if (gcdata->counter1 > gpdata->counter1) {
		printf("Something is Wrong! Bad Consume\n");
	}

	if (gpdata->counter1 - gcdata->counter1 > BSIZE) {
		printf("Something is Wrong! Bad Produce\n");
	}

	b->buf[i] = i;
	int pctr = PAUSE_COUNT;

    gpdata->counter1++;

    while(pctr--)   /*Spend some cpu cycles w/o performing anything useful*/
 		usleep(100);
 
    gpdata->counter2++;
    
    pctr = PAUSE_COUNT;           
        
    while(pctr--)  /*Spend some cpu cycles w/o performing anything useful*/
		usleep(100);

    gpdata->result = gpdata->counter1 + gpdata->counter2;
	printf("P %d %d\n",t, i);
}

// consumers call this function to consume from the producer-consumer buffer.
// NOT TO BE UPDATED, only called.
// t: thread number
// i: index in buffer for consumption
// this function is part of the critical section
void consume(int t, int i) {

	if (gpdata->result != gpdata->counter1 + gpdata->counter2) {
		printf("Something is Wrong! Illegal Access\n");
	}

	if (gcdata->result != gcdata->counter1 + gcdata->counter2) {
		printf("Something is Wrong! Illegal Access\n");
	}

	if (gcdata->counter1 > gpdata->counter1) {
		printf("Something is Wrong! Bad Consume\n");
	}

	if (gpdata->counter1 - gcdata->counter1 > BSIZE) {
		printf("Something is Wrong! Bad Produce\n");
	}

	int pctr = PAUSE_COUNT;

    gcdata->counter1++;

    while(pctr--)   /*Spend some cpu cycles w/o performing anything useful*/
 		usleep(100);
 
    gcdata->counter2++;
    
    pctr = PAUSE_COUNT;           
        
    while(pctr--)  /*Spend some cpu cycles w/o performing anything useful*/
		usleep(100);

    gcdata->result = gcdata->counter1 + gcdata->counter2;
	printf("C %d %d\n",t, i);
}


// this is the producer thread function
// used to initialize the producer threads 
void *producer(void * arg)
{
	int threadNumber = *((int *)arg);

	int pTotalCount = prod_exec_count;

	while(pTotalCount--) {
		usleep(prod_wait * 1000000);
		// DONE
	// setup synchronization
		pthread_mutex_lock(&b->mutex);
	// crictical section begins
		while(b->itemCount >= BSIZE) {
			pthread_cond_wait(&b->producers, &b->mutex);
		}
        // tasks that require synchronized access
	// one of them is to produce
		
        produce(threadNumber, b->prodIndex); // pass index in buffer
       	 b->itemCount ++;
		b->prodIndex ++;
		b->prodIndex %= BSIZE;
        //
        pthread_cond_signal(&b->consumers);
	// critical section ends

	// release synchronization
        pthread_mutex_unlock(&b->mutex);
	}
    
}


// this is the consumer thread function
// used to initialize the consumer threads 
void *consumer(void * arg)
{
	int threadNumber = *((int *)arg);

	int cTotalCount = cons_exec_count;

	while(cTotalCount--) {
		usleep(cons_wait * 1000000);

	// DONE
	// setup synchronization
		pthread_mutex_lock(&b->mutex);
	// crictical section begins
       	while(b->itemCount <= 0) {
			pthread_cond_wait(&b->consumers, &b->mutex);
		}

        // tasks that require synchronized access
        
		
	// one of them is to consumer
		// b->itemCount --;
        consume(threadNumber, b->consIndex); // pass index in buffer
        b->itemCount --;
        b->consIndex ++;
		b->consIndex %= BSIZE;
        pthread_cond_signal(&b->producers);
        pthread_mutex_unlock(&b->mutex);
	// critical section ends
	// release synchronization
	}
}


int main(int argc, char *argv[]) {
	pthread_t *consumer_threads;
	pthread_t *producer_threads;
	int *threadNumber;

	time_t start, stop;

	if(argc<3)
	{
printf("usage: ./a.out  <#consumers> <#producers> <consumption-gap> <production-gap> <consumption-per-consumer> <production-per-producer>\n\n \
               <#consumers>: is number of consumers (consumer threads) \n \
               <#producers>: is number of producers (producerthreads) \n \
         <#consumption-gap>: delay between two consume events by each consumer \n \
          <#production-gap>: delay between two production events by each producer \n \
<#consumption-per-consumer>: number of items consumer per consumer \n \
 <#production-per-producer>: number of items produced per produver\n\n");

	exit(1);
	}

	// consumer and producer counts
	consumer_count=atoi(argv[1]);
	producer_count=atoi(argv[2]);

	// delay between consecutive consumption and production by each consumer/producer
        // e.g., for each consumer: ... consume ... cons_wait ... consume ... etc.
	if (argc >= 5) {
		cons_wait = atoi(argv[3]);
		prod_wait = atoi(argv[4]);
	}

	// number of events per consumer and producer
	
	if (argc == 6) {
		cons_exec_count = atoi(argv[5]);
	}

	if (argc == 7) {
		cons_exec_count = atoi(argv[5]);
		prod_exec_count = atoi(argv[6]);
	}
	
	// DONE initialise locks
	// this setup and test code, do no update
	b = (buffer_t *)malloc(sizeof(buffer_t));
	pthread_mutex_init(&b->mutex, NULL);
    pthread_cond_init(&b->producers, NULL);
    pthread_cond_init(&b->consumers, NULL);
	b->prodIndex = 0;
	b->consIndex = 0;
	b->itemCount = 0;

	gpdata = (gpdata_t *)malloc(sizeof(gpdata_t));
	gpdata->counter1 = 0;
	gpdata->counter2 = 17;
	gpdata->result = 17;
	gcdata = (gcdata_t *)malloc(sizeof(gcdata_t));
	gcdata->counter1 = 0;
	gcdata->counter2 = 13;
	gcdata->result = 13;

	time(&start);
	// DONE
	// end setup and test code
	consumer_threads = calloc(consumer_count, sizeof(pthread_t));
	producer_threads = calloc(producer_count, sizeof(pthread_t));

	for(int i = 0; i < consumer_count; i++) {
		threadNumber = malloc(sizeof(int));
		*threadNumber = i;
		pthread_create(&consumer_threads[i], NULL, consumer, (void*)threadNumber); // Doubt
	}

	for(int i = 0; i < producer_count; i++) {
		threadNumber = malloc(sizeof(int));
		*threadNumber = i;
		pthread_create(&producer_threads[i], NULL, producer, (void*)threadNumber);
	}
        // create consumer and producer threads. 
        // each thread needs a thread number, consumer threads 0 to N-1 and same for producers
        // check sample outputs

	for(int i = 0; i < consumer_count; i++) {
		pthread_join(consumer_threads[i], NULL);
	}

	for(int i = 0; i < producer_count; i++) {
		pthread_join(producer_threads[i], NULL);
	}

	// wait for threads to complete


	if ((prod_exec_count*producer_count) == (cons_exec_count*consumer_count))
 		if (gpdata->counter1 != gcdata->counter1) {
		printf("Something is Wrong! Bad Produce Consume\n");
	}

	time(&stop);
   	printf("Items produced: %ld, items consumed: %ld \nFinished in about %.0f seconds. \n", gpdata->counter1, gcdata->counter1, difftime(stop, start));

	exit(1);

}
