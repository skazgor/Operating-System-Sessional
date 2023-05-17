#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>
#include "zemaphore.h"

int N ;
int num_iterations=0;
zem_t agent_zem, tobacco_zem, paper_zem, match_zem, smoker_tobacco_zem, smoker_paper_zem, smoker_match_zem;
int is_paper, is_tobacco, is_match=0;
pthread_mutex_t is_var_mutex;
void *agent(void *arg)
{
    int num_1, num_2;
    while (1)
    {
        if (num_iterations == N)
            break;
        zem_down(&agent_zem);
        num_1 = rand() % 3;
        num_2 = rand() % 3;
        while (num_1 == num_2)
        {
            num_2 = rand() % 3;
        }
        // printf("%d %d",num_1,num_2);
        if ((num_1 == 0 && num_2 == 1) ||(num_2==0 && num_1==1))
        {
            printf("Agent puts tobacco and paper on the table\n");
            zem_up(&tobacco_zem);
            zem_up(&paper_zem);
        }
        else if ((num_1 == 0 && num_2 == 2) ||(num_2==0 && num_1==2))
        {
            printf("Agent puts tobacco and match on the table\n");
            zem_up(&tobacco_zem);
            zem_up(&match_zem);
        }
        else if ((num_1 == 1 && num_2 == 2) ||(num_2==1 && num_1==2))
        {
            printf("Agent puts paper and match on the table\n");
            zem_up(&paper_zem);
            zem_up(&match_zem);
        }
        num_iterations++;
    }
    zem_down(&agent_zem);
    num_iterations++;
    zem_up(&smoker_match_zem);
    zem_up(&smoker_paper_zem);
    zem_up(&smoker_tobacco_zem);
    zem_up(&tobacco_zem);
    zem_up(&paper_zem);
    zem_up(&match_zem);
    return NULL;
}
void* pusher_tobacco( void* arg )
{
    while(1)
    {
        zem_down(&tobacco_zem);
        if (num_iterations == N+1)
            break;
        pthread_mutex_lock(&is_var_mutex);
        if(is_paper)
        {
            is_paper = 0;
            zem_up(&smoker_match_zem);
        }
        else if( is_match)
        {
            is_match = 0;
            zem_up(&smoker_paper_zem);
        }
        else
        {
            is_tobacco = 1;
        }
        pthread_mutex_unlock(&is_var_mutex);
    }
    return NULL;
}
void* pusher_paper( void* arg )
{
    while(1)
    {
        zem_down(&paper_zem);
        if (num_iterations == N+1)
            break;
        pthread_mutex_lock(&is_var_mutex);
        if(is_tobacco)
        {
            is_tobacco = 0;
            zem_up(&smoker_match_zem);
        }
        else if( is_match)
        {
            is_match = 0;
            zem_up(&smoker_tobacco_zem);
        }
        else
        {
            is_paper = 1;
        }
        pthread_mutex_unlock(&is_var_mutex);
    }
    return NULL;
}
void* pusher_match( void* arg )
{
    while(1)
    {
        zem_down(&match_zem);
        if (num_iterations == N+1)
            break;
        pthread_mutex_lock(&is_var_mutex);
        if(is_tobacco)
        {
            is_tobacco = 0;
            zem_up(&smoker_paper_zem);
        }
        else if( is_paper)
        {
            is_paper = 0;
            zem_up(&smoker_tobacco_zem);
        }
        else
        {
            is_match = 1;
        }
        pthread_mutex_unlock(&is_var_mutex);
    }
    return NULL;
}
void* smoker_tobacco( void* arg )
{
    while(1)
    {
        zem_down(&smoker_tobacco_zem);
        if (num_iterations == N+1)
            break;
        printf("Smoker with tobacco is smoking\n");
        zem_up(&agent_zem);
    }
    return NULL;
}
void* smoker_paper( void* arg )
{
    while(1)
    {
        zem_down(&smoker_paper_zem);
        if (num_iterations == N+1)
            break;
        printf("Smoker with paper is smoking\n");
        zem_up(&agent_zem);
    }
    return NULL;
}
void* smoker_match( void* arg )
{
    while(1)
    {
        zem_down(&smoker_match_zem);
        if (num_iterations == N+1)
            break;
        printf("Smoker with match is smoking\n");
        zem_up(&agent_zem);
    }
    return NULL;
}
int main(int argc, char *argv[])
{ 
    pthread_t agent_thread, pusher_tobacco_thread, pusher_paper_thread, pusher_match_thread, smoker_tobacco_thread, smoker_paper_thread, smoker_match_thread;
    
    if(argc != 2)
    {
        printf("Please enter the number of iterations");
        exit(1);
    }
    N = atoi(argv[1]);

    zem_init(&agent_zem, 1);
    zem_init(&tobacco_zem, 0);
    zem_init(&paper_zem, 0);
    zem_init(&match_zem, 0);
    zem_init(&smoker_tobacco_zem, 0);
    zem_init(&smoker_paper_zem, 0);
    zem_init(&smoker_match_zem, 0);
    pthread_mutex_init(&is_var_mutex, NULL);

    pthread_create(&agent_thread, NULL, agent, NULL);
    pthread_create(&pusher_tobacco_thread, NULL, pusher_tobacco, NULL);
    pthread_create(&pusher_paper_thread, NULL, pusher_paper, NULL);
    pthread_create(&pusher_match_thread, NULL, pusher_match, NULL);
    pthread_create(&smoker_tobacco_thread, NULL, smoker_tobacco, NULL);
    pthread_create(&smoker_paper_thread, NULL, smoker_paper, NULL);
    pthread_create(&smoker_match_thread, NULL, smoker_match, NULL);
    
    pthread_join(agent_thread, NULL);
    pthread_join(pusher_tobacco_thread, NULL);
    pthread_join(pusher_paper_thread, NULL);
    pthread_join(pusher_match_thread, NULL);
    pthread_join(smoker_tobacco_thread, NULL);
    pthread_join(smoker_paper_thread, NULL);
    pthread_join(smoker_match_thread, NULL);
    return 0;
}