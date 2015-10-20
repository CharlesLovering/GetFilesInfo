#ifndef _PROJ_INCLUDED__   // if x.h hasn't been included yet...
#define _PROJ_INCLUDED__   //   #define this so the compiler knows it has been included

/* getfilesinfo header file */

/* imports */
#include <semaphore.h>
#include <vector>

/* function prototypes*/
int process(char* file_name);
void *thread_process(void* file_copy);
void print_times(struct timeval begin, struct timeval end, struct rusage ru);
void print_data();
void init_mutexi();
void destroy_pthreads();

/* globals, info on the files */
int BAD_FLS = 0;
int DIRECTS = 0;
int REGULAR = 0;
int SPECIAL = 0;
int R_BYTES = 0;
int ALL_TXT = 0;
int T_BYTES = 0;

std::vector<pthread_t> THREADS;

/* mutexi for global data */
pthread_mutex_t mutex_b; 
pthread_mutex_t mutex_d; 
pthread_mutex_t mutex_r; 
pthread_mutex_t mutex_s; 
pthread_mutex_t mutex_rb; 
pthread_mutex_t mutex_at; 
pthread_mutex_t mutex_tb; 

pthread_mutex_t mutex; /* general */

/* semaphore for threads */
sem_t thread_sem; 

#endif
