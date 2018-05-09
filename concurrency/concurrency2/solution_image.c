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

int philosopher_status[5], forks_status[5], philosopher_sleep_time[5];
char *philosopher_status_string[5], *forks_status_string[5];
char recent_buffer[512];

void get_forks(void*);
void put_forks(void*);
void checkSystem();
int left(int);
int right(int);
void set_board();
void print_board();


int main(int argc, char *argv[]) {
	int i, seed = time(NULL);

	pthread_t philosopher[5];
	//checkSystem();
	//use mt19937:
	init_genrand(seed);

	//set up fork semaphores and footman:
	for(i=0; i<4; i++) {
		sem_init(&forks[i], 0, 1);
	}
	sem_init(&footman, 0, 4);

	for(i=0; i<5; i++) {
		philosopher_status[i] = 0;
		forks_status[i] = 0;
		philosopher_sleep_time[i] = 0;

		philosopher_status_string[i] = malloc(sizeof(char) * 8 + 1);
		strcpy(philosopher_status_string[i], "Thinking");

		forks_status_string[i] = malloc(sizeof(char) * 6 + 1);
		strcpy(forks_status_string[i], "Unused");
	}

	

	printf("Beginning dining philosophers simulation... \n  --Press Enter to stop\n");

	print_board();
	
	for(i=0; i<5; i++){
		int *index = malloc(sizeof(*index));
		*index = i;
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

		//printf("%s -- " ANSI_COLOR_MAGENTA "Philosopher %d will think for %d seconds\n" ANSI_COLOR_RESET, strtok(asctime(timeinfo), "\n"), i, think_time);
		philosopher_status[i] = 0;
		philosopher_sleep_time[i] = think_time;
		set_board();
		sleep(think_time);

		//printf("%s -- " ANSI_COLOR_CYAN "Philosopher %d is done thinking\n" ANSI_COLOR_RESET, strtok(asctime(timeinfo), "\n"), i);
		//strcpy(recent_buffer, snprintf("Philosopher %d is done thinking\n"), i);

		//sem_wait(&footman);
		sem_wait(&forks[right(i)]);
		sem_wait(&forks[left(i)]);	//must have both forks to eat

		printf("%s -- " ANSI_COLOR_GREEN "Philosopher %d is getting forks\n" ANSI_COLOR_RESET, strtok(asctime(timeinfo), "\n"), i);
		forks_status[right(i)] = 1;
		forks_status[left(i)] = 1;
		set_board();

		//simulate eating:
		eat_time = genrand_int32() % (9 + 1 - 2) + 2; //"eating" time from 2 - 9

  		//printf("%s -- " ANSI_COLOR_YELLOW "Philosopher %d taking %d seconds to eat\n" ANSI_COLOR_RESET, strtok(asctime(timeinfo), "\n"), i, eat_time);
		philosopher_sleep_time[i] = eat_time;
		philosopher_status[i] = 1;
		set_board();
		sleep(eat_time);

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

		forks_status[right(i)] = 0;
		forks_status[left(i)] = 0;

		sem_post(&forks[right(i)]);
		sem_post(&forks[left(i)]);
		//sem_post(&footman);
	
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

void set_board(){
	int i;
	for(i=0; i<5; i++){
		if (philosopher_status[i] == 0) {
			strcpy(philosopher_status_string[i], "Thinking");
		} else {
			strcpy(philosopher_status_string[i], "Eating");
		}

		if (forks_status[i] == 0) {
			strcpy(forks_status_string[i], "Unused");
		} else {
			strcpy(forks_status_string[i], "In Use");
		}
	}
	print_board();
}	

void print_board(){

		printf("\e[1;1H\e[2J");
		printf("\n");
		printf("						   // ____ \\   \n");
		printf("						  // ///..) \\  \n");
		printf("						 || <||  >\\  || \n");
		printf("						 ||  $$ --/  || \n");
		printf("						 *\\  |\\_/  //* \n");
		printf("						 *\\/___\\_//*   \n");
		printf("						Kevin\n");
		printf("						%s...\n", philosopher_status_string[0]);
		printf("						For %d seconds\n", philosopher_sleep_time[0]--);		
		printf("			     ||||			   		  ||||  \n");	
		printf("			     \\__(			   		  \\__(  \n");	
		printf("			      || 			   		   ||  \n"); 	
		printf("			      ||  			   		   ||  \n");							
		printf("			      ""  			   		   ""  \n");
		printf("			Status: %s				Status: %s\n", forks_status_string[0], forks_status_string[1]);					
		printf("   // ____ \\     									 // ____ \\    	\n");
		printf("  // ///..) \\   									// ///..) \\  	\n");
		printf(" || <||  >\\  || 									|| <||  >\\  ||	\n");
		printf(" ||  $$ --/  || 									||  $$ --/  ||	\n");
		printf(" *\\  |\\_/  //*\t 									*\\  |\\_/  //*	\n");
		printf(" *\\/___\\_//*  \t 									*\\/___\\_//*  	\n");
		printf("Socrates:										Descartes...\n");
		printf("%s...										%s...\n", philosopher_status_string[1], philosopher_status_string[2]);
		printf("For %d seconds									\tFor %d seconds\n", philosopher_sleep_time[1]--, philosopher_sleep_time[2]--);
		printf("			     ||||			   		  ||||  \n");	
		printf("			     \\__(			   		  \\__(  \n");	
		printf("			      || 					   ||  \n"); 	
		printf("			      ||  			   		   ||  \n");							
		printf("			      ""  			   		   ""  \n");
		printf("			Status: %s				Status: %s\n", forks_status_string[2], forks_status_string[3]);			
		printf("		   // ____ \\     				 	 // ____ \\    	\n");
		printf("		  // ///..) \\   					// ///..) \\  	\n");
		printf("		 || <||  >\\  || 					|| <||  >\\  ||	\n");
		printf("		 ||  $$ --/  || 					||  $$ --/  ||	\n");
		printf("		 *\\  |\\_/  //* \t					*\\  |\\_/  //*	\n");
		printf("		 *\\/___\\_//*   \t					*\\/___\\_//*  	\n");
		printf("		Plato:							Aristotle:\n");
		printf("		%s...						%s...\n", philosopher_status_string[3], philosopher_status_string[4]);
		printf("		For %d seconds					\tFor %d seconds\n", philosopher_sleep_time[3]--, philosopher_sleep_time[4]--);
		printf(" 				  		  ||||  \n");
		printf(" 				  		  \\__(  \n");
		printf(" 				  		   ||  \n"); 
		printf(" 				  		   ||  \n");
		printf(" 				  		   ""  \n");
		printf("						Status: %s\n", forks_status_string[4]);
		//sleep(1);


}


