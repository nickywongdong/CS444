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

#define NUM_CHAIRS 5
 

sem_t barber_sem;
int thread_state[NUM_CHAIRS], thread_consume_time[NUM_CHAIRS];
int active_threads_counter, init_flag, barber_status, barber_time, num_customers, customer_time, wait_time, timer, paused_timer;
struct Queue* queue;
pthread_t customer;

void print_table();
int getkey();
void get_hair_cut();
void cut_hair();
void main_loop();
void barber_function();

int main(int argc, char *argv[]) 
{
	int i, seed = time(NULL);

	//initialize threads for each chair, and single barber
	pthread_t main_thread, barber_thread;
	init_genrand(seed);

	//initialize variables for printing later
	barber_status = 0;
	barber_time = 10;
	num_customers = 0;
	customer_time = 10;

	timer = 40;

	//paused_timer = 30;

	//initialize semaphore for barber
	sem_init(&barber_sem, 0, 1);
	
	//set up thread_state array:
	for(i=0; i<NUM_CHAIRS; i++) {
		thread_state[i] = 0;
	}

	//set up queue structure with maximum capacity equal to num chairs
	queue = createQueue(NUM_CHAIRS);

	// initial counter for number of threads using resource:
	active_threads_counter = 0;

	//initialize main thread:
	pthread_create(&main_thread, NULL, (void *) main_loop, NULL);
	pthread_create(&barber_thread, NULL, (void *) barber_function, NULL);


	//print the table
	while(getkey() != 27) {
		printf("\e[1;1H\e[2J");
		print_table();
		sleep(1);
	}

	printf("ESC char received, terminating now...\n");
	//clean up:
	sem_destroy(&barber_sem);

	return 0;
}


//barber function for thread
void barber_function()
{
	while(1) {
		// have barber sleep until queue is not empty
		while(isEmpty(queue)) {
			barber_status = 0;
			nanosleep((const struct timespec[]){{0, 5000000000L}}, NULL);
			barber_status = 1;
		}
	}
	barber_status = 0;
}

//main thread which invokes other threads, will generate customers randomly, and invoke other methods
void main_loop()
{
	pthread_t barber;

	while(1) {
		//formula: r = (rand() % (max + 1 - min)) + min
		wait_time = genrand_int32() % (5 + 1 - 3) + 3; //"wait" time between customer generation from 3-5
		sleep(wait_time);
		//If the shop is full, a customer exits the shop.
		if(queue->size >= NUM_CHAIRS)
			printf("customer has left\n");
		//else
			//num_customers++;

		if(timer <= 0 ) {
			paused_timer = 55;
			sleep(55);
			timer = 40;
		} else {
			enqueue(queue, 1);
		}

		//Customers invoke get_hair_cut when sitting in the barber chair.
		//The barber thread invokes cut_hair.
		pthread_create(&barber, NULL, (void *) cut_hair, NULL);
	}
}

//get hair_cut thread call for customers
void get_hair_cut()
{
	sleep(10);
}

//cut hair thread call for barber
void cut_hair()
{
	sem_wait(&barber_sem);
	barber_time = 10;


	//allow customer to run thread concurrently
	pthread_create(&customer, NULL, (void *) cut_hair, NULL);
	sleep(10);

	dequeue(queue);
	//num_customers--;
	sem_post(&barber_sem);

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
	printf("Concurrency 4 Barber Chair Simulation - Press " ANSI_COLOR_CYAN "Escape" ANSI_COLOR_RESET " to quit...\n\n");
	if(timer > 0 ) {
		printf("After " ANSI_COLOR_GREEN "%d " ANSI_COLOR_RESET "seconds, stop generating customers to demonstrate barber sleep\n", timer);
		timer-=1;

		printf("Taking %d seconds to generate a new customer\n", wait_time);
	} else if(timer <= 0 ) {
		printf("Stopping generation of customers to demonstrate barber sleep for" ANSI_COLOR_RED " %d " ANSI_COLOR_RESET "seconds\n", paused_timer);
		paused_timer-=1;
	}

	//printf("State of queue: %d\n", isEmpty(queue));
	//printf("size of queue: %d\n", queue->size);
	//printf("capacity of queue: %d\n", queue->capacity);
	wait_time -= 1;
	if(barber_status == 0)
		printf("Barber Status: %s\n", "Sleeping");
	else if (barber_status == 1)
		printf("Barber Status: %s, %d seconds left\n", "Cutting Hair", barber_time);
	barber_time -= 1;

	printf("Customers:\n");
	printf("------------------\n");

	printf("%d Customers Waiting\n", queue->size);

	if(barber_status == 1) {
		printf("Customer getting hair cut, %d seconds left", customer_time);
	}

	customer_time -= 1;
}
