#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
/**
 * Description: multi-threaded programming and locking shared variables are used to print the unique names and their number of occurences in exactly 2 txt files..
 * Author names: Preet LNU, Nahal Bagheri
 * Author emails: preet.lnu@sjsu.edu, nahal.bagheri@sjsu.edu
 * Last modified date: 12/02/2022
 * Creation date: 11/25/2022
 **/

//thread mutex lock for access to the log index
//TODO you need to use this mutexlock for mutual exclusion
//when you print log messages from each thread
pthread_mutex_t tlock1 = PTHREAD_MUTEX_INITIALIZER;
//thread mutex lock for critical sections of allocating THREADDATA
//TODO you need to use this mutexlock for mutual exclusion
pthread_mutex_t tlock2 = PTHREAD_MUTEX_INITIALIZER;
//thread mutex lock for access to the name counts data structure
//TODO you need to use this mutexlock for mutual exclusion
pthread_mutex_t tlock3 = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t tlock4 = PTHREAD_MUTEX_INITIALIZER;

void* thread_runner(void*);
pthread_t tid1, tid2;
//struct points to the thread that created the object.
//This is useful for you to know which is thread1. Later thread1 will also deallocate.
struct THREADDATA_STRUCT
{
    pthread_t creator;
};
typedef struct THREADDATA_STRUCT THREADDATA;
THREADDATA* p = NULL;

//node that contains string, counts and next pointer
typedef struct node_struct
{
    char str[30];
    int counts;
    struct node_struct* nextPtr;
}node_struct;
/**
 * typedef of node_struct
 */
typedef struct node_struct TRACE_NODE;

//listhead of linkedlist
static TRACE_NODE *head;

void pushNode(char * strLine);
void PrintNodes(TRACE_NODE *currentNode);
void freeList(TRACE_NODE *currentNode);

//variable for indexing of messages by the logging function.
int logindex=0;
int *logip = &logindex;

/**
 * This function checks the duplicate in the list
 * Input parameters: char * strLine
 * Returns: int
**/
int checkDuplicate(char *strLine)
{
    TRACE_NODE *temp = head;        //temporary variable

    //This while loop checks if the lines are same or not
    while(temp != NULL)
    {
        if(strcmp(temp->str, strLine) == 0)
        {
            temp->counts+=1;
            return 1;
        }
        else
        {
            temp = temp->nextPtr;
        }
    }
    return 0;
}

/**
 * This function pushes the node to the linkedlist
 * Input parameters: char * strLine
 * Returns: -
**/
void pushNode(char * strLine)
{
    if(checkDuplicate(strLine) == 1)
    {
        return;
    }

    TRACE_NODE *addNode = (TRACE_NODE*) malloc(sizeof(TRACE_NODE));
    strcpy(addNode->str, strLine);
    addNode->counts = 1;
    addNode->nextPtr = NULL;
    if(head == NULL)
    {
        head = addNode;
        head->nextPtr = NULL;
    }
    else
    {
        TRACE_NODE *temp = head;
        while(temp)
        {
            if(temp->nextPtr != NULL)
            {
                temp = temp->nextPtr;
            }
            else
            {
                break;
            }
        }
        temp->nextPtr = addNode;
    }
}

/**
 * This function reads command lines in stdin and execute them in parallel and then print the output in files
 * Input parameters: TRACE_NODE *currentNode
 * Returns: -
**/
void PrintNodes(TRACE_NODE *currentNode)
{
    if (currentNode == NULL)
    {
        return;
    }

    //Printing the node
    if(currentNode->str != NULL)
    {
        printf("%s: %d \n", currentNode->str, currentNode->counts);
    }

    PrintNodes(currentNode->nextPtr);

}

/**
 * This function recursively frees the list
 * Input parameters: TRACE_NODE *currentNode
 * Returns: -
**/
void freeList(TRACE_NODE *currentNode)
{
    if(currentNode == NULL)
    {
        return;
    };

    //recirsively freeing the node
    freeList(currentNode->nextPtr);
    free(currentNode);
}

