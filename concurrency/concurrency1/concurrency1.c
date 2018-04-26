//Use of rdrand: https://stackoverflow.com/questions/43389380/working-example-intel-rdrand-in-c-language-how-to-generate-a-float-type-number?rq=1


#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <semaphore.h>
#include <fcntl.h>
#include "concurrency1.h"
#include "linkedList.c"
#include "mt19937ar.c"

#define BUFFER_SIZE 31		//define maximum buffer size (desired size - 1)

int randFlag, bufferSize;
struct linkedList *buffer;
sem_t mutex, items, spaces;

void consumer();
void producer();
void checkSystem();
char randoms(int32_t *, int32_t, int32_t);


int main(int argc, char **argv)
{
	pthread_t consumer_pid, producer_pid;
	int seed = time(NULL);

	checkSystem();

	if(randFlag == 0) {
  	//create seed and initialize mt19937:
  	init_genrand(seed);
	}

  //testing:
  //printf("rand seed: %d\n", seed);

  //initialize buffer as linkedList:
  buffer = createLinkedList();
  bufferSize = 0;

  //initialize semaphores:
  sem_init(&mutex, 0, 1);			// buffer can only be accessed by 1 thread at a time
  sem_init(&items, 0, 0);			// no items initially, cannot consume right away
  sem_init(&spaces, 0, BUFFER_SIZE);   // limit spaces to 32

	printf("Beginning consumer-producer simulation with finite buffer size... \n  --Press Enter to stop\n");

  //create producer and consumer threads:
  pthread_create(&producer_pid, NULL, (void *)producer, NULL);
	//sem_post(&mutex);
  pthread_create(&consumer_pid, NULL, (void *)consumer, NULL);

  getchar();

  //no need, we want threads to end after main
  //pthread_join(consumer_pid, NULL);
  //pthread_join(producer_pid , NULL);

	//clean up
	freeLinkedList(buffer);
  sem_destroy(&mutex);
  sem_destroy(&items);
  sem_destroy(&spaces);


	return 0;
}


void consumer() {
	while (1) {
		//printf("CONSUMER THREAD - waiting on producer\n");
		sem_wait(&items);
		//printf("CONSUMER THREAD - waiting for buffer access\n");

    sem_wait(&mutex);
		//printf("CONSUMER THREAD - Consuming...\n");

				//event = buffer.get
        struct item event = backList(buffer);
				removeBackList(buffer);

    sem_post(&mutex);		// signal we are done with buffer
    sem_post(&spaces);	// increment number spaces, we have removed an item
		//time it takes to "consume" the product
		printf("CONSUMER THREAD --Taking %d seconds to consume value: %lu\n", event.wait, (unsigned long)event.value);
		sleep(event.wait);
		bufferSize--;
		printf("CONSUMER THREAD --BufferSize: %d--\n", bufferSize);

  }
}

void producer() {

  while(1) {

		if(bufferSize>=BUFFER_SIZE) {
			printf("**Reached max BufferSize, waiting for consumer to consume...\n");
		}

		//create the value to be pushed:
		struct item newItem;
		int produceWait;

		//use mersenne twist:
		if(randFlag == 0) {
			newItem.wait = genrand_int32() % (9 + 1 - 2) + 2; //"consume" time from 2 - 9
			newItem.value = genrand_int32();  //generate random "value" for item

			//take 3-7 seconds to "produce" an item:
			produceWait = genrand_int32() % (7 + 1 - 3) + 3;
		}

		//otherwise use rdrand:
		else if(randFlag == 1) {
			randoms(&newItem.wait, 2, 9); //"consume" time from 2 - 9
			randoms(&newItem.value, 0, UINT32_MAX);  //generate random "value" for item

			//take 3-7 seconds to "produce" an item:
			produceWait = randoms(&produceWait, 3, 7);
		}

		printf("PRODUCER THREAD - Taking %d seconds to produce an item...\n", produceWait);
		sleep(produceWait);
		printf("PRODUCER THREAD - Generated new event with value: %lu, and consume time: %d\n", (unsigned long)newItem.value, newItem.wait);


    sem_wait(&spaces);  //decrement spaces
    sem_wait(&mutex);


			addBackList(buffer, newItem);
			bufferSize++;
			printf("PRODUCER THREAD --BufferSize: %d--\n", bufferSize);

    sem_post(&mutex);
		//sleep(1);	//dont know why, but without this, consumer will never have access to buffer
    sem_post(&items);
  }
}

char randoms(int32_t *randf, int32_t min, int32_t max)
{
    int retries= 10;
    int32_t rand32;

    while(retries--) {
        if ( __builtin_ia32_rdrand32_step(&rand32) ) {
            *randf= (int32_t)rand32/ULONG_MAX*(max - min) + min;
            return 1;
        }
    }
		//testing:
		printf("rdrand produced: %lu\n", (unsigned long)randf);
    return 0;
}

void checkSystem() {
	unsigned int eax;
	unsigned int ebx;
	unsigned int ecx;
	unsigned int edx;

	char vendor[13];

  //determine whether system supports rdrand or not:
	eax = 0x01;

	__asm__ __volatile__(
	                     "cpuid;"
	                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
	                     : "a"(eax)
	                     );

	if(ecx & 0x40000000){
		//use rdrand
    randFlag = 1;
	}
	else{
		//use mt19937
    randFlag = 0;
	}
}
