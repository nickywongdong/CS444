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


sem_t footman, forks[5], state[5];
time_t rawtime;
struct tm * timeinfo;
int philosopher_status[5], forks_status[5], philosopher_sleep_time[5];
char philosophers[5][128];

void get_forks(void*);
void put_forks(void*);
void philosopher_thread(void*);
void checkSystem();
void print_table();
int left(int);
int right(int);


int main(int argc, char *argv[]) {
	int i, seed = time(NULL);

	pthread_t philosopher[5];
	//checkSystem();
	//use mt19937:
	init_genrand(seed);

	//set up fork semaphores and footman:
	for(i=0; i<4; i++) {
		sem_init(&state[i], 0, 0);
		sem_init(&forks[i], 0, 1);
	}
	sem_init(&footman, 0, 4);

	strcpy(philosophers[0], "Kevin");
	strcpy(philosophers[1], "Socrates");
	strcpy(philosophers[2], "Descartes");
	strcpy(philosophers[3], "Plato");
	strcpy(philosophers[4], "Aristotle");
	

	printf("Beginning dining philosophers simulation... \n  --Press Enter to stop\n");

	for(i=0; i<5; i++){
		int *index = malloc(sizeof(*index));
		*index = i;
		pthread_create(&philosopher[i], NULL, (void *) philosopher_thread, index); //begin thread for each philosopher
	}

	while(1) {
		print_table();
	}
	//getchar();

	printf("cleaning up...\n");
	//clean up:
	sem_destroy(&footman);
	for(i=0; i<4; i++){
		sem_destroy(&forks[i]);
	}


	return 0;
}

/*
while True:
	think()
	get_forks()
	eat()
	put_forks()
*/
void philosopher_thread(void *index) {
	int think_time, eat_time, i = *((int *) index);


	while(1) {
		//simulate thinking:
		think_time = genrand_int32() % (20 + 1 - 1) + 1; //"thinking" time from 1 - 20

		time (&rawtime);
  		timeinfo = localtime (&rawtime);
		//printf("%s -- " ANSI_COLOR_MAGENTA "Philosopher %d will think for %d seconds\n" ANSI_COLOR_RESET, strtok(asctime(timeinfo), "\n"), i, think_time);
		sleep(think_time);

 	 	time (&rawtime);
  		timeinfo = localtime (&rawtime);
		//printf("%s -- " ANSI_COLOR_CYAN "Philosopher %d is done thinking\n" ANSI_COLOR_RESET, strtok(asctime(timeinfo), "\n"), i);
		
		get_forks(index);

		//simulate eating:
		eat_time = genrand_int32() % (9 + 1 - 2) + 2; //"eating" time from 2 - 9

  		time (&rawtime);
  		timeinfo = localtime (&rawtime);
  		//printf("%s -- " ANSI_COLOR_YELLOW "Philosopher %d taking %d seconds to eat\n" ANSI_COLOR_RESET, strtok(asctime(timeinfo), "\n"), i, eat_time);
		sleep(eat_time);

  		time (&rawtime);
  		timeinfo = localtime (&rawtime);
		//printf("%s -- " ANSI_COLOR_BLUE "Philosopher %d is done eating, putting fork down\n" ANSI_COLOR_RESET , strtok(asctime(timeinfo), "\n"), i);
		

		put_forks(index);
	}
}

/*
footman . wait ()
fork [ right (i )]. wait ()
fork [ left ( i )]. wait ()
*/
void get_forks(void *index) {
	int i = *((int *) index);

		sem_wait(&footman);
		sem_wait(&forks[right(i)]);
		sem_wait(&forks[left(i)]);	//must have both forks to eat

		time (&rawtime);
  		timeinfo = localtime (&rawtime);
		//printf("%s -- " ANSI_COLOR_GREEN "Philosopher %d is getting forks\n" ANSI_COLOR_RESET, strtok(asctime(timeinfo), "\n"), i);


		//sem_post(&state[i]);	//signal that philosopher has finished eating.
}

/*
fork [ right (i )]. signal ()
fork [ left ( i )]. signal ()
footman . signal ()

*/

void put_forks(void *index){
	int i = *((int *) index);

		//sem_wait(&state[i]);		//only put forks down if this philosopher has finished eating
		sem_post(&forks[right(i)]);
		sem_post(&forks[left(i)]);
		sem_post(&footman);
	
}

void print_table() {
	int i;

	printf("%-20s|%-10s|%-20s|%-20s|%-20s\n", "Name", "State", "Left Fork", "Right Fork", "Sleep Time");
	printf("----------------------------------------------------------------\n");

	for(i = 0; i < 5; i++) {
		printf("%-20s|", philosophers[i]);
		if(philosopher_status[i] == 0) {
			printf("%-10s|", "Thinking");
		} else {
			printf("%-10s|", "Eating");
		}

		if(forks_status[left(i)] == 0) {
			printf("%-20s|", "Unused");
		} else {
			printf("%-20s|", "Used");
		}

		if(forks_status[right(i)]) {
			printf("%-20s|", "Unused");
		} else {
			printf("%-20s|", "Used");

		}

		//printf("%-20s|", philosopher_sleep_time[i]);
		//philosopher_sleep_time[i]-=1;


		printf("\n");
	}
	sleep(1);
	printf("\e[1;1H\e[2J");

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

