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
#include "queue.c"


#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_BOLD_GREEN	   "\x1b[1;32m"

#define SIZE_OF_POOL 15
 

sem_t resource_insurance, lock;
int shared_resource[256], thread_state[SIZE_OF_POOL], thread_consume_time[SIZE_OF_POOL];		//shared resource aspect of the assignment
int active_threads_counter, init_flag;

void print_table();
int getkey();


int main(int argc, char *argv[]) 
{
	int i, seed = time(NULL);

	pthread_t threads[SIZE_OF_POOL];
	pthread_t lock_check_thread;
	init_genrand(seed);

	//set up semaphore for mutual exclusion
	sem_init(&resource_insurance, 0, 3);

	//initialize semaphore lock
	sem_init(&lock, 0, 0);

	//set up thread_state array:
	for(i=0; i<SIZE_OF_POOL; i++) {
		thread_state[i] = 0;
	}


	// initial counter for number of threads using resource:
	active_threads_counter = 0;

	//initialize threads
	pthread_create(&lock_check_thread, NULL, (void *) lock_check, NULL);

	for(i=0; i<SIZE_OF_POOL; i++) {
		int *index = malloc(sizeof(*index));
		*index = i;
		pthread_create(&threads[i], NULL, (void *) concurrent_thread, index);
	}

	//print the table
	while(getkey() != 27) {
		printf("\e[1;1H\e[2J");
		print_table();
		sleep(1);
	}

	printf("ESC char received, terminating now...\n");
	//clean up:
	sem_destroy(&resource_insurance);
    sem_destroy(&lock);

	return 0;
}


void print_table() 
{
	int i;
	printf("Concurrency 3 Resource Lock Simulation - Press " ANSI_COLOR_CYAN "Escape" ANSI_COLOR_RESET " to quit...\n\n");
	printf("Total Threads Using Resouce: %d\n", active_threads_counter);
	printf("%-10s|%-20s|%-10s|\n", "Thread", "State", "Consume Time");
	printf("--------------------------------------------------------\n");

	for(i = 0; i < SIZE_OF_POOL; i++) {
		printf("%-10d|", i);

		if(thread_state[i] == 0) {
			printf("%-20s|", "Waiting");
		} else if (thread_state[i] == 1 ) {
			printf(ANSI_COLOR_BLUE "%-20s" ANSI_COLOR_RESET "|", "Using Resource");
		} else if (thread_state[i] == 2 ) {
			printf(ANSI_COLOR_GREEN "%-20s" ANSI_COLOR_RESET "|", "Cooling Down");

		}

		if(thread_consume_time[i] > 0 ) {
			printf("%-20d|", thread_consume_time[i]);
		} else {
			printf(ANSI_COLOR_RED "%-20s" ANSI_COLOR_RESET "|", "Waiting");
		}
		thread_consume_time[i] = thread_consume_time[i] - 1;

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
