
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

long int reader_loop=1;
long int writer_loop=1;

struct read_write_lock
{
	pthread_mutex_t mutex;
	pthread_cond_t readerQ;
	pthread_cond_t writerQ;
	int num_waiting_readers;
	int num_waiting_writers;
	int writing;
	int reading;
}rwlock;

struct argument_t
{
	long int threadNum;
	int delay;
};

long int data=0;			//	Shared data variable

void InitalizeReadWriteLock(struct read_write_lock * rw)
{
	rw->num_waiting_readers = 0;
	rw->num_waiting_writers = 0;
	rw->writing = 0;
	rw->reading = 0;
	pthread_cond_init(&rw->readerQ, NULL);
	pthread_cond_init(&rw->writerQ, NULL);
	pthread_mutex_init(&rw->mutex, NULL);	
}

void ReaderLock(struct read_write_lock * rw)
{
	printf("Reader Waiting!\n");
	pthread_mutex_lock(&rw->mutex);
	rw->num_waiting_readers++;
	while(rw->writing || rw->num_waiting_writers){
		pthread_cond_wait(&rw->readerQ, &rw->mutex);
	}
	rw->num_waiting_readers--;
	rw->reading++;
	printf("Reader Entered!\n");
	pthread_mutex_unlock(&rw->mutex);
}

void ReaderUnlock(struct read_write_lock * rw)
{
	pthread_mutex_lock(&rw->mutex);
	printf("Reader Exited!\n");
	rw->reading--;
	if(rw->num_waiting_writers){
		pthread_cond_signal(&rw->writerQ);
	}
	else{
		pthread_cond_signal(&rw->readerQ);
	}
	pthread_mutex_unlock(&rw->mutex);
}

void WriterLock(struct read_write_lock * rw)
{
	printf("Writer Waiting!\n");
	pthread_mutex_lock(&rw->mutex);
	rw->num_waiting_writers++;
	while(rw->writing || rw->reading) {
		pthread_cond_wait(&rw->writerQ, &rw->mutex);
	}
	rw->num_waiting_writers--;
	rw->writing++;
	printf("Writer Entered!\n");
	pthread_mutex_unlock(&rw->mutex);
}

void WriterUnlock(struct read_write_lock * rw)
{
	pthread_mutex_lock(&rw->mutex);
	printf("Writer Exited!\n");
	rw->writing--;
	if(rw->num_waiting_writers){
		pthread_cond_signal(&rw->writerQ);
	}
	else{
		pthread_cond_signal(&rw->readerQ);
	}
	pthread_mutex_unlock(&rw->mutex);
}

void *ReaderFunction(void *args)
{
	struct argument_t *arg;
	int threadNum;
	int delay;
	struct timespec wait;

	arg=(struct argument_t *)args;
	threadNum=arg->threadNum;
	delay=arg->delay;

	wait.tv_sec=delay/1000;
	wait.tv_nsec=(delay%1000)*1000*1000;
	nanosleep(&wait,NULL);

	for(int i=0;i<reader_loop;i++)
	{
		//	Acquiring lock
		ReaderLock(&rwlock);
				
		printf("Reader %2d, value: %ld\n",threadNum, data);
		usleep(10);
				
		// Release lock

		ReaderUnlock(&rwlock);
	}

	free(arg);
}

void *WriterFunction(void *args)
{
	struct argument_t *arg;
	int threadNum;
	int delay;    
	struct timespec wait;


	arg=(struct argument_t *)args;
	threadNum=arg->threadNum;
	delay=arg->delay;

	wait.tv_sec=delay/1000;
	wait.tv_nsec=(delay%1000)*1000*1000;
	nanosleep(&wait,NULL);

	for(int i=0;i<writer_loop;i++)
	{
		//	Acquiring lock
		WriterLock(&rwlock);
		
		data++;
		printf("Writer %2d, value: %ld\n",threadNum, data);
		usleep(10);
			
		// Release lock
		WriterUnlock(&rwlock);

	}
	
	free(arg);
}

int main(int argc, char *argv[])
{
	pthread_t *writer_threads;
	pthread_t *reader_threads;
	long int reader_count=0;
	long int writer_count=0;
	long int reader_number=0;
	long int writer_number=0;
	time_t start, stop;

	//	Verifying number of arguments
	if(argc<4 || (argc-1)%3!=0)
	{
		printf("reader-writer <r/w> <no-of-threds> <delay in ms> ...\n");
		exit(1);
	}

	//	Verifying inputs, and  counting total readers and writers
	for(int i=0; i<(argc-1)/3;i++)
	{
		char rw[2];
		strcpy(rw, argv[(i*3)+1]);

		if(strcmp(rw,"r")==0)
		{
			long int count=atol(argv[(i*3)+2]);
			reader_count+=count;
		}
		else if(strcmp(rw,"w")==0)
		{
			long int count=atol(argv[(i*3)+2]);
			writer_count+=count;
		}
		else
		{
			printf("reader-writer <r/w> <no-of-threds> <delay in ms> ...\n");
			exit(1);
		}
	}
	
	// create reader, writer thread variables
	writer_threads=(pthread_t *)malloc(sizeof(pthread_t)*writer_count);
	reader_threads=(pthread_t *)malloc(sizeof(pthread_t)*reader_count);

	int readin = 0;
	int writein = 0;
	//	Initlizing and rwlock;
	InitalizeReadWriteLock(&rwlock);
	// int* indexes[(argc-1)/3];
	time(&start);
	for(int i=0; i<(argc-1)/3;i++)
	{
		struct argument_t *arg;
		char rw[2];
		long int thread_count;
		int delay;

		strcpy(rw, argv[(i*3)+1]);
		thread_count=atol(argv[(i*3)+2]);
		delay=atoi(argv[(i*3)+3]);

		// indexes[i] = calloc(thread_count, sizeof(int));

		for(long int j=0;j<thread_count;j++)
		{
			arg =(struct argument_t *) malloc(sizeof(struct argument_t));
  		    arg->delay=delay;
			
			if(strcmp(rw,"r")==0)   // 
			{
				
				// indexes[i][j] = j;
				arg->threadNum = readin;
				pthread_create(&reader_threads[readin], NULL, ReaderFunction, (void*)arg);
				readin++;
			// add unique thread identifier/number to arg and pass arg as argument during thread creation
			// create reader thread here
			}

			else
			{
				// indexes[i][j] = j;
				arg->threadNum = writein;
				pthread_create(&writer_threads[writein], NULL, WriterFunction, (void*)arg);
				writein++;
            // add unique thread identifier/number to arg and pass arg as argument during thread creation
			// create writer thread here
			}
		}
	}

	for(int i=0;i<reader_count; i++)
		pthread_join(reader_threads[i], NULL);
		// wait for reader threads to complete 

	for(int i=0;i<writer_count; i++)
		pthread_join(writer_threads[i], NULL);
		// wait for writer threads to complete 

    free(writer_threads);
    free(reader_threads);
    
	time(&stop);
   	printf("Finished in about %.0f seconds. \n", difftime(stop, start));

	exit(1);
}
