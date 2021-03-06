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
#include <termios.h>
#include "mt19937ar.c"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_BOLD_GREEN	   "\x1b[1;32m"


sem_t footman, forks[5];
int philosopher_status[5], forks_status[5], philosopher_sleep_time[5];
char philosophers[5][128];

void get_forks(void *);
void put_forks(void *);
void philosopher_thread(void *);
void checkSystem();
void print_table();
int left(int);
int right(int);
int getkey();

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

	//name the philosophers
	strcpy(philosophers[0], "Kevin");		// best one there is
	strcpy(philosophers[1], "Socrates");
	strcpy(philosophers[2], "Descartes");
	strcpy(philosophers[3], "Plato");
	strcpy(philosophers[4], "Aristotle");
	

	//initialize threads
	for(i=0; i<5; i++){
		int *index = malloc(sizeof(*index));
		*index = i;
		pthread_create(&philosopher[i], NULL, (void *) philosopher_thread, index);
	}


	//print the table
	while(getkey() != 27) {
		printf("\e[1;1H\e[2J");
		print_table();
		sleep(1);
	}


	printf("ESC char received, terminating now...\n");
	//clean up:
	sem_destroy(&footman);
	for(i=0; i<5; i++){
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
		philosopher_sleep_time[i] = think_time;
		philosopher_status[i] = 0;
		sleep(think_time);
		
		get_forks(index);

		//simulate eating:
		eat_time = genrand_int32() % (9 + 1 - 2) + 2; //"eating" time from 2 - 9
		philosopher_sleep_time[i] = eat_time;
		philosopher_status[i] = 1;
		sleep(eat_time);

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

	forks_status[right(i)] = 1;
	forks_status[left(i)] = 1;

}

/*
fork [ right (i )]. signal ()
fork [ left ( i )]. signal ()
footman . signal ()

*/

void put_forks(void *index){
	int i = *((int *) index);

	sem_post(&forks[right(i)]);
	sem_post(&forks[left(i)]);
	sem_post(&footman);

	forks_status[right(i)] = 0;
	forks_status[left(i)] = 0;
	
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
	return ( i + 1 ) % 5;
}

void print_table() {
	int i;
	printf("Dining Philosopher's Simulation - Press " ANSI_COLOR_CYAN "Escape" ANSI_COLOR_RESET " to quit...\n\n");
	printf("%-20s|%-10s|%-10s|%-10s|%-20s|\n", "Philosopher", "State", "Left Fork", "Right Fork", "Sleep Time");
	printf("------------------------------------------------------------------------------------------\n");

	for(i = 0; i < 5; i++) {
		printf("%-20s|", philosophers[i]);

		if(philosopher_status[i] == 0) {
			printf("%-10s|", "Thinking");
		} else {
			printf(ANSI_COLOR_BLUE "%-10s" ANSI_COLOR_RESET "|", "Eating");
		}

		if(forks_status[left(i)] == 0) {
			printf("%-10s|", "Unused");
		} else {
			printf(ANSI_COLOR_MAGENTA "%-10s" ANSI_COLOR_RESET "|", "Used");
		}

		if(forks_status[right(i)] == 0) {
			printf("%-10s|", "Unused");
		} else {
			printf(ANSI_COLOR_MAGENTA "%-10s" ANSI_COLOR_RESET "|", "Used");
		}

		if(philosopher_sleep_time[i] >0 ) {
			printf("%-20d|", philosopher_sleep_time[i]);
		} else {
			printf(ANSI_COLOR_RED "%-20s" ANSI_COLOR_RESET "|", "Waiting on Forks");
		}
		philosopher_sleep_time[i] = philosopher_sleep_time[i] - 1;


		printf("\n");
	}

}


// Sourced from JetsonHacks: http://www.jetsonhacks.com/2015/12/29/gpio-interfacing-nvidia-jetson-tx1/
// Simple function for non-blocking input read
int getkey() {
    int character;
    struct termios orig_term_attr;
    struct termios new_term_attr;

    /* set the terminal to raw mode */
    tcgetattr(fileno(stdin), &orig_term_attr);
    memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
    new_term_attr.c_lflag &= ~(ECHO|ICANON);
    new_term_attr.c_cc[VTIME] = 0;
    new_term_attr.c_cc[VMIN] = 0;
    tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);

    /* read a character from the stdin stream without blocking */
    /*   returns EOF (-1) if no character is available */
    character = fgetc(stdin);

    /* restore the original terminal attributes */
    tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);

    return character;
}
