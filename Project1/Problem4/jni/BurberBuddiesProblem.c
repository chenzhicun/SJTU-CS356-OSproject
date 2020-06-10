#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#define CUSTOMER_TIME 100
#define BURGER_TIME 5

struct COOK{
    int cook_id;
    int cook_time;
};

sem_t mutex1;
sem_t mutex2;
sem_t console_mutex;
sem_t empty_slots_on_rack;
sem_t burgers_on_rack;
sem_t cashier;
sem_t customer;

int total_burger_count;
int remain_customer_count;
int customer_size;
int cook_size;
int cashier_size;
int rack_size;

/*
            pseudo code for customer

CUSTOMER{
    // customer coming.....
    signal(customer);
    wait(cashier);
}

*/

void *customer_func(void *param)
{
    int customer_id=(*(int*)param);
    sleep(rand()%CUSTOMER_TIME);

    sem_wait(&console_mutex);
    printf("Customer[%d] come.\n",customer_id);
    sem_post(&console_mutex);
    sem_post(&customer);
    sem_wait(&cashier);

    return NULL;
}

/*
            pesudo code for cashier

CASHIER{
    while(true)
    {
        wait(mutex1)
        if(remain_customer_count<=0){
            signal(mutex1);
            break;
        }
        else{
            remain_customer_count--;
            signal(mutex1);
        }
        wait(customer);
        //take order....
        wait(burger_on_rack);
        signal(empty_slots_on_rack);
        //get burger for customer;
        signal(cashier);
    }
}

*/

void *cashier_func(void *param)
{
    int cashier_id=*(int*)param;
    while(1)
    {
        //sleep(1);
        sem_wait(&mutex1);
        if(remain_customer_count<=0)
        {
            sem_post(&mutex1);
            break;
        }
        else
        {
            remain_customer_count--;
            sem_post(&mutex1);
        }
        
        sem_wait(&customer);
        sem_wait(&console_mutex);
        printf("Cashier[%d] accepts an order.\n",cashier_id);
        sem_post(&console_mutex);

        sem_wait(&burgers_on_rack);
        sem_post(&empty_slots_on_rack);
        sem_post(&cashier);
        sem_wait(&console_mutex);
        printf("Cashier[%d] takes a burger to customer.\n",cashier_id);
        sem_post(&console_mutex);
    }
    
    return NULL;
}

/*
            pseudo code for cook

COOK{
    //determine how long costs to make a burger for this cook
    while(true){
        //sleep for making burger time;
        wait(mutex2);
        if(total_burger>=customer_size){
            signal(mutex2);
            break;
        }
        else{
            total_burger++;
            signal(mutex2);
        }
        //make burger.....
        wait(empty_slots_on_rack);
        signal(burgers_on_rack);
    }
}

*/

void *cook_func(void *param)
{
    struct COOK *cook=(struct COOK*)param;
    int cook_id=cook->cook_id;
    int cook_time=cook->cook_time;
    /*for a specific cook, he has a fixed time to cook a burger*/
    
    while(1)
    {
        sem_wait(&mutex2);
        if(total_burger_count>=customer_size){
            sem_post(&mutex2);
            break;
        }
        else{
            total_burger_count++;
            sem_post(&mutex2);
        }

        sem_wait(&empty_slots_on_rack);
        sleep(cook_time);
        sem_wait(&console_mutex);
        printf("Cook[%d] make a burger.\n",cook_id);
        sem_post(&console_mutex);
        sem_post(&burgers_on_rack);
    }

    return NULL;
}

int main(int argc, char **argv)
{
    if(argc!=5)
    {
        printf("Wrong argument format! Should like ./BBC ${COOK_SIZE} ${CASHIER_SIZE} ${CUSTOMER_SIZE} {RACK_SIZE}.\n");
        return -1;
    }
    if(atoi(argv[1])<=0){
        /*input is negative number or can not be transformed into integer.*/
        printf("cook_size input format is wrong!\n");
        return -1;
    }
    else cook_size=atoi(argv[1]);
    if(atoi(argv[2])<=0){
        /*input is negative number or can not be transformed into integer.*/
        printf("cashier_size input format is wrong!\n");
        return -1;
    }
    else cashier_size=atoi(argv[2]);
    if(atoi(argv[3])<=0){
        /*input is negative number or can not be transformed into integer.*/
        printf("customer_size input format is wrong!\n");
        return -1;
    }
    else customer_size=atoi(argv[3]);
    if(atoi(argv[4])<=0){
        /*input is negative number or can not be transformed into integer.*/
        printf("rack_size input format is wrong!\n");
        return -1;
    }
    else rack_size=atoi(argv[4]);

    int i;
    srand(time(NULL));
    remain_customer_count=customer_size;
    total_burger_count=0;
    sem_init(&mutex1,0,1);
    sem_init(&mutex2,0,1);
    sem_init(&console_mutex,0,1);
    sem_init(&empty_slots_on_rack,0,rack_size);
    sem_init(&burgers_on_rack,0,0);
    sem_init(&cashier,0,0);
    sem_init(&customer,0,0);
    int *customer_args=(int*)malloc(customer_size*sizeof(int));
    int *cashier_args=(int*)malloc(cashier_size*sizeof(int));
    struct COOK *cook_args=(struct COOK*)malloc(cook_size*sizeof(struct COOK));
    pthread_t *customer_threads=(pthread_t*)malloc(customer_size*sizeof(pthread_t));
    pthread_t *cashier_threads=(pthread_t*)malloc(cashier_size*sizeof(pthread_t));
    pthread_t *cook_threads=(pthread_t*)malloc(cook_size*sizeof(pthread_t));

    printf("Cooks[%d],Cashiers[%d],Customers[%d],Rack[%d]\n\nBegin run.\n\n",cook_size,cashier_size,customer_size,rack_size);

    for(i=0;i<cook_size;i++)
    {
        cook_args[i].cook_id=i+1;
        /*a specific cook should have a non-zero fixed time to cook a burger.*/
        cook_args[i].cook_time=(rand()%BURGER_TIME)+1;
        if(pthread_create(cook_threads+i,NULL,cook_func,cook_args+i)!=0)
        {
            printf("error happened while creating a cook thread.\n");
            return -1;
        }
    }

    for(i=0;i<customer_size;i++)
    {
        customer_args[i]=i+1;
        if(pthread_create(customer_threads+i,NULL,customer_func,customer_args+i)!=0)
        {
            printf("error happened while creating a customer thread.\n");
            return -1;
        }
    }

    for(i=0;i<cashier_size;i++)
    {
        cashier_args[i]=i+1;
        if(pthread_create(cashier_threads+i,NULL,cashier_func,cashier_args+i)!=0)
        {
            printf("error happened while creating a cashier thread.\n");
            return -1;
        }
    }

    for(i=0;i<customer_size;i++)
        pthread_join(customer_threads[i],NULL);
    for(i=0;i<cashier_size;i++)
        pthread_join(cashier_threads[i],NULL);
    for(i=0;i<cook_size;i++)
        pthread_join(cook_threads[i],NULL);
    
    printf("\nEnd run.\n");

    sem_destroy(&mutex1);
    sem_destroy(&mutex2);
    sem_destroy(&console_mutex);
    sem_destroy(&empty_slots_on_rack);
    sem_destroy(&burgers_on_rack);
    sem_destroy(&cashier);
    sem_destroy(&customer);

    free(cook_args);
    free(cook_threads);
    free(customer_args);
    free(customer_threads);
    free(cashier_args);
    free(cashier_threads);

    return 0;
}