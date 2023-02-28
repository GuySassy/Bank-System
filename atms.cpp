#include "atms.h"

using namespace std;
extern list <Account> accounts;
// locks for list declaration and init
extern pthread_mutex_t listRdLock;
extern pthread_mutex_t listWrtLock;
extern pthread_mutex_t logLock;
extern pthread_mutex_t lockCheck;
extern int listReaders;
extern Account BankAcc;
extern ofstream outputFile_Handler;


//*******************************************
// Function name: lock
// Description: Locks the lock given as input
// Parameters: pthread_mutex_t m
//*******************************************
void lock(pthread_mutex_t m)
{
    int err=pthread_mutex_lock(&m);
    if (err!=0)
    {
        cerr<< "mutex failed to lock : [%s]" << strerror(err) << endl;
        exit(-1);
    }
}

//*******************************************
// Function name: unlock
// Description: Unlocks the lock given as input
// Parameters: pthread_mutex_t m
//*******************************************
void unlock(pthread_mutex_t m)
{
    int err=pthread_mutex_unlock(&m);
    if (err!=0)
    {
        cerr<< "mutex failed to unlock : [%s]" << strerror(err) << endl;
        exit(-1);
    }
}

//*******************************************
// Function name: get_acc_by_accNum
// Description: Finds the account with accNum as id
// Parameters: account number. returns pointer to the node
//*******************************************
Account* get_acc_by_accNum(int acc)
{
    std::list<Account>::iterator it;
    Account* ptr = NULL;
	for (it = accounts.begin(); it != accounts.end(); it++) 
    {
		if (it->getAcc()==acc)
            ptr = &(*it);
    }
    return ptr;
}
//********************************************
// function name: delete_acc_by_accNum
// Description: Deletes an account by id
// Parameters: acc id
//********************************************
void delete_acc_by_accNum(int acc)
{
    std::list<Account>::iterator it;
	for (it = accounts.begin(); it != accounts.end(); it++) 
    {
		if (it->getAcc()==acc)
        {
            accounts.erase(it);
            break;
        }
            
    }
}

