#ifndef _ATMS_H
#define _ATMS_H

#include <iostream>
#include <time.h>
#include "account.h"
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h> 
#include <stdio.h>
#include <cstring>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <string>
#include <vector>
#include <sstream> 
#include <list>
#include <fstream>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;
void lock(pthread_mutex_t m);
void unlock(pthread_mutex_t m);
Account* get_acc_by_accNum(int acc);
void delete_acc_by_accNum(int acc);
void* run(void* dataIn);

struct threadData{
    char* path;
    int id;
    ifstream InputFile;
};

#endif 