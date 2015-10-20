/* Author: Charles Lovering   */
/* Email : cjlovering@wpi.edu */
/* getfilesinfo */

using namespace std;
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include "getfilesinfo.h"
#include <pthread.h>
#include <semaphore.h>
#include <string>
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include <vector>
#include <sys/time.h>
#include <sys/resource.h>


#define BUFSIZE 1024
#define LINEMAX 256

 
int main(int argc, char *argv[]) {
    /* vars */
    char file[LINEMAX];            /* buffer to be read lines to */
    bool thread = false;           /* boolean on threaded */
    int NUM_THREAD;                /* max number of threads */
    std::vector<char*> FILE_NAMES; /* will hold all file names */

    /* time tracking */
    struct rusage ru;
    struct timeval begin, end;
    gettimeofday(&begin, NULL);


    /* thread arch. : */
    if (argc == 3){
        if (strcmp(argv[1], "thread") == 0){ /* is argv[1] "thread"? */
            thread = true;                   /* use threads!         */    
            NUM_THREAD = atoi(argv[2]);      /* max number of threads*/   
            if(!(NUM_THREAD <= 15 && NUM_THREAD > 0)){ /* valid num? */
                perror("invalid number of threads");
                exit(1);
            }
            /* create semaphore for threads */
            sem_init(&thread_sem, 0, NUM_THREAD); /* create sem that allows a max number of threads to run at a time */
        } else {
            perror("invalid command line args");
            exit(1);
        }
    }

    /* inits the mutexes for global vars */
    init_mutexi();
    int index = 0;

    /* reads input into vector FILE_NAMES */
    while(fgets(file, LINEMAX, stdin) != NULL){
        file[strlen(file) - 1] = '\0';                            /* removes '\n' char */
        char* file_copy = (char*) malloc(sizeof(char) * LINEMAX); /* allocate memory */
        strncpy(file_copy, file, LINEMAX);                        /* copy file name to new char */
        FILE_NAMES.push_back(file_copy);                          /* add new char* to end of vector */
    }

    if (thread){ /* if user wants threads */

        for (int i = 0; i < FILE_NAMES.size(); i++){              /* for each file */
            sem_wait(&thread_sem);                                /* entering 'restricted' region */
            pthread_t t;

            if (pthread_create(&t, NULL, thread_process, (void *)FILE_NAMES[i]) != 0){
                perror("pthread_create");
                exit(1);
            }
        
            THREADS.push_back(t);                                 /* add thread to gloval vector to be waited upon later */
        }

    } else {
        /* serial: */
        for (int i = 0; i < FILE_NAMES.size(); i++){
            if (process(FILE_NAMES[i]) == -1){ /* if negative, bad file! */
                pthread_mutex_lock(&mutex_b);
                BAD_FLS++;
                pthread_mutex_unlock(&mutex_b);
            }
        }
    }
    
    destroy_pthreads();
    print_data();
    gettimeofday(&end, NULL);
    getrusage(RUSAGE_SELF, &ru);
    print_times(begin, end, ru);
}


void *thread_process(void* file_copy){
    char* file_name = (char*) file_copy;  /* cast file name back to a char* */

    if (process(file_name) == -1){        /* file bad? */
        pthread_mutex_lock(&mutex_b);
        BAD_FLS++;
        pthread_mutex_unlock(&mutex_b);
    }

    sem_post(&thread_sem);                /* increment/up sem to allow next/more threads to run. */
    pthread_exit(file_copy);              /* time to split */
}