//********************************************
// function name: O
// Description: Opens a new account
// Parameters: account number, password and amount.
//********************************************
void O(int accNum, string pass, int init_balance, int ATM_ID)
{
    if (get_acc_by_accNum(accNum)==NULL)
    {
        // list level writer implemented as macro (from calling - run)
        Account acc(accNum, pass, init_balance);
        accounts.push_back(acc);
        pthread_mutex_lock(&logLock);
        outputFile_Handler << "<" << ATM_ID << ">: New account id is <"<< accNum <<"> with password <"<< pass <<"> and initial balance <"<< init_balance << ">" << endl;
        pthread_mutex_unlock(&logLock);
    }
    else {
        pthread_mutex_lock(&logLock);
        outputFile_Handler << "Error <" << ATM_ID <<">: Your transaction failed - account with the same id exists" << endl;
        pthread_mutex_unlock(&logLock);
    }
}
//********************************************
// function name: D
// Description: Deposit amount to the account balance
// Parameters: account number, password and amount.
//********************************************
void D(int accNum, string pass, int amount, int ATM_ID)
{
    Account* curr_acc = get_acc_by_accNum(accNum);
    if (curr_acc==NULL)
    {
        pthread_mutex_lock(&logLock);
        outputFile_Handler << "Error <" << ATM_ID << ">: Your transaction failed -  Account id <"<< accNum <<"> does not exist" << endl;
        pthread_mutex_unlock(&logLock);   
    }
    else
    {
        if (!curr_acc->checkPassword(pass))
        {
            pthread_mutex_lock(&logLock);
            outputFile_Handler << "Error <" << ATM_ID << ">: Your transaction failed – password for account id <"<< accNum <<"> is incorrect" << endl;
            pthread_mutex_unlock(&logLock);
        }
        else 
        {
            // enter as writer to account
            curr_acc->lockwrtLock();
            // account level critical section
            int new_balance = curr_acc->getBalance()+amount;
            curr_acc->setBalance(new_balance);
            pthread_mutex_lock(&logLock);
            outputFile_Handler << "<" << ATM_ID << ">: Account <" << accNum << "> new balance is <" << new_balance << "> after <" << amount << "> $ was deposited" << endl;
            pthread_mutex_unlock(&logLock);
            // exit as writer from account
            curr_acc->unlockwrtLock();
        }
    }
}
//********************************************
// function name: W
// Description: Withdraw an amount from account
// Parameters: account number, password and amount.
//********************************************
void W(int acc, string pass, int amount, int ATM_ID)
{
    Account* curr_acc = get_acc_by_accNum(acc);
    if (curr_acc==NULL)
    {
        pthread_mutex_lock(&logLock);
        outputFile_Handler << "Error <" << ATM_ID << ">: Your transaction failed -  Account id <"<< acc <<"> does not exist" << endl;
        pthread_mutex_unlock(&logLock);
    }
    else
    {
        if (!curr_acc->checkPassword(pass))
        {
            pthread_mutex_lock(&logLock);
            outputFile_Handler << "Error <" << ATM_ID << ">: Your transaction failed – password for account id <"<< acc << "> is incorrect" << endl;
            pthread_mutex_unlock(&logLock);
        }
        else 
        {
            // enter as reader to account
            curr_acc->lockwrtLock();
            //  account level critical section
            int new_balance = curr_acc->getBalance()-amount;
            if (new_balance < 0)
            {
                pthread_mutex_lock(&logLock);
                outputFile_Handler << "Error <" << ATM_ID << ">: Your transaction failed – account id <"<< acc << "> balance is lower than <" << amount << ">" << endl;
                pthread_mutex_unlock(&logLock);
                // exit as writer from account
                curr_acc->unlockwrtLock();
            }
            else {
                curr_acc->setBalance(new_balance);
                pthread_mutex_lock(&logLock);
                outputFile_Handler << "<" << ATM_ID << ">: Account <" << acc << "> new balance is <" << new_balance << "> after <" << amount << "> $ was withdrew" << endl;
                pthread_mutex_unlock(&logLock);
                // exit as writer from account
                curr_acc->unlockwrtLock();
            }
        }
    }
}
//********************************************
// function name: B - MIGHT BE PROBLEMATIC
// Description: Balance check of an account
// Parameters: account number, password.
//********************************************
void B(int acc, string pass, int ATM_ID)
{
    Account* curr_acc = get_acc_by_accNum(acc);
    if (curr_acc==NULL)
    {
        pthread_mutex_lock(&logLock);
        outputFile_Handler << "Error <" << ATM_ID << ">: Your transaction failed -  Account id <"<< acc <<"> does not exist" << endl;
        pthread_mutex_unlock(&logLock);
    }
    else
    {
        if (!curr_acc->checkPassword(pass))
        {
            pthread_mutex_lock(&logLock);
            outputFile_Handler << "Error <" << ATM_ID << ">: Your transaction failed – password for account id <"<< acc << "> is incorrect" << endl;
            pthread_mutex_unlock(&logLock);
        }
        else 
        {
            //enter as reader to account
            curr_acc->lockrdLock();
            curr_acc->incReaders();
            if (curr_acc->getReaders()==1) curr_acc->lockwrtLock();
            curr_acc->unlockrdLock();
            // account level critical section
            int balance = curr_acc -> getBalance();
            pthread_mutex_lock(&logLock);
            outputFile_Handler << "<" << ATM_ID << ">: Account <"<< acc <<"> balance is <" << balance << ">" << endl;
            pthread_mutex_unlock(&logLock);
            // exit as reader from account
            curr_acc->lockrdLock();
            curr_acc->decReaders();
            if (curr_acc->getReaders()==0) curr_acc->unlockwrtLock();
            curr_acc->unlockrdLock();
        }
    }
}
//********************************************
// function name: Q
// Description: Delete an account
// Parameters: account number, password.
//********************************************
void Q(int acc, string pass, int ATM_ID) // STILL IN PROGRESS
{
    Account* curr_acc = get_acc_by_accNum(acc);
    if (curr_acc==NULL)
    {
        pthread_mutex_lock(&logLock);
        outputFile_Handler << "Error <" << ATM_ID << ">: Your transaction failed -  Account id <"<< acc <<"> does not exist" << endl;
        pthread_mutex_unlock(&logLock);
    }
    else
    {
        if (!curr_acc->checkPassword(pass))
        {
            pthread_mutex_lock(&logLock);
            outputFile_Handler << "Error <" << ATM_ID << ">: Your transaction failed – password for account id <"<< acc << "> is incorrect" << endl;
            pthread_mutex_unlock(&logLock);
        }
        else
        {
            // enter as writer to list is implemented as macro (calling by run)
            // enter as writer to account
            curr_acc->lockwrtLock();
            // account level critical section - writer
            int bal=curr_acc->getBalance();
            curr_acc->~Account();
            delete_acc_by_accNum(acc);
            accounts.sort();
            pthread_mutex_lock(&logLock);
            outputFile_Handler << "<" << ATM_ID <<">: Account <" << acc << "> is now closed. Balance was <" << bal << ">" << endl;
            pthread_mutex_unlock(&logLock);
            // exit as writer from account
            curr_acc->unlockwrtLock();
        }
    }
}

