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


void print_table();
int getkey();
void pusher_a();
void pusher_b();
void pusher_c();
void agent_a();
void agent_b();
void agent_c();
void smoker_tobacco();
void smoker_matches();
void smoker_paper();
void makeCigarette();
void smoke();

/*
agentSem = Semaphore (1)
2 tobacco = Semaphore (0)
3 paper = Semaphore (0)
4 match = Semaphore (0)
*/
/*

isTobacco = isPaper = isMatch = False
2 tobaccoSem = Semaphore (0)
3 paperSem = Semaphore (0)
4 matchSem = Semaphore (0)
*/

sem_t tobaccoSem, paperSem, matchSem;
sem_t tobacco, paper, match, agentSem;
sem_t mutex;
int isTobacco, isPaper, isMatch;
int agent_status, resource_wait, smoker_status[3], smoker_smoke_time[3], cigarette_craft_time[3];

int main(int argc, char *argv[]) 
{
	int i, seed = time(NULL);
	init_genrand(seed);

	pthread_t pusher_a_thread, pusher_b_thread, pusher_c_thread;
	pthread_t agent_a_thread, agent_b_thread, agent_c_thread;
	pthread_t smoker_paper_thread, smoker_matches_thread, smoker_tobacco_thread;


	//initialize semaphores:
	sem_init(&tobaccoSem, 0, 0);
	sem_init(&paperSem, 0, 0);
	sem_init(&matchSem, 0, 0);
	sem_init(&tobacco, 0, 0);
	sem_init(&paper, 0, 0);
	sem_init(&match, 0, 0);
	sem_init(&agentSem, 0, 1);
	sem_init(&mutex, 0, 1);

	//initialize booleans for check
	isTobacco = 0;
	isPaper = 0;
	isMatch = 0;

	//variables used for visualization in print_table
	for(i=0; i<3; i++) {
		smoker_status[i] = 0;
		smoker_smoke_time[i] = 0;
		cigarette_craft_time[i] = 0;
	}

	agent_status = 0;
	resource_wait = 0;

	pthread_create(&pusher_a_thread, NULL, (void *) pusher_a, NULL);
	pthread_create(&pusher_b_thread, NULL, (void *) pusher_b, NULL);
	pthread_create(&pusher_c_thread, NULL, (void *) pusher_c, NULL);

	pthread_create(&agent_a_thread, NULL, (void *) agent_a, NULL);
	pthread_create(&agent_b_thread, NULL, (void *) agent_b, NULL);
	pthread_create(&agent_c_thread, NULL, (void *) agent_c, NULL);

	pthread_create(&smoker_paper_thread, NULL, (void *) smoker_paper, NULL);
	pthread_create(&smoker_matches_thread, NULL, (void *) smoker_matches, NULL);
	pthread_create(&smoker_tobacco_thread, NULL, (void *) smoker_tobacco, NULL);

	//print the table
	while(getkey() != 27) {
		printf("\e[1;1H\e[2J");
		print_table();
		sleep(1);
	}


	printf("ESC char received, terminating now...\n");

	//clean up:
	sem_destroy(&tobaccoSem);
	sem_destroy(&paperSem);
	sem_destroy(&matchSem);
	sem_destroy(&tobacco);
	sem_destroy(&paper);
	sem_destroy(&match);
	sem_destroy(&agentSem);
	sem_destroy(&mutex);

	return 0;
}

/*
agentSem . wait ()
2 tobacco . signal ()
3 paper . signal ()
*/
void agent_a()
{
	while(1) {
		sem_wait(&agentSem);
		agent_status = 1;
		sem_post(&tobacco);
		sem_post(&paper);
		agent_status = 0;
		sleep(15);
	}
}

/*
agentSem . wait ()
2 paper . signal ()
3 match . signal ()
*/
void agent_b()
{
	while(1) {
		sem_wait(&agentSem);
		agent_status = 2;
		sem_post(&paper);
		sem_post(&match);
		agent_status = 0;
		sleep(15);
	}
}

/*
agentSem . wait ()
2 tobacco . signal ()
3 match . signal ()*/
void agent_c()
{
	while(1) {
		sem_wait(&agentSem);
		agent_status = 3;
		sem_post(&tobacco);
		sem_post(&match);
		agent_status = 0;
		sleep(15);
	}
	
}


/*
tobacco . wait ()
2 mutex . wait ()
3 if isPaper :
4 isPaper = False
5 matchSem . signal ()
6 elif isMatch :
7 isMatch = False
8 paperSem . signal ()
9 else :
10 isTobacco = True
11 mutex . signal ()
*/
void pusher_a()
{
	while(1) {
		sem_wait(&tobacco);
		sem_wait(&mutex);
		if (isPaper) {
			isPaper = 0;
			sem_post(&matchSem);
		} else if (isMatch) {
			isMatch = 0;
			sem_post(&paperSem);
		} else {
			isTobacco = 1;
		}
		sem_post(&mutex);
	}
}

void pusher_b()
{
	while(1) {
		sem_wait(&paper);
		sem_wait(&mutex);
		if (isTobacco) {
			isTobacco = 0;
			sem_post(&matchSem);
		} else if (isMatch) {
			isMatch = 0;
			sem_post(&tobaccoSem);
		} else {
			isPaper = 1;
		}
		sem_post(&mutex);
	}
}

void pusher_c()
{
	while(1) {
		sem_wait(&match);
		sem_wait(&mutex);
		if (isTobacco) {
			isTobacco = 0;
			sem_post(&paperSem);
		} else if (isPaper) {
			isPaper = 0;
			sem_post(&tobaccoSem);
		} else {
			isMatch = 1;
		}
		sem_post(&mutex);
	}
}

void smoker_matches()
{
	while(1) {
		sem_wait(&matchSem);
		makeCigarette(0);
		sem_post(&agentSem);
		smoke(0);
	}
}

void smoker_paper()
{
	while(1) {
		sem_wait(&paperSem);
		makeCigarette(1);
		sem_post(&agentSem);
		smoke(1);
	}
}
/*
tobaccoSem . wait ()
2 makeCigarette ()
3 agentSem . signal ()
4 smoke ()
*/
void smoker_tobacco()
{
	while(1) {
		sem_wait(&tobaccoSem);
		makeCigarette(2);
		sem_post(&agentSem);
		smoke(2);
	}
}

//smoker will call this function when he receives all necessary items
void makeCigarette(int i)
{	
	smoker_status[i] = 1;		//smoker status is now crafting cigarette
	//take between 1-5 seconds to make a cigarette;
	int rand_craft_time = genrand_int32() % (5 + 1 - 1) + 1;
	cigarette_craft_time[i] = rand_craft_time;
	sleep(rand_craft_time);
}

//smoker will call this function once he has made a cigarette, and will spend some time in here
void smoke(int i)
{
	smoker_status[i] = 2;		//smoker status is now smoking
	//take 10-15 seconds to smoke a cigarette;
	int rand_smoke_time = genrand_int32() % (15 + 1 - 10) + 10;
	smoker_smoke_time[i] = rand_smoke_time;
	sleep(rand_smoke_time);
	smoker_status[i] = 0;		//smoker goes back to waiting for resources

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
	printf("Cigarette Smokers Simulation - Press " ANSI_COLOR_CYAN "Escape" ANSI_COLOR_RESET " to quit...\n\n");
	/*printf("Agent Status: ");
	
	if(agent_status == 0) {
		printf("Waiting\n");
	} else if (agent_status == 1) {
		printf("Providing Tobacco & Paper\n");
	} else if (agent_status == 2) {
		printf("Providing Paper & Match\n");
	} else if (agent_status == 3) {
		printf("Providing Tobacco & Match\n");
	}*/

	printf("%-20s|%-20s|%-20s|\n", "Smoker", "Status", "Sleep Time");
	printf("--------------------------------------------------------\n");

	for(i=0; i<3; i++) {
		if(i == 0)
			printf("%-20s|", "Smoker With Matches");
		else if (i == 1)
			printf("%-20s|", "Smoker With Paper");
		else
			printf("%-20s|", "Smoker With Tobacco");


		if(smoker_status[i] == 0) {
			printf("%-20s|", "Waiting");

		} else if (smoker_status[i] == 1 ) {
			printf(ANSI_COLOR_BLUE "%-20s" ANSI_COLOR_RESET "|", "Crafting Cigarette");
			
			//display wait time
			if(cigarette_craft_time[i] > 0 ) {
				printf("%-20d|", cigarette_craft_time[i]);
			cigarette_craft_time[i]--;

			} else {
				printf(ANSI_COLOR_RED "%-20s" ANSI_COLOR_RESET "|", "Waiting");
			}

		} else if (smoker_status[i] == 2 ) {
			printf(ANSI_COLOR_BLUE "%-20s" ANSI_COLOR_RESET "|", "Smoking");

			//display wait time
			if(smoker_smoke_time[i] > 0 ) {
				printf("%-20d|", smoker_smoke_time[i]);
				smoker_smoke_time[i]--;

			} else {
				printf(ANSI_COLOR_RED "%-20s" ANSI_COLOR_RESET "|", "Waiting");
			}
		}

		printf("\n");
	}
}