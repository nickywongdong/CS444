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
#include <time.h>
#include "mt19937ar.c"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


sem_t footman, forks[5];
time_t rawtime;
struct tm * timeinfo;

void get_forks(void*);
void put_forks(void*);
void checkSystem();
int left(int);
int right(int);


int main(int argc, char *argv[]) {
	int i, seed = time(NULL);

	pthread_t philosopher[5];
	//checkSystem();
	//use mt19937:
	init_genrand(seed);

	/*void (*get_forks_functions[5])(int *);
	void (*put_forks_functions[5])(int *);

	for(i=0; i<4; i++){
		get_forks_functions[i] = &get_forks;
		put_forks_functions[i] = &put_forks;
	}*/

	//set up fork semaphores and footman:
	for(i=0; i<4; i++) {
		sem_init(&forks[i], 0, 1);
	}
	sem_init(&footman, 0, 4);
	

	printf("Beginning dining philosophers simulation... \n  --Press Enter to stop\n");

	for(i=0; i<5; i++){
		int *index = malloc(sizeof(*index));
		*index = i+1;
		pthread_create(&philosopher[i], NULL, (void *) get_forks, index);	//grab the fork
		pthread_create(&philosopher[i], NULL, (void *) put_forks, index);	//put fork down
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
void get_forks(void *index) {
	int i = *((int *) index);
	while(1) {

		int think_time, eat_time;

		//simulate thinking:
		think_time = genrand_int32() % (20 + 1 - 1) + 1; //"thinking" time from 1 - 20

		time (&rawtime);
  		timeinfo = localtime (&rawtime);
		printf("%s -- " ANSI_COLOR_MAGENTA "Philosopher %d will think for %d seconds\n" ANSI_COLOR_RESET, strtok(asctime(timeinfo), "\n"), i, think_time);
		sleep(think_time);

 	 	time (&rawtime);
  		timeinfo = localtime (&rawtime);
		printf("%s -- " ANSI_COLOR_CYAN "Philosopher %d is done thinking\n" ANSI_COLOR_RESET, strtok(asctime(timeinfo), "\n"), i);
		

		sem_wait(&footman);
		sem_wait(&forks[right(i)]);
		sem_wait(&forks[left(i)]);	//must have both forks to eat

		time (&rawtime);
  		timeinfo = localtime (&rawtime);
		printf("%s -- " ANSI_COLOR_GREEN "Philosopher %d is getting forks\n" ANSI_COLOR_RESET, strtok(asctime(timeinfo), "\n"), i);

		//simulate eating:
		eat_time = genrand_int32() % (9 + 1 - 2) + 2; //"eating" time from 2 - 9

  		time (&rawtime);
  		timeinfo = localtime (&rawtime);
  		printf("%s -- " ANSI_COLOR_YELLOW "Philosopher %d taking %d seconds to eat\n" ANSI_COLOR_RESET, strtok(asctime(timeinfo), "\n"), i, eat_time);
		sleep(eat_time);

  		time (&rawtime);
  		timeinfo = localtime (&rawtime);
		printf("%s -- " ANSI_COLOR_BLUE "Philosopher %d is done eating, putting fork down\n" ANSI_COLOR_RESET , strtok(asctime(timeinfo), "\n"), i);
	
	}
}

/*
fork [ right (i )]. signal ()
fork [ left ( i )]. signal ()
footman . signal ()

*/

void put_forks(void *index){
	int i = *((int *) index);
	while(1) {

		sem_post(&forks[right(i)]);
		sem_post(&forks[left(i)]);
		sem_post(&footman);
	
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
	i=i-1;
	return i;
}

int right(int i) {
	i=i-1;
	return ( i + 1) % 5;
}