//********************************************
// function name: T
// Description: Applying transaction between two accounts
// Parameters: account number, password, target account and amount.
//********************************************
void T(int acc, string pass, int target, int amount, int ATM_ID)
{
    Account* curr_acc = get_acc_by_accNum(acc);
    Account* target_acc = get_acc_by_accNum(target);
    if (curr_acc==NULL)
    {
        pthread_mutex_lock(&logLock);
        outputFile_Handler << "Error <" << ATM_ID << ">: Your transaction failed -  Account id <"<< acc <<"> does not exist" << endl;
        pthread_mutex_unlock(&logLock);
    }
    else{
        if (target_acc==NULL)
        {
            pthread_mutex_lock(&logLock);
            outputFile_Handler << "Error <" << ATM_ID << ">: Your transaction failed -  Account id <"<< target_acc <<"> does not exist" << endl;
            pthread_mutex_unlock(&logLock);
        }
        else
        {
            if (!curr_acc->checkPassword(pass))
            {
                pthread_mutex_lock(&logLock);
                outputFile_Handler << "Error <" << ATM_ID << ">: Your transaction failed – password for account id <"<< acc<< "> is incorrect" << endl;
                pthread_mutex_unlock(&logLock);
            }
            else
            {
                // enter as writer to both account - bigger id first
                if (acc>target) 
                {
                    curr_acc->lockwrtLock();
                    target_acc->lockwrtLock();
                }
                else
                {
                    target_acc->lockwrtLock();
                    curr_acc->lockwrtLock();
                }
                // accounts level critical section
                int new_balance = curr_acc->getBalance()-amount;
                int target_new_balance = target_acc->getBalance()+amount;
                if (new_balance < 0)
                {
                    pthread_mutex_lock(&logLock);
                    outputFile_Handler << "Error <" << ATM_ID << ">: Your transaction failed – account id <"<< acc << "> balance is lower than <" << amount << ">" << endl;
                    pthread_mutex_unlock(&logLock);
                }
                else {
                    curr_acc->setBalance(new_balance);
                    target_acc->setBalance(target_new_balance);
                    pthread_mutex_lock(&logLock);
                    outputFile_Handler << "<" << ATM_ID << ">: Transfer <" << amount << "> from account <" << acc << "> to account <" << target << "> new account balance is <" << new_balance << "> new target account balance is <" << target_new_balance << ">" << endl;
                    pthread_mutex_unlock(&logLock);
                    
                }
                // exit as writer from both account - bigger id first
                if (acc>target)
                {
                    target_acc->unlockwrtLock();
                    curr_acc->unlockwrtLock();
                }
                else
                {
                    curr_acc->unlockwrtLock();
                    target_acc->unlockwrtLock();
                }
            }
        }
    }
}

