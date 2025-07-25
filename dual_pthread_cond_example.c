#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // For sleep

pthread_mutex_t     my_mutex_1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t      my_condition_1 = PTHREAD_COND_INITIALIZER;

pthread_mutex_t     my_mutex_2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t      my_condition_2 = PTHREAD_COND_INITIALIZER;

int shared_data_ready_1 = 0; // The condition to wait for
int shared_data_ready_2 = 0; // The condition to wait for

void *producer_thread(void *arg)
{
    printf("Producer: Doing some work...\n");
    sleep(2); // Simulate work

    while(1)
    {
        while( 1 )
        {
            pthread_mutex_lock(&my_mutex_1);

            shared_data_ready_1 = 1; // Set the condition
            printf("Producer: Data is ready, signaling consumer.\n");


            pthread_cond_signal(&my_condition_1); // Signal one waiting thread
            pthread_mutex_unlock(&my_mutex_1);

            sleep(1);
            if( shared_data_ready_1 == 0 )
                break;
        }

        while(1)
        {
            pthread_mutex_lock(&my_mutex_2);
            while (shared_data_ready_2 == 0)   // Loop to handle spurious wakeups
            {
                pthread_cond_wait(&my_condition_2, &my_mutex_2); // Atomically unlocks mutex and waits
            }

            printf("Producer: Data received! Processing...\n");
            shared_data_ready_2 = 0;

            // Process the shared data
            pthread_mutex_unlock(&my_mutex_2);
            break;
        }
    }

    return NULL;
}

void *consumer_thread(void *arg)
{
    printf("Consumer: Waiting for data...\n");

    while(1)
    {
        while(1)
        {
            pthread_mutex_lock(&my_mutex_1);
            while (shared_data_ready_1 == 0)   // Loop to handle spurious wakeups
            {
                pthread_cond_wait(&my_condition_1, &my_mutex_1); // Atomically unlocks mutex and waits
            }

            printf("Consumer: Data received! Processing...\n");
            shared_data_ready_1 = 0;

            // Process the shared data
            pthread_mutex_unlock(&my_mutex_1);

            break;
        }

        while( 1 )
        {
            pthread_mutex_lock(&my_mutex_2);

            shared_data_ready_2 = 1; // Set the condition
            printf("Consumer: Data is ready, signaling consumer.\n");

            pthread_cond_signal(&my_condition_2); // Signal one waiting thread
            pthread_mutex_unlock(&my_mutex_2);

            sleep(1);
            if( shared_data_ready_2 == 0 )
                break;
        }
    }
    return NULL;
}

int main()
{
    pthread_t prod_tid, cons_tid;

    pthread_create(&prod_tid, NULL, producer_thread, NULL);
    pthread_create(&cons_tid, NULL, consumer_thread, NULL);

    pthread_join(prod_tid, NULL);
    pthread_join(cons_tid, NULL);

    pthread_mutex_destroy(&my_mutex_1);
    pthread_cond_destroy(&my_condition_1);

    pthread_mutex_destroy(&my_mutex_2);
    pthread_cond_destroy(&my_condition_2);

    system("pause");

    return 0;
}
