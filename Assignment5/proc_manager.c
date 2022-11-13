/**
 * Description: This program Tracks data on processes, restart processes, time processes
 * Author names: Preet LNU, Nahal Bagheri
 * Author emails: preet.lnu@sjsu.edu, nahal.bagheri@sjsu.edu
 * Last modified date: 11/11/2022
 * Creation date: 11/03/2022
 **/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#define HASHSIZE 101
#define FILE_NAME_NUM 25                        // constant for file name size
static struct nlist *hashtab[HASHSIZE];         // pointer table
int hashTableCount = 0;

struct nlist {
    struct nlist *next;                         // next index in hashtable
    struct timespec sta, fin;                   // holds start and end times of process
    int index;                                  // index of pid in hashtable
    int pid;                                    // pid of command
    char *command;
};

unsigned hash(int pid)
{
    return pid % HASHSIZE;
}
/* lookup: look for s in hashtab */
/* TODO change to lookup by pid: struct nlist *lookup(int pid) */
/* TODO modify to search by pid, you won't need strcmp anymore */
/* This is traversing the linked list under a slot of the hash
table. The array position to look in is returned by the hash
function */
struct nlist *lookup(int pid)
{
    struct nlist *np;
    for (np = hashtab[hash(pid)]; np != NULL; np = np->next)
        if (np->pid == pid)
            return np;                          // found
    return NULL;
}
/* insert: put (name, defn) in hashtab */
/* TODO: change this to insert in hash table the info for a new
pid and its command */
/* TODO: change signature to: struct nlist *insert(char *command,
int pid, int index). */
/* This insert returns a nlist node. Thus when you call insert in
your main function  */
/* you will save the returned nlist node in a variable (mynode).
*/
/* Then you can set the starttime and finishtime from your main
function: */
/* mynode->starttime = starttime; mynode->finishtime = finishtime;
*/
struct nlist *insert(char *command, int pid, int index)
{
    struct nlist *np;
    unsigned hashval;
    //TODO change to lookup by pid. There are 2 cases:
    if ((np = lookup(pid)) == NULL)
    {
        hashTableCount++;

        /* case 1: the pid is not
        found, so you have to create it with malloc.
        Then you want to set the pid, command and index */
        np = (struct nlist *) malloc(sizeof(*np));
        if (np == NULL || ((np->command = strdup(command)) == NULL))
        {
            return NULL;
        }
        np->pid = pid;
        np->index = index;

        hashval = hash(pid);
        np->next = hashtab[hashval];
        hashtab[hashval] = np;
    }
    else
    {
        /* case 2: the pid is already there in the hashslot,
        i.e. lookup found the pid. In this case you can either
        do nothing, or you may want to set again the command
        and index (depends on your implementation). */
        free((void *) np->command);                             // free previous command
        if ((np->command = strdup(command)) == NULL)
        {
            return NULL;
        }
        np->index = index;
    }
    return np;
}

/**
 * This function helps to execute execvp in the main function
 * Input parameters: char line[], int cmdNum, int restart
**/
void doExecvp(char line[], int cmdNum, int restart)
{
    int pid = getpid();
    char pidFile[FILE_NAME_NUM];
    sprintf(pidFile, "%d.out", pid);

    int fwrite = open(pidFile, O_RDWR | O_CREAT | O_TRUNC, 0777);
    dup2(fwrite, fileno(stdout));                               // create copy of the file descriptor

    // if the process is restarting prints the message RESTARTING
    if(restart)
    {
        fprintf(stdout, "RESTARTING\n");
    }

    fprintf(stdout, "Starting command %d: child %d pid of parent %d\n", cmdNum, getpid(), getppid());

    fflush(stdout);                                             // move buffered data to console

    // variables to parse string to pass into execvp arguments
    char *input[strlen(line)];
    char *token = strtok(line, " \n");
    int index = 0;

    //This while loop is used for parsing
    while (token != NULL)
    {
        input[index] = token;
        index++;
        token = strtok(NULL, " \n");
    }

    input[index] = NULL;

    fflush(stdout);
    fflush(stderr);

    close(fwrite);

    // call execvp
    if (execvp(input[0], input) == -1)
    {
        char pidErrFile[FILE_NAME_NUM];

        sprintf(pidErrFile, "%d.err", pid);

        int ferr = open(pidErrFile, O_RDWR | O_CREAT | O_APPEND, 0777);

        // if execvp succeeds it doesn't run below but if Execvp fails and it runs the following:
        dup2(ferr, STDOUT_FILENO);
        dup2(ferr, STDERR_FILENO);
        perror("Execvp failed: \n");
        fflush(stdout);
        fflush(stderr);
        close(ferr);
    }
}