//********************************************
// function name: run
// Description: The function that runs when a thread is created for an ATM
// Parameters: struct threadData with two inner parameters - file path and atm id.
// look at theardData sturct in atms.h
//********************************************
void* run(void* dataIn)
{
    struct threadData *data = (struct threadData*)dataIn;
    int id = (*data).id;
    if (!(*data).InputFile){
        cerr << "Path to ATM <" << id << "> is invalid" << endl;
        exit(-1);
    }
    string line;
    char delimiters[] = " \t\n";
    while(getline((*data).InputFile, line))
    {
        pthread_mutex_unlock(&lockCheck);
        if (strlen(line.c_str()) == 0){
            pthread_exit(NULL);
        }
        char* tokLine = (*data).path;
        strcpy(tokLine, line.c_str());
        string passArg = "JUST TO INIT";
        pthread_mutex_lock(&lockCheck);
        char* cmd = strtok(tokLine, delimiters);
        if (cmd == NULL){
            cerr << "Missing function argument in ATM " << id << endl;
            exit(-1);
        }
        string token;
        vector<string> tokens;
        stringstream line_stream(line);
        while(getline(line_stream, token, ' ')) {
        	tokens.push_back(token);
        }
        int accId = stoi(tokens[1]);
        int amount, targetAcc = 0;
        passArg = tokens[2];
        if (!strcmp(cmd, "T")) {
            targetAcc = stoi(tokens[3]); 
            amount = stoi(tokens[4]);
        }
        else if (!strcmp(cmd, "B") || !strcmp(cmd, "Q")) 
        {
            amount = 0;
        }
        else{
            amount = stoi(tokens[3]);
        }
        pthread_mutex_unlock(&lockCheck);
        
        if (!strcmp(cmd, "O")) 
        {
            // enter as writer to list
            pthread_mutex_lock(&listWrtLock);
            // list level critical section - writer
            sleep(1);
            O(accId, passArg, amount, id);
            // exit as writer from list
            pthread_mutex_unlock(&listWrtLock);
        }
        else if (!strcmp(cmd, "D")){
            pthread_mutex_lock(&listRdLock);
            listReaders++;
            if (listReaders == 1)
            {
                pthread_mutex_lock(&listWrtLock);
            }
            pthread_mutex_unlock(&listRdLock);
            sleep(1);
            D(accId, passArg, amount, id);
            pthread_mutex_lock(&listRdLock);
            listReaders--;
            if (listReaders==0)
            {
                pthread_mutex_unlock(&listWrtLock);
            }
            pthread_mutex_unlock(&listRdLock);
        } 
        else if (!strcmp(cmd, "W"))
        {
            pthread_mutex_lock(&listRdLock);
            listReaders++;
            if (listReaders == 1)
            {
                pthread_mutex_lock(&listWrtLock);
            }
            pthread_mutex_unlock(&listRdLock);
            sleep(1);
            W(accId, passArg, amount, id);
            pthread_mutex_lock(&listRdLock);
            listReaders--;
            if (listReaders==0)
            {
                pthread_mutex_unlock(&listWrtLock);
            }
            pthread_mutex_unlock(&listRdLock);
        } 
        else if (!strcmp(cmd, "B")) {
            pthread_mutex_lock(&listRdLock);
            listReaders++;
            if (listReaders == 1)
            {
                pthread_mutex_lock(&listWrtLock);
            }
            pthread_mutex_unlock(&listRdLock);
            sleep(1);
            B(accId, passArg, id);
            pthread_mutex_lock(&listRdLock);
            listReaders--;
            if (listReaders==0)
            {
                pthread_mutex_unlock(&listWrtLock);
            }
            pthread_mutex_unlock(&listRdLock);
            
        }
        else if (!strcmp(cmd, "Q")) {
            // enter as writer to list
            pthread_mutex_lock(&listWrtLock);
            // list level critical section - writer
            sleep(1);
            Q(accId, passArg, id);
            // exit as writer from list
            pthread_mutex_unlock(&listWrtLock);
        }
        else if (!strcmp(cmd, "T")) {
            pthread_mutex_lock(&listRdLock);
            listReaders++;
            if (listReaders == 1)
            {
                pthread_mutex_lock(&listWrtLock);
            }
            pthread_mutex_unlock(&listRdLock);
            sleep(1);
            T(accId, passArg, targetAcc, amount, id);
            pthread_mutex_lock(&listRdLock);
            listReaders--;
            if (listReaders==0)
            {
                pthread_mutex_unlock(&listWrtLock);
            }
            pthread_mutex_unlock(&listRdLock);
        }
        usleep(100000);
        pthread_mutex_lock(&lockCheck);
    }
    pthread_mutex_unlock(&lockCheck);
    pthread_exit(NULL);
}   