#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

long int reader_count;
long int writer_count;

long int reader_loop;
long int writer_loop;

int rwlock_enabled=1;

struct read_write_lock
{
	pthread_mutex_t mutex;
	pthread_cond_t readers;
	pthread_cond_t writers;
	int wait_writer_num;
	int reading;
};

struct read_write_lock rwlock;
pthread_spinlock_t spinlock;

long int data=0;			//	Shared data variable

void InitalizeReadWriteLock(struct read_write_lock * rw)
{
	rw->wait_writer_num = 0;
	rw->reading = 0;
	pthread_mutex_init(&rw->mutex, NULL);
	pthread_cond_init(&rw->readers, NULL);
	pthread_cond_init(&rw->writers, NULL);
	//	Write the code for initializing your read-write lock.
}

void ReaderLock(struct read_write_lock * rw)
{
	pthread_mutex_lock(&rw->mutex);
	while(rw->wait_writer_num != 0)
		pthread_cond_wait(&rw->readers, &rw->mutex);
	rw->reading++;
	pthread_mutex_unlock(&rw->mutex);
	//	Write the code for aquiring read-write lock by the reader.
}

void ReaderUnlock(struct read_write_lock * rw)
{
	pthread_mutex_lock(&rw->mutex);
	rw->reading--;
	pthread_mutex_unlock(&rw->mutex);
	pthread_cond_signal(&rw->writers);
	//	Write the code for releasing read-write lock by the reader.
}

void WriterLock(struct read_write_lock * rw)
{
	pthread_mutex_lock(&rw->mutex);
	while(rw->reading !=0 || rw->wait_writer_num != 0)
		pthread_cond_wait(&rw->writers, &rw->mutex);
	rw->wait_writer_num++;
	pthread_mutex_unlock(&rw->mutex);
	//	Write the code for aquiring read-write lock by the writer.
}

void WriterUnlock(struct read_write_lock * rw)
{
	pthread_mutex_lock(&rw->mutex);
	rw->wait_writer_num--;
	pthread_mutex_unlock(&rw->mutex);
	pthread_cond_signal(&rw->writers);
	pthread_cond_signal(&rw->readers);
	//	Write the code for releasing read-write lock by the writer.
}




void *ReaderFunction(void *arg)
{
	int threadNUmber = *((int *)arg);

	for(int i=0;i<reader_loop;i++)
	{
		//	Acquiring lock
		if(rwlock_enabled)
			ReaderLock(&rwlock);
		else
			pthread_spin_lock(&spinlock);

		printf("Reader: %2d, value: %ld\n",threadNUmber, data);

		// Release lock
		if(rwlock_enabled)
			ReaderUnlock(&rwlock);
		else
			pthread_spin_unlock(&spinlock);

		usleep(900);

	}
}

void *WriterFunction(void *arg)
{

	int threadNUmber = *((int *)arg);

	for(int i=0;i<writer_loop;i++)
	{
		//	Acquiring lock
		if(rwlock_enabled)
			WriterLock(&rwlock);
		else
			pthread_spin_lock(&spinlock);

		data++;
		printf("Writer: %2d, value: %ld\n",threadNUmber, data);

		// Release lock
		if(rwlock_enabled)
			WriterUnlock(&rwlock);
		else
			pthread_spin_unlock(&spinlock);

		usleep(900);
	}

}

int main(int argc, char *argv[])
{
	pthread_t *writer_threads;
	pthread_t *reader_threads;
	int *threadNUmber;

	struct args *arg;
	time_t start, stop;

	if(argc<6)
	{
		printf("read-write <readers-count> <writers-count> <reader-loop-size> <writer-loop-size> <enable/disable rwlock>\n");
		exit(1);
	}

	reader_count=atol(argv[1]);
	writer_count=atol(argv[2]);

	reader_loop=atol(argv[3]);
	writer_loop=atol(argv[4]);

	rwlock_enabled=atoi(argv[5]);

	writer_threads = calloc(writer_count, sizeof(pthread_t));
	reader_threads = calloc(reader_count, sizeof(pthread_t));

	int* reader_index = calloc(reader_count, sizeof(int));
	int* writer_index = calloc(writer_count, sizeof(int));

	//	Initlizing spin lock and rwlock;
	pthread_spin_init(&spinlock,0);   //spinlock used only if rwlock is disabled

	InitalizeReadWriteLock(&rwlock);

	time(&start);

	for(int i=0;i<writer_count; i++)
	{
		writer_index[i] = i;
		pthread_create(&writer_threads[i], NULL, WriterFunction, (void *)&writer_index[i]);
		//	Create a writer thread.
		//	use writer_threads array to map pthread_t variable for each thread
		//	writer thread number needs to be send to the writer function
	}

	for(int i=0;i<reader_count; i++)
	{
		reader_index[i] = i;
		pthread_create(&reader_threads[i], NULL, ReaderFunction, (void*)&reader_index[i]);
		//	Create a reader thread.
		//	use reader_threads array to map pthread_t variable for each thread
		//	reader thread number needs to be send to the reader function
	}

	for(int i=0;i<writer_count; i++)
		pthread_join(writer_threads[i],NULL);

	for(int i=0;i<reader_count; i++)
		pthread_join(reader_threads[i],NULL);

	free(writer_threads);
	free(reader_threads);

	time(&stop);
   	printf("Finished in about %.0f seconds. \n", difftime(stop, start));

	exit(1);
}