/**
 * This is the main function of the program.
 * Return: int
**/
int main()
{
    int cmdNum = 0;                                 // int to record number of commands

    char * line = NULL;                             // points to line in cmdfile
    size_t size = 0;                                // size of line cmdfile
    ssize_t read;                                   // variable used to read from file

    struct timespec sta, fin;

    //new line is recorded from text file
    while ((read = getline(&line, &size, stdin)) != -1)
    {
        line[read - 1] = '\0';
        cmdNum++;

        clock_gettime(CLOCK_MONOTONIC, &sta);
        int pid = fork();


        if(pid == 0)
        {
            doExecvp(line, cmdNum, 0);
            exit(0);

        }
        else if(pid > 0)                            // parent process
        {
            // store process data in hash
            struct nlist* myNode = insert(line, pid, cmdNum);
            myNode->sta = sta;
        }
        else
        {
            fprintf(stderr, "Fork failed");         // fork fails, exit
            return 1;
        }
    }

    pid_t p;                                        // pid of child process

    int status;                                     // status of child process
    struct nlist* node;                             // node in linked list of hash table
    double time;                                    // time elapsed for child process

    while((p = wait(&status)) >= 0)
    {
        fflush(stdout);
        fflush(stderr);
        node = lookup(p);
        clock_gettime(CLOCK_MONOTONIC, &fin);
        if(node != NULL)
        {
            node->fin = fin;
            time = ((double)node->fin.tv_sec + 1.0e-9 * node->fin.tv_nsec)
                   - ((double)node->sta.tv_sec + 1.0e-9 * node->sta.tv_nsec);
        }
        else {
            printf("node = NULL!\n");
        }

        char pidOutFile[20];
        sprintf(pidOutFile, "%d.out", p);
        FILE * outFP = fopen(pidOutFile, "a+b");
        if(outFP == NULL)
        {
            return 2;
        }
        fprintf(outFP, "Finished child %i pid of parent %i\n", p, getpid());
        fprintf(outFP, "Finished at %ld, runtime duration %f\n", node->fin.tv_nsec, time);

        char pidErrFile[FILE_NAME_NUM];             // name of file
        sprintf(pidErrFile, "%d.err", p);           // convert to string

        int ferr = open(pidErrFile, O_RDWR | O_CREAT | O_APPEND,0777);

        dup2(ferr, fileno(stderr));

        // termination with exit code
        if(WIFEXITED(status))
        {
            fprintf(stderr, "Exited with exitcode = %d\n", WEXITSTATUS(status));
            if(time <= 2)
            {
                fprintf(stderr, "Spawning too fast\n");
            }
        }

            // termination with exit signal
        else if(WIFSIGNALED(status))
        {
            fprintf(stderr, "Killed with signal = %d\n", WTERMSIG(status));
            if(time <= 2)
            {
                fprintf(stderr, "Spawning too fast\n");
            }
        }

        close(ferr);

        if (time > 2)                                       // restart process if time > 2
        {
            clock_gettime(CLOCK_MONOTONIC, &sta);           // save start time

            int pid = fork();

            if (pid < 0)
            {
                fprintf(stderr, "Fork failed\n");             // fork fails, exit
                return 1;
            }
            if (pid == 0)                                   // child
            {
                doExecvp(node->command, node->index, 1);    // execute execvp
            }
            else if (pid > 0)
            {
                struct nlist* myNode2 = insert(node->command, pid, node->index);
                myNode2->sta = sta;
            }
        }
    }

    return 0;
}
