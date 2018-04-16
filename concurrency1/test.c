#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

sem_t sem1, sem2;
int buffer;

//consumer
void threadfunc() {
    while (1) {
        sem_wait(&sem2);
        buffer--;
        printf(" --Consuming an item, buffer is now: %d!\n", buffer);
        sleep(5);
        sem_post(&sem1);
    }
}

//producer
void threadfunc2() {
    while (1) {
        sem_wait(&sem1);
        buffer++;
        printf("Producing an item, buffer is now: %d!\n", buffer);
        sleep(2);
        sem_post(&sem2);
    }
}

int main(void) {
    
    // initialize semaphore, only to be used with threads in this process, set value to 1
    sem_init(&sem1, 0, 5);
    sem_init(&sem2, 0, 0);

    buffer=0;

    
    pthread_t mythread, mythread2;
        
    // start the thread
    printf("Starting thread, semaphore is unlocked.\n");
    printf("Press Enter to end program\n");
    pthread_create(&mythread, NULL, (void*)threadfunc, NULL);
    pthread_create(&mythread2, NULL, (void*)threadfunc2, NULL);

    getchar();
    
    
    return 0;
}