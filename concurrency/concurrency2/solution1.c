#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <semaphore.h>
#include <fcntl.h>
#include "mt19937ar.c"

sem_t footman, forks[5];

void get_forks(int*);
void put_forks(int*);
void checkSystem();
int left(int);
int right(int);


int main(int argc, char *argv[]) {
	int i, seed = time(NULL);

	pthread_t philosopher[5];
	//checkSystem();
	//use mt19937:
	init_genrand(seed);

	//set up fork semaphores and footman:
	for(i=0; i<5; i++) {
		sem_init(&forks[i], 0, 1);
	}
	sem_init(&footman, 0, 4);
	

	printf("Beginning dining philosophers simulation... \n  --Press Enter to stop\n");

	for(i=0; i<4; i++){
		pthread_create(&philosopher[i], NULL, (void *) get_forks, (void *) &i);	//grab the fork
		pthread_create(&philosopher[i], NULL, (void *) put_forks, (void *) &i);	//put fork down
	}

	getchar();

	printf("cleaning up...\n");
	//clean up:
	sem_destroy(&footman);
	for(i=0; i<4; i++){
		sem_destroy(&forks[i]);
	}


	return 0;
}

/*
footman . wait ()
fork [ right (i )]. wait ()
fork [ left ( i )]. wait ()
*/
void get_forks(int *i) {
	while(1) {
		int eat_time;
	
		printf("Philosopher %d is getting fork\n", *i);
		sem_wait(&footman);
		sem_wait(&forks[right(*i)]);
		sem_wait(&forks[left(*i)]);
	
		//simulate eating:
		printf("Philosopher %d taking %d seconds to eat\n", *i, eat_time);
		eat_time = genrand_int32() % (9 + 1 - 2) + 2; //"eating" time from 2 - 9
		sleep(eat_time);
	}
}

/*
fork [ right (i )]. signal ()
fork [ left ( i )]. signal ()
footman . signal ()

*/

void put_forks(int *i){
	while(1) {
		int think_time;
	
		printf("Philosopher %d is putting fork down\n", *i);
		sem_post(&forks[right(*i)]);
		sem_post(&forks[left(*i)]);
		sem_post(&footman);
	
		//simulate thinking:
		think_time = genrand_int32() % (20 + 1 - 1) + 1; //"thinking" time from 1 - 20
		printf("Philosopher %d will think for %d seconds\n", *i, think_time);
		sleep(think_time);
	}
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
    //randFlag = 1;
        }
        else{
                //use mt19937
    //randFlag = 0;
        }
}


int left(int i) {
	return i;
}

int right(int i) {
	return ( i + 1) % 5;
}