#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include "concurrency1.h"
#include "linkedList.c"
#include "mt19937ar.c"

int randFlag, bufferSize;
struct linkedList *buffer;
sem_t mutex, items, spaces;

void consumer();
void producer();
double genSeed();


int main(int argc, char **argv)
{

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

    //create seed and initialize mt19937:
		double mySeed = genSeed();
    init_genrand(mySeed);

    //testing:
    printf("rand seed: %lf\n", mySeed);
	}

  //initialize buffer as linkedList:
  buffer = createLinkedList();
  bufferSize = 0;

  pthread_t consumer_pid, producer_pid;

  //create producer and consumer threads:

  //initialize semaphores:
  sem_init(&mutex, 0, 1);
  sem_init(&items, 0, 1);
  sem_init(&spaces, 0, 1);

  pthread_create(&consumer_pid, NULL, (void *)consumer, NULL);
  pthread_create(&producer_pid, NULL, (void *)producer, NULL);

  pthread_join(consumer_pid, NULL);
  pthread_join(producer_pid , NULL);

	return 0;
}


void consumer() {
  sem_wait(&items);
  sem_wait(&mutex);
    struct item event = frontList(buffer);
  sem_post(&mutex);
  sem_post(&spaces);
  printf("Consumed value: %d\n", event.value);
}

void producer() {
  //wait for event
  sem_wait(&spaces);
  sem_wait(&mutex);

    //create the value to be pushed:
    struct item newItem;
    newItem.value = genrand_real1();
    newItem.wait = genrand_real1();

    //testing:
    printf("value: %d\n", newItem.value);
    printf("wait: %d\n", newItem.wait);


    addBackList(buffer, newItem);

  sem_post(&mutex);
  sem_post(&items);
}

double genSeed() {
	double seed;
  FILE* f;

  f = fopen("/dev/random", "r");
  if(f == NULL){
    fprintf(stderr, "WARNING: Failed to open /dev/random. Random seed defaults to 1. \n");
    return 1;
  }

  fread(&seed, sizeof(double), 1, f);
  fclose(f);

	return seed;
}