/* main functionality for getting info on the files */
int process(char* file_name){ 

    struct stat stats;

    if(stat(file_name, &stats) < 0) { /* fills stat struct with info */
        return -1;
    }

    if(S_ISDIR(stats.st_mode) != 0) { /* directory? */
        pthread_mutex_lock(&mutex_d);
        DIRECTS++;
        pthread_mutex_unlock(&mutex_d);
    } else if (S_ISREG(stats.st_mode) != 0){ /* regular? */
        pthread_mutex_lock(&mutex_r);
        REGULAR++;
        pthread_mutex_unlock(&mutex_r);

        /* number of bytes used by a regular file */
        pthread_mutex_lock(&mutex_r);
        R_BYTES += stats.st_size;
        pthread_mutex_unlock(&mutex_r);

        /* text file? */
        unsigned char byte;
        bool text = true;

        int file_open = open(file_name, O_RDONLY);

        while(read(file_open, &byte, sizeof(byte)) > 0){ /* checks if file is a text file */
          if (isspace(byte) || isprint(byte)){           /* either space or printable? */
            continue;
        }  else {
            text = false;
            break;
            }
        }

        close(file_open);

        if (text){ /* text file ? */
            
            pthread_mutex_lock(&mutex_r);
            ALL_TXT++; /* + 1 text files */
            pthread_mutex_unlock(&mutex_r);

            pthread_mutex_lock(&mutex_at);
            T_BYTES += stats.st_size; /* add to size */
            pthread_mutex_unlock(&mutex_at); 
        }

    }  else { /* not regular or a directory, must be real special... */

        pthread_mutex_lock(&mutex_s);
        SPECIAL++;
        pthread_mutex_unlock(&mutex_s);
    }
    
    return 0;
}

void print_data(){

    cout << "Bad Files: " << BAD_FLS << endl;
    cout << "Directories: " << DIRECTS << endl;
    cout << "Regular Files: " << REGULAR << endl;
    cout << "Special Files: " << SPECIAL << endl;
    cout << "Regular File Bytes: " << R_BYTES << endl;
    cout << "Text Files: " << ALL_TXT << endl;
    cout << "Text File Bytes: " << T_BYTES << endl;
}

void init_mutexi(){
        
        /* create the mutex */
    if (pthread_mutex_init(&mutex_b, NULL) < 0) {
        perror("pthread_mutex_init");
        exit(1);
    }
        /* create the mutex */
    if (pthread_mutex_init(&mutex_d, NULL) < 0) {
        perror("pthread_mutex_init");
        exit(1);
    }
        /* create the mutex */
    if (pthread_mutex_init(&mutex_r, NULL) < 0) {
        perror("pthread_mutex_init");
        exit(1);
    }
        /* create the mutex */
    if (pthread_mutex_init(&mutex_s, NULL) < 0) {
        perror("pthread_mutex_init");
        exit(1);
    }
        /* create the mutex */
    if (pthread_mutex_init(&mutex_rb, NULL) < 0) {
        perror("pthread_mutex_init");
        exit(1);
    }
        /* create the mutex */
    if (pthread_mutex_init(&mutex_at, NULL) < 0) {
        perror("pthread_mutex_init");
        exit(1);
    }
        /* create the mutex */
    if (pthread_mutex_init(&mutex_tb, NULL) < 0) {
        perror("pthread_mutex_init");
        exit(1);
    }
}

void destroy_pthreads(){
    pthread_mutex_destroy(&mutex_b);
    pthread_mutex_destroy(&mutex_d);
    pthread_mutex_destroy(&mutex_r);
    pthread_mutex_destroy(&mutex_s);
    pthread_mutex_destroy(&mutex_rb);
    pthread_mutex_destroy(&mutex_at);
    pthread_mutex_destroy(&mutex_tb);

    /* waits on any threads not yet completed - returning immediately on those that have*/
    for (int i = 0; i < THREADS.size(); i++){
        pthread_join(THREADS.at(i), NULL);
    }
}

void print_times(struct timeval begin, struct timeval end, struct rusage ru){
  printf("User Time: %ld [ms]\n", (ru.ru_utime.tv_sec * 1000 + (ru.ru_utime.tv_usec / 1000)));
  printf("System Time: %ld [ms]\n", (ru.ru_stime.tv_sec * 1000 + (ru.ru_stime.tv_usec / 1000)));
  printf("WallClockTime: %ld [ms]\n", ((end.tv_sec - begin.tv_sec) * 1000) + ((end.tv_usec - begin.tv_usec) / 1000));
}

