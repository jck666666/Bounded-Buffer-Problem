#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <semaphore.h>  /* Semaphore */
#define SIZE 1024



// semaphore declare
// 1. mutual exclusion
sem_t mutex_maxbuffer;
sem_t mutex_minbuffer;

// 2. Synchronous
sem_t sem_maxbuffer;
sem_t sem_minbuffer; 

// big_buffer
int big_buffer[SIZE];

// max_buffer, min_buffer and index for them
int max_buffer[4];
int min_buffer[4];
int index_max = 0;
int index_min = 0;


// 最後的答案
int maximum = 0;
int minimum = 2147483647;

void show(int* arr, int begin, int end)
{

	for(int i = begin ; i < end ; i++)
	{
		printf("%d ", arr[i]);
	}
	printf("\n");
}

typedef struct producer_info{
	int begin_index;
	int end_index;
}producer_info;



void* producer(void* arg)
{	
	producer_info *data = (producer_info*)arg;
	int begin = data->begin_index;
	int end = data->end_index;
	int max = 0;
	int min = 2147483647;

	for(int i = begin ; i < end ; i++)
	{
		int item = big_buffer[i];
		if(item > max)
		{
			max = item;
		}
		if(item < min)
		{
			min = item;
		}
	}
	printf("Producer: Temporary max=%d and min=%d \n", max, min);

	//----------critial section for max_buffer------------------------
	sem_wait(&mutex_maxbuffer); 
	max_buffer[index_max] = max;
	printf("Producer: Put %d into max_buffer at %d \n", max, index_max);
	index_max++;
	sem_post(&mutex_maxbuffer); 
	sem_post(&sem_maxbuffer);
	//-----------------------------------------------------------------

	//----------critial section for min_buffer------------------------
	sem_wait(&mutex_minbuffer); 
	min_buffer[index_min] = min;
	printf("Producer: Put %d into min_buffer at %d \n", min, index_min);
	index_min++;
	sem_post(&mutex_minbuffer); 
	sem_post(&sem_minbuffer);
	//-----------------------------------------------------------------
	pthread_exit(NULL);
}

void* consumer1(void* arg)
{
	for(int i = 0 ; i < 4 ; i++)
	{
		//----------critial section for max_buffer------------------------
		sem_wait(&sem_maxbuffer); 
		sem_wait(&mutex_maxbuffer);
		int temp = max_buffer[i];
		if(temp > maximum)
		{
			maximum = temp;
			printf("Consumer: Updated! maximum=%d \n", maximum);
		}
		sem_post(&mutex_maxbuffer); 
		//-----------------------------------------------------------------
	}
	pthread_exit(NULL);
}

void* consumer2(void* arg)
{
	for(int i = 0; i < 4; i++)
	{
		//----------critial section for min_buffer------------------------
		sem_wait(&sem_minbuffer);  
		sem_wait(&mutex_minbuffer);
		int temp = min_buffer[i];
		if(temp < minimum)
		{
			minimum = temp;
			printf("Consumer: Updated! minimum=%d \n", minimum);
		}
		sem_post(&mutex_minbuffer); 
		//-----------------------------------------------------------------
	}
	pthread_exit(NULL);
}


void main()
{
	// random initialize big buffer
	for(int i = 0 ; i < SIZE ; i++)
	{
		big_buffer[i] = rand();
	}

	// create Producer and consu
	producer_info data1 = {0, SIZE/4};
	producer_info data2 = {SIZE/4, 2 * (SIZE / 4)};
	producer_info data3 = {2 * (SIZE / 4), 3 * (SIZE/4)};
	producer_info data4 = {3 * (SIZE / 4), SIZE};


	// sempahore initialize
	sem_init(&mutex_maxbuffer, 0, 1);
	sem_init(&mutex_minbuffer, 0, 1);
	sem_init(&sem_maxbuffer, 0, 0);
	sem_init(&sem_minbuffer, 0, 0);

	// init pthread id
	pthread_t p1, p2, p3, p4, c1, c2;

	// pthread exec
	pthread_create(&p1, NULL, producer, (void*) &data1);
	pthread_create(&p4, NULL, producer, (void*) &data4);
	pthread_create(&p2, NULL, producer, (void*) &data2);
	pthread_create(&p3, NULL, producer, (void*) &data3);
	pthread_create(&c1, NULL, consumer1, (void*) NULL);
	pthread_create(&c2, NULL, consumer2, (void*) NULL);

	// wait for each thread
	pthread_join(p1, NULL);
	pthread_join(p2, NULL);
	pthread_join(p3, NULL);
	pthread_join(p4, NULL);
	pthread_join(c1, NULL);
	pthread_join(c2, NULL);

	printf("Success! maximum= %d and minimum= %d \n", maximum, minimum);
}