/*********************************************************
// function main
*********************************************************/
int lineIndex = 0;
char lineStr[100][30];
int main(int argc, char *argv[])
{
    //TODO similar interface as A2: give as command-line arguments three filenames of numbers (the numbers in the files are newline-separated).
    if(argc > 3 || argc < 3)
    {
        printf("Please only enter two fies\n");
        exit(0);
    }

    head = NULL;
    printf("***************************************************** Log Messages *****************************************************\n");
    printf("create first thread\n");
    pthread_create(&tid1,NULL,thread_runner,argv[1]);

    printf("create second thread\n");
    pthread_create(&tid2,NULL,thread_runner,argv[2]);

    printf("wait for first thread to exit\n");
    pthread_join(tid1,NULL);
    printf("first thread exited\n");
    printf("wait for second thread to exit\n");
    pthread_join(tid2,NULL);
    printf("second thread exited\n");
    //TODO print out the sum variable with the sum of all the numbers
    printf("\n");
    printf("***************************************************** Name Counts *****************************************************\n");
    PrintNodes(head);           //prints the head whihch has all the names and number of occurences of a name.

    //This for loop is used to print the names
    for(int i = 0; i < lineIndex;i++)
    {
        printf("%s\n", lineStr[i]);
    }

    //freeing the head of the list
    freeList(head);

    exit(0);
}//end main

/**********************************************************************
// function thread_runner runs inside each thread
**********************************************************************/
void* thread_runner(void* x)
{
    time_t currentTime;
    time(&currentTime);
    char strLine[30];                   //has all the unique names
    char fullLine[50];                  //mainly used to print errors
    char time[50];                      //has the current time.

    pthread_t me;
    me = pthread_self();

    sprintf(time, "%s", ctime(&currentTime));
    time[strcspn(time, "\n")] = 0;

    logindex = logindex + 1;
    printf("logindex %d, thread %ld, pid %d, %s. This is thread %ld (p=%p)\n",logindex, me, getpid(), time, me, p);
    logindex = logindex + 1;

    // critical section starts
    pthread_mutex_lock(&tlock2);
    if (p == NULL)
    {
        p = (THREADDATA*) malloc(sizeof(THREADDATA));
        p->creator=me;
    }

    pthread_mutex_unlock(&tlock2);  // critical section ends
    if (p != NULL && p->creator == me)
    {
        printf("logindex %d, thread %ld, pid %d, %s. This is thread %ld and I created THREADDATA %p\n",logindex, me, getpid(), time, me, p);
    }
    else
    {
        printf("logindex %d, thread %ld, pid %d, %s. This is thread %ld and I can access the THREADDATA %p\n",logindex, me, getpid(), time, me, p);
    }
    /**
     * //TODO implement any thread name counting functionality you need.
     * Assign one fp per thread. Hint: you can either pass each argv filename as a
  thread_runner argument from main.
     * Or use the logindex to index argv, since every thread will increment the
  logindex anyway
     * when it opens a fp to print a log message (e.g. logindex could also index
  argv)....
     * //Make sure to use any mutex locks appropriately
     */
    pthread_mutex_lock(&tlock1);
    logindex = logindex + 1;
    printf("logindex %d, thread %ld, pid %d, %s opened file %s \n",logindex, me, getpid(),time, (char*) x);
    pthread_mutex_unlock(&tlock1);

    //modified assignment 2 code
    FILE *fp = fopen((char*)x, "r");
    if(fp != NULL)
    {
        int count = 0;
        while(fgets(strLine, 30, fp ))
        {
            pthread_mutex_lock(&tlock3);
            if(strcmp(strLine, " ") < 0)
            {
                sprintf(fullLine, "Warning- file %s line %d is empty", (char*) x, count + 1);
                strcpy(lineStr[lineIndex], fullLine);
                lineIndex+=1;
            }
            else
            {
                strLine[strcspn(strLine, "\n")] = 0;
                pushNode(strLine);
            }
            count++;
            pthread_mutex_unlock(&tlock3);
        }
    }
    else
    {
        printf("File %s doesnt exist", (char*)x);
    }

    //free p thread
    pthread_mutex_lock(&tlock4);
    logindex = logindex + 1;
    // TODO use mutex to make this a start of a critical section
    if (p != NULL && p->creator == me) {
        printf("logindex %d, thread %ld, pid %d, %s. This is thread %ld and I delete THREADDATA\n",logindex, me, getpid(), time, me);
        free(p);
        p = NULL;
    }
    else
    {
        printf("logindex %d, thread %ld, pid %d, %s. This is thread %ld and I can access the THREADDATA\n",logindex, me, getpid(), time, me);
    }
    // TODO critical section ends
    fclose(fp);
    pthread_mutex_unlock(&tlock4);
    pthread_exit(NULL);
    return NULL;
}//end thread_runner
