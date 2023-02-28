
#ifndef _ACCOUNT_H
#define _ACCOUNT_H

#include <iostream>
#include <cstdio>
#include <time.h>
#include <string>
#include <pthread.h> 
#include <stdio.h>
#include <fstream>
#include <time.h>
#include <unistd.h> 
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <list>
#include <sys/types.h>
#include <sys/wait.h>
using namespace std;
extern ofstream outputFile_Handler;
class Account
{
    // This class is implemented as monitor with writers-readers. Secure methods are setBalance() and getBalance().
    // In addition, the destructor is taking all the locks before deleting the instance from the list and destructing the object. 
    private:
        int account_num_;
        string password_;
        int balance_;
        int readers;
        pthread_mutex_t rdLock;
        pthread_mutex_t wrtLock;

    public:
        Account(int num, string pass, int balance) : account_num_(num), password_(pass), balance_(balance), readers(0) {
                // is it ok to initiliaze locks here? safe?
                pthread_mutex_init(&rdLock, NULL);
                pthread_mutex_init(&wrtLock, NULL);
        }
        ~Account()
        {
            pthread_mutex_destroy(&wrtLock);
            pthread_mutex_destroy(&rdLock);
        }

        void lockrdLock()
        {
            int err=pthread_mutex_lock(&rdLock);
            if (err!=0)
            {
                outputFile_Handler<< "mutex of account id <" << account_num_ << "> failed to lock : [%s]" << strerror(err) << endl;
            }
        }
        void lockwrtLock()
        {
            int err=pthread_mutex_lock(&wrtLock);
            if (err!=0)
            {
                outputFile_Handler<< "mutex of account id <" << account_num_ << "> failed to lock : [%s]" << strerror(err) << endl;
            }
        }
        void unlockrdLock()
        {
           int err=pthread_mutex_unlock(&rdLock);
            if (err!=0)
            {
                outputFile_Handler<< "mutex of account id <" << account_num_ << "> failed to unlock : [%s]" << strerror(err) << endl;
            }
        }
        void unlockwrtLock()
        {
           int err=pthread_mutex_unlock(&wrtLock);
            if (err!=0)
            {
                outputFile_Handler<< "mutex of account id <" << account_num_ << "> failed to unlock : [%s]" << strerror(err) << endl;
            }
        }
        void incReaders()
        {
            readers++;
        }
        void decReaders()
        {
            readers--;
        }
        int getReaders()
        {
            return readers;
        }
        bool checkPassword(string pass)
        {
            if (!pass.compare(password_))
                return true;
            return false;
        }
        void setBalance(int balance){
            balance_=balance;
        }
        int getBalance(){
            return balance_;
        }
        int getAcc(){
            return account_num_;
        }
        string getPass(){
            return password_;
        }
        bool operator<(const Account &a) const //enables sorting
        {
        return (account_num_ < a.account_num_);
        }    
};


#endif 