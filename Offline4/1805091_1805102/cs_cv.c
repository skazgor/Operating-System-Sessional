#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>



int N ;
int num_iterations=0;
pthread_cond_t agent_cv, tobacco_cv, paper_cv, match_cv, smoker_tobacco_cv, smoker_paper_cv, smoker_match_cv;
int agent_indicator, tobaco_indicator,paper_indicator, match_indicator,smoker_tobacco_indicator, smoker_paper_indicator, smoker_match_indicator=0;
int is_paper, is_tobacco, is_match=0;
pthread_mutex_t is_var_mutex;
pthread_mutex_t common_mutex;

void *agent(void *arg)
{
    int num_1, num_2;
    while (1)
    {
        pthread_mutex_lock(&common_mutex);
        if (num_iterations == N)
            break;
        while (!agent_indicator)
        {
        pthread_cond_wait(&agent_cv,&common_mutex);
        }
        
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
            tobaco_indicator=1;
            paper_indicator=1;
            pthread_cond_signal(&tobacco_cv);
            pthread_cond_signal(&paper_cv);
        }
        else if ((num_1 == 0 && num_2 == 2) ||(num_2==0 && num_1==2))
        {
            printf("Agent puts tobacco and match on the table\n");
            tobaco_indicator=1;
            match_indicator=1;
            pthread_cond_signal(&tobacco_cv);
            pthread_cond_signal(&match_cv);
        }
        else if ((num_1 == 1 && num_2 == 2) ||(num_2==1 && num_1==2))
        {
            printf("Agent puts paper and match on the table\n");
            paper_indicator=1;
            match_indicator=1;
            pthread_cond_signal(&paper_cv);
            pthread_cond_signal(&match_cv);
        }
        num_iterations++;
        agent_indicator=0;
        pthread_mutex_unlock(&common_mutex);
    }
    while (!agent_indicator)
    {
        pthread_cond_wait(&agent_cv,&common_mutex);
    }
    num_iterations++;
    tobaco_indicator=paper_indicator=match_indicator=smoker_match_indicator=smoker_paper_indicator=smoker_tobacco_indicator=1;

    pthread_cond_signal(&smoker_match_cv);
    pthread_cond_signal(&smoker_paper_cv);
    pthread_cond_signal(&smoker_tobacco_cv);
    pthread_cond_signal(&tobacco_cv);
    pthread_cond_signal(&paper_cv);
    pthread_cond_signal(&match_cv);
    pthread_mutex_unlock(&common_mutex);
    return NULL;
}
void* pusher_tobacco( void* arg )
{
    while(1)
    {
        pthread_mutex_lock(&common_mutex);
        while (!tobaco_indicator)
        {
            pthread_cond_wait(&tobacco_cv,&common_mutex);
        }
         
        if (num_iterations == N+1){
            pthread_mutex_unlock(&common_mutex);
            break;}
        pthread_mutex_lock(&is_var_mutex);
        if(is_paper)
        {
            is_paper = 0;
            smoker_match_indicator = 1;
            pthread_cond_signal(&smoker_match_cv);
        }
        else if( is_match)
        {
            is_match = 0;
            smoker_paper_indicator = 1;
            pthread_cond_signal(&smoker_paper_cv);
        }
        else
        {
            is_tobacco = 1;
        }
         tobaco_indicator=0;
        pthread_mutex_unlock(&is_var_mutex);
        pthread_mutex_unlock(&common_mutex);
    }
    return NULL;
}
void* pusher_paper( void* arg )
{
    while(1)
    {
        pthread_mutex_lock(&common_mutex);
        while (!paper_indicator)
        {
            pthread_cond_wait(&paper_cv,&common_mutex);
        }
        if (num_iterations == N+1){
            pthread_mutex_unlock(&common_mutex);
            break;}
        pthread_mutex_lock(&is_var_mutex);
        if(is_tobacco)
        {
            is_tobacco = 0;
            smoker_match_indicator = 1;
            pthread_cond_signal(&smoker_match_cv);
        }
        else if( is_match)
        {
            is_match = 0;
            smoker_tobacco_indicator = 1;
            pthread_cond_signal(&smoker_tobacco_cv);
        }
        else
        {
            is_paper = 1;
        }
        paper_indicator=0;
        pthread_mutex_unlock(&is_var_mutex);
        pthread_mutex_unlock(&common_mutex);
    }
    return NULL;
}
void* pusher_match( void* arg )
{
    while(1)
    {
        pthread_mutex_lock(&common_mutex);
        while (!match_indicator)
        {
            pthread_cond_wait(&match_cv,&common_mutex);
        }
        if (num_iterations == N+1){
            pthread_mutex_unlock(&common_mutex);
            break;}
        pthread_mutex_lock(&is_var_mutex);
        if(is_tobacco)
        {
            is_tobacco = 0;
            smoker_paper_indicator = 1;
            pthread_cond_signal(&smoker_paper_cv);
        }
        else if( is_paper)
        {
            is_paper = 0;
            smoker_tobacco_indicator = 1;
            pthread_cond_signal(&smoker_tobacco_cv);
        }
        else
        {
            is_match = 1;
        }
        match_indicator=0;
        pthread_mutex_unlock(&is_var_mutex);
        pthread_mutex_unlock(&common_mutex);
    }
    return NULL;
}
void* smoker_tobacco( void* arg )
{
    while(1)
    {
        pthread_mutex_lock(&common_mutex);
        while (!smoker_tobacco_indicator)
        {
            pthread_cond_wait(&smoker_tobacco_cv,&common_mutex);
        }
        if (num_iterations == N+1){
            pthread_mutex_unlock(&common_mutex);
            break;}
        printf("Smoker with tobacco smokes\n");
        agent_indicator=1;
        pthread_cond_signal(&agent_cv);
        smoker_tobacco_indicator=0;
        pthread_mutex_unlock(&common_mutex);
    }
    return NULL;
}
void* smoker_paper( void* arg )
{
    while(1)
    {
        pthread_mutex_lock(&common_mutex);
        while (!smoker_paper_indicator)
        {
            pthread_cond_wait(&smoker_paper_cv,&common_mutex);
        }
        if (num_iterations == N+1){
            pthread_mutex_unlock(&common_mutex);
            break;}
        printf("Smoker with paper smokes\n");
        agent_indicator=1;
        pthread_cond_signal(&agent_cv);
        smoker_paper_indicator=0;
        pthread_mutex_unlock(&common_mutex);
    }
    return NULL;
}
void* smoker_match( void* arg )
{
    while(1)
    {
        pthread_mutex_lock(&common_mutex);
        while (!smoker_match_indicator)
        {
            pthread_cond_wait(&smoker_match_cv,&common_mutex);
        }
        if (num_iterations == N+1){
            pthread_mutex_unlock(&common_mutex);
            break;}
        printf("Smoker with match smokes\n");
        agent_indicator=1;
        pthread_cond_signal(&agent_cv);
        smoker_match_indicator=0;
        pthread_mutex_unlock(&common_mutex);
    }
    return NULL;
}
int main(int argc, char *argv[])
{
    pthread_t agent_thread;
    pthread_t pusher_tobacco_thread;
    pthread_t pusher_paper_thread;
    pthread_t pusher_match_thread;
    pthread_t smoker_tobacco_thread;
    pthread_t smoker_paper_thread;
    pthread_t smoker_match_thread;
    pthread_mutex_init(&common_mutex,NULL);
    pthread_mutex_init(&is_var_mutex,NULL);
    pthread_cond_init(&agent_cv,NULL);
    pthread_cond_init(&tobacco_cv,NULL);
    pthread_cond_init(&paper_cv,NULL);
    pthread_cond_init(&match_cv,NULL);
    pthread_cond_init(&smoker_tobacco_cv,NULL);
    pthread_cond_init(&smoker_paper_cv,NULL);
    pthread_cond_init(&smoker_match_cv,NULL);
    
    if(argc!=2)
    {
        printf("please enter the number of iterations\n");
        return 0;
    }
    N=atoi(argv[1]);
    if(pthread_create(&agent_thread,NULL,agent,NULL) != 0){
        printf("Error in creating agent thread");
    }
    if(pthread_create(&pusher_tobacco_thread,NULL,pusher_tobacco,NULL) != 0){
        printf("Error in creating pusher tobacco thread");
    }
    if(pthread_create(&pusher_paper_thread,NULL,pusher_paper,NULL) != 0){
        printf("Error in creating pusher paper thread");
    }
    if(pthread_create(&pusher_match_thread,NULL,pusher_match,NULL) != 0){
        printf("Error in creating pusher match thread");
    }
    if(pthread_create(&smoker_tobacco_thread,NULL,smoker_tobacco,NULL) != 0){
        printf("Error in creating smoker tobacco thread");
    }
    if(pthread_create(&smoker_paper_thread,NULL,smoker_paper,NULL) != 0){
        printf("Error in creating smoker paper thread");
    }
    if(pthread_create(&smoker_match_thread,NULL,smoker_match,NULL) != 0){
        printf("Error in creating smoker match thread");
    }

    agent_indicator=1;
    pthread_cond_signal(&agent_cv);
    pthread_join(agent_thread,NULL);
    pthread_join(pusher_tobacco_thread,NULL);
    pthread_join(pusher_paper_thread,NULL);
    pthread_join(pusher_match_thread,NULL);
    pthread_join(smoker_tobacco_thread,NULL);
    pthread_join(smoker_paper_thread,NULL);
    pthread_join(smoker_match_thread,NULL);
    return 0;
}