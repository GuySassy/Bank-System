#include <pthread.h>
#include <semaphore.h>
#include <iostream>
#include <fstream>
#include "atms.h"
#include "account.h"
#include <stdio.h>
#include <sstream> 
#include <time.h>
#include <vector>
#include <cstring>
#include <unistd.h> 
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <list>
#include <sys/types.h>
#include <sys/wait.h>
#include <cmath>

using namespace std;

ofstream outputFile_Handler;

// declaration and init of accounts list and its locks 
list <Account> accounts;
pthread_mutex_t listRdLock;
pthread_mutex_t listWrtLock;
pthread_mutex_t logLock;
pthread_mutex_t lockCheck; // for atms input lock - check if needed
int listReaders;
int waiting;

// init for bank account
string BankPass = "1234";
Account BankAcc(0, BankPass, 0); // bank account has id 0 and 0 balance at the init


//******************************************************************************************************//
//******************************************************************************************************//
// function name: Commisions
// Description: Commisions charge of every account
// Parameters: stop flag
//********************************************
void* Commisions(void* stopFlag)
{
    int* stop = (int*)stopFlag;
    while (*stop)
    {
        sleep(3);
        if (*stop == 0) pthread_exit(NULL);
        int percentage = 2 + ( std::rand() % 3 );
        int amount, bal, bankBal, accId;
        std::list<Account>::iterator it;

        // entry as reader to the list
        pthread_mutex_lock(&listRdLock);
        listReaders++;
        if (listReaders == 1)
        {
            pthread_mutex_lock(&listWrtLock);
        }
        pthread_mutex_unlock(&listRdLock);
        // list level critical section - reader

        // entry as writer to the list
        //pthread_mutex_lock(&listWrtLock);
        // list level critical section - writer
        //pthread_mutex_lock(&logLock);
        //outputFile_Handler << "Commisions trying to get lock as writer from some account and bank" << endl;
        //pthread_mutex_unlock(&logLock);
	    for (it = accounts.begin(); it != accounts.end(); it++) 
        {
            
            it->lockwrtLock();
            BankAcc.lockwrtLock();
            bal = it->getBalance();
            accId = it->getAcc();
            bankBal = BankAcc.getBalance();
	    	amount = (int)round( ((double)bal * percentage) / 100);
            it -> setBalance(bal - amount);
            BankAcc.setBalance(bankBal + amount);
            pthread_mutex_lock(&logLock);
            outputFile_Handler << "Bank: commissions of <" << percentage << "> % were charged, the bank gained <" << amount << 
            "> $ from account <" << accId <<">" << endl;
            pthread_mutex_unlock(&logLock);
            it->unlockwrtLock();
            BankAcc.unlockwrtLock();
        }
        // exit as writer of the list
        //pthread_mutex_unlock(&listWrtLock);
        // exit as reader of the list
        pthread_mutex_lock(&listRdLock);
        listReaders--;
        if (listReaders==0)
        {
            pthread_mutex_unlock(&listWrtLock);
        }
        pthread_mutex_unlock(&listRdLock);
    }
    pthread_exit(NULL);
}

// function name: Commisions
// Description: Commisions charge of every account
// Parameters: stop flag
//********************************************
void* bankStatus(void* stopFlag)
{
    int* stop = (int*)stopFlag;
    while (*stop)
    {
        usleep(500000);
        if (*stop == 0) pthread_exit(NULL);
        int bal,accId,bankBal;
        string accPass;
        std::list<Account>::iterator it;
        // enter as writer to list
        //pthread_mutex_lock(&listWrtLock);
        // list level critical section - writer
        // entry as reader to the list
        pthread_mutex_lock(&listRdLock);
        listReaders++;
        if (listReaders == 1)
        {
            pthread_mutex_lock(&listWrtLock);
        }
        pthread_mutex_unlock(&listRdLock);
        // list level critical section - reader
        accounts.sort();
        printf("\033[2J");
        printf("\033[1;1H");
        printf("Current Bank Status\n");
        for (it = accounts.begin(); it != accounts.end(); it++) 
        {
            //enter as reader to account
            it->lockrdLock();
            it->incReaders();
            if (it->getReaders()==1) it->lockwrtLock();
            it->unlockrdLock();
            // account level critical section
            bal = it->getBalance();
            accId = it-> getAcc();
            accPass = it->getPass();
            printf("Account %d: Balance - %d $ , Account Password - %s\n",accId,bal,accPass.c_str());
            // exit as reader from account
            it->lockrdLock();
            it->decReaders();
            if (it->getReaders()==0) it->unlockwrtLock();
            it->unlockrdLock();
        }
        //enter as reader to account
        BankAcc.lockrdLock();
        BankAcc.incReaders();
        if (BankAcc.getReaders()==1) BankAcc.lockwrtLock();
        BankAcc.unlockrdLock();
        // account level critical section
        bankBal = BankAcc.getBalance();
        printf("The Bank has %d $\n", bankBal);
        // exit as reader from account
        BankAcc.lockrdLock();
        BankAcc.decReaders();
        if (BankAcc.getReaders()==0) BankAcc.unlockwrtLock();
        BankAcc.unlockrdLock();
        // exit as writer from list
        //pthread_mutex_unlock(&listWrtLock);
        // exit as reader of the list
        pthread_mutex_lock(&listRdLock);
        listReaders--;
        if (listReaders==0)
        {
            pthread_mutex_unlock(&listWrtLock);
        }
        pthread_mutex_unlock(&listRdLock);
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    // output file open
    outputFile_Handler.open("log.txt");
    // init the list locks
    pthread_mutex_init(&listRdLock, NULL);
    pthread_mutex_init(&listWrtLock, NULL);
    pthread_mutex_init(&logLock, NULL);
    pthread_mutex_init(&lockCheck, NULL);
    // get input
    if (argc < 3) {
		cerr << "illegal arguments" << endl;
		exit(-1);
	}

	int ATMS_NUM = 0;
	try {
		ATMS_NUM = stoi(string(argv[1]));
	} catch(...) {
		cerr << "illegal arguments" << endl;
		exit(-1);
	}

	if (ATMS_NUM != argc-2) { // argc >= 3
		cerr << "illegal arguments" << endl;
		exit(-1);
	}
    int err;
    int stopFlag=1;
    // assign two threads for commisions and status
    pthread_t* bank_threads = new pthread_t[2];
    err = pthread_create(&bank_threads[0], NULL, bankStatus, (void*)&stopFlag);
    if (err != 0)
        {
            cerr << "bankStatus thread failed to create : " << strerror(err) << endl;
			exit(-1);
        }
    err = pthread_create(&bank_threads[1], NULL, Commisions, (void*)&stopFlag);
    if (err != 0)
        {
            cerr << "Commision thread failed to create : " << strerror(err) << endl;
			exit(-1);
        }
    //assign files to atms by threads
    pthread_t* tid = new pthread_t[ATMS_NUM]; // array of threads 
    threadData* threads_data = new threadData[ATMS_NUM];
    for (int i=0;i<ATMS_NUM;i++)
    {
        threads_data[i].path = argv[i+2];
        threads_data[i].id = i+1;
        threads_data[i].InputFile.open(argv[i+2]);
        err = pthread_create(&tid[i], NULL, run, (void*)&threads_data[i]);
        if (err != 0)
        {
            cerr << "Thread failed to create : " << strerror(err) << endl;
			exit(-1);
        }
    }
    // Wait for threads to finish:
	for (int i = 0; i < ATMS_NUM; i++) {
		if (pthread_join(tid[i], NULL) != 0) {
		    cerr << "atms threads pthread_join() failure : " << strerror(errno) << endl;
		    exit(-1);
		}
	}
    stopFlag=0; // now the two bank threads will stop
    for (int i=0; i < 2 ; i++){
        if(pthread_join(bank_threads[i], NULL) !=0) {
            cerr << "bank threads pthread_join() failure : " << strerror(errno) << endl;
			exit(-1);
        }
    }
    // File Close
    outputFile_Handler.close();
    return 0;
}