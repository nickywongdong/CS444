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

typedef struct 
{
    int counter;
    sem_t mutex;

}Lightswitch;

void print_table();
int getkey();
void lock(Lightswitch *, sem_t *);
void unlock(Lightswitch *, sem_t *);
void searcher(void *);
void inserter(void *);
void deleter(void *);

sem_t insertMutex, noSearcher, noInserter;
Lightswitch searchSwitch, insertSwitch;
int search_status[2], insert_status[2], delete_status[2];
int search_time[2], insert_time[2], delete_time[2];

int main(int argc, char *argv[]) 
{
	int i, seed = time(NULL);
	int *index = malloc(sizeof(*index));


	pthread_t searcher_thread1, inserter_thread1, deleter_thread1;
	pthread_t searcher_thread2, inserter_thread2, deleter_thread2;

	init_genrand(seed);

	//initialize variables for visualization:
	for(i=0; i<2; i++) {
		search_status[i] = 0;
		insert_status[i] = 0;
		delete_status[i] = 0;
		search_time[i] = 0;
		insert_time[i] = 0;
		delete_time[i] = 0;
	}

	//initialize semaphores and lightswitches:
	sem_init(&insertMutex, 0, 1);
	sem_init(&noSearcher, 0, 1);
	sem_init(&noInserter, 0, 1);

		searchSwitch.counter = 0;
		sem_init(&searchSwitch.mutex, 0, 1);
		insertSwitch.counter = 0;
		sem_init(&insertSwitch.mutex, 0, 1);


	*index = 0;
	pthread_create(&searcher_thread1, NULL, (void *) searcher, index);
	pthread_create(&inserter_thread1, NULL, (void *) inserter, index);
	pthread_create(&deleter_thread1, NULL, (void *) deleter, index);

	*index = 1;
	pthread_create(&searcher_thread2, NULL, (void *) searcher, index);
	pthread_create(&inserter_thread2, NULL, (void *) inserter, index);
	pthread_create(&deleter_thread2, NULL, (void *) deleter, index);

	//print the table
	while(getkey() != 27) {
		printf("\e[1;1H\e[2J");
		print_table();
		sleep(1);
	}

	printf("ESC char received, terminating now...\n");
	//clean up:
	sem_destroy(&insertMutex);
    sem_destroy(&noSearcher);
    sem_destroy(&noInserter);

    	sem_destroy(&searchSwitch.mutex);
    	sem_destroy(&insertSwitch.mutex);


	return 0;
}



//searcher
/*
searchSwitch . wait ( noSearcher )
//critical section
searchSwitch . signal ( noSearcher )
*/
void searcher(void *index)
{
	int i = *((int *) index);
	while(1) {
		lock(&searchSwitch, &noSearcher);
		
		// critical section
		search_status[i] = 1;	//signal that searcher is running
		search_time[i] = genrand_int32() % (5 + 1 - 1) + 1; //"consume" time from 1 - 5

		sleep(search_time[i]);				//emulate a search time

		unlock(&searchSwitch, &noSearcher);

		search_status[i] = 0;

		sleep(5);		//allow other operations to take place
	}
}


//inserter
/*
insertSwitch . wait ( noInserter )
insertMutex . wait ()
// critical section
insertMutex . signal ()
insertSwitch . signal ( noInserter )
*/
void inserter(void *index)
{
	int i = *((int *) index);
	while(1) {
		lock(&insertSwitch, &noInserter);
		sem_wait(&insertMutex);

		//critical section:
		insert_status[i] = 1;
		insert_time[i] = genrand_int32() % (5 + 1 - 1) + 1; //"consume" time from 1 - 5
		sleep(insert_time[i]);

		sem_post(&insertMutex);
		unlock(&insertSwitch, &noInserter);
		insert_status[i] = 0;

		sleep(5);		//allow other operations to take place

	}

}

//deleter
/*
noSearcher . wait ()
noInserter . wait ()
//critical section
noInserter . signal ()
noSearcher . signal ()
*/
void deleter(void *index)
{
	int i = *((int *) index);
	while(1) {
		sem_wait(&noSearcher);
		sem_wait(&noInserter);

		//critical section:
		delete_status[i] = 1;
		delete_time[i] = genrand_int32() % (5 + 1 - 1) + 1; //"consume" time from 1 - 5
		sleep(delete_time[i]);

		sem_post(&noInserter);
		sem_post(&noSearcher);

		delete_status[i] = 0;

		sleep(5);		//allow other operations to take place

	}
}

/*
Lightswitch Implementation:
class Lightswitch :
def __init__ ( self ):
self . counter = 0
self . mutex = Semaphore (1)
def lock ( self , semaphore ):
self . mutex . wait ()
self . counter += 1
if self . counter == 1:
 semaphore . wait ()
 self . mutex . signal ()

 def unlock ( self , semaphore ):
 self . mutex . wait ()
 self . counter -= 1
 if self . counter == 0:
 semaphore . signal ()
 self . mutex . signal ()*/

void lock(Lightswitch *self, sem_t *semaphore)
{
    sem_wait(&self->mutex);
    self->counter+=1;
    if (self->counter == 1) {
        sem_wait(semaphore);
    }
    sem_post(&self->mutex);
}

void unlock(Lightswitch *self, sem_t* semaphore)
{
    sem_wait(&self->mutex);
    self->counter-=1;
    if (self->counter == 0) {
         sem_post(semaphore);
    }
    sem_post(&self->mutex);
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


void print_table() 
{
	int i;
	printf("Search-Insert-Delete Simulation - Press " ANSI_COLOR_CYAN "Escape" ANSI_COLOR_RESET " to quit...\n\n");
	printf("%-10s|%-20s|%-20s|\n", "Thread", "Status", "Operation Time");
	printf("------------------------------------------------------------\n");

		for(i=0; i<2; i++) {
			printf("%-10s %d|", "Search:", i);

			if(search_status[i] == 0) {
				printf("%-20s|", "Waiting");
			} else if (search_status[i] == 1 ) {
				printf(ANSI_COLOR_BLUE "%-20s" ANSI_COLOR_RESET "|", "Using Resource");
			}

			if(search_time[i] > 0 ) {
				printf("%-20d|", search_time[i]);
			} else {
				printf(ANSI_COLOR_RED "%-20s" ANSI_COLOR_RESET "|", "Waiting");
			}
			search_time[i] = search_time[i] - 1;

			printf("\n");
		}

		for(i=0; i<2; i++) {
			printf("%-10s %d|", "Insert:", i);

			if(insert_status[i] == 0) {
				printf("%-20s|", "Waiting");
			} else if (insert_status[i] == 1 ) {
				printf(ANSI_COLOR_BLUE "%-20s" ANSI_COLOR_RESET "|", "Using Resource");
			}

			if(insert_time[i] > 0 ) {
				printf("%-20d|", insert_time[i]);
			} else {
				printf(ANSI_COLOR_RED "%-20s" ANSI_COLOR_RESET "|", "Waiting");
			}
			insert_time[i] = insert_time[i] - 1;

			printf("\n");
		}

		for(i=0; i<2; i++) {
			printf("%-10s %d|", "Delete:", i);

			if(delete_status[i] == 0) {
				printf("%-20s|", "Waiting");
			} else if (delete_status[i] == 1 ) {
				printf(ANSI_COLOR_BLUE "%-20s" ANSI_COLOR_RESET "|", "Using Resource");
			}

			if(delete_time[i] > 0 ) {
				printf("%-20d|", delete_time[i]);
			} else {
				printf(ANSI_COLOR_RED "%-20s" ANSI_COLOR_RESET "|", "Waiting");
			}
			delete_time[i] = delete_time[i] - 1;

			printf("\n");
		}

}