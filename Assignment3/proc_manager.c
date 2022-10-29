/**
 * Description: processes get executed in parallel, keep logs, track exit codes and signals, and duplicate file descriptors.
 * Author names: Preet LNU, Nahal Bagheri
 * Author emails: preet.lnu@sjsu.edu, nahal.bagheri@sjsu.edu
 * Last modified date: 10/07/2022
 * Creation date: 09/30/2022
 **/

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

/**
 * This function reads the commands in stdin, exectues them in parallel and stores the result in two files .out and .err.
 * Assumption: File has 100 command lines and each command is limited to 30 character
 * Input parameters: N/A
 * Returns: int type 0 to exit the code.
**/
int main()
{
    int count = 0;  // line number

    char *str = NULL; // store each line from the command line file
    size_t length = 0; // length of the str
    ssize_t read;  // number of characters in one line

    char newString[100][30];
    int i, j, ctr;

    // read by line in the terminal
    while ((read = getline(&str, &length, stdin)) != -1)
    {
        count++;

        if (str[read - 1] == '\n'){
            str[read - 1] = '\0';
        }

        for (int i = 0; i < 100; i++)
            for (int j = 0; j < 30; j++)
                newString[i][j]=0;

        j = 0; ctr = 0;
        for(i = 0; i <= (strlen(str)); i++)
        {
            // if space or NULL found, assign NULL into newString[ctr]
            if(str[i] == ' ' || str[i] == '\0')
            {
                newString[ctr][j]='\0';
                ctr++;  //for next word
                j = 0;    //for next word, init index to 0
            }
            else
            {
                newString[ctr][j] = str[i];
                j++;
            }
        }

        pid_t pids;
        if ((pids = fork()) < 0)
        {
            perror("Fork Failed!");
            exit(1);
        }
            // child process
        else if (pids == 0)
        {
            // create PID.out and PID.err files to store messages
            char out[20],err[20];
            sprintf(out, "%d.out",getpid());
            sprintf(err, "%d.err",getpid());
            int outFile = open(out, O_RDWR | O_CREAT | O_APPEND, 0777);
            int errFile = open(err, O_RDWR | O_CREAT | O_APPEND, 0777);
            dup2(outFile, 1); // put stdout into outFile
            dup2(errFile, 2); // put stderr into errFile

            fprintf(stdout, "Starting command %d: child %d pid of parent %d\n", count, getpid(), getppid());    // print out starting message into outFile
            fflush(stdout);

            //initialize command line buff
            char *buf[ctr + 1];
            for (i = 0; i < ctr; i++)
            {
                buf[i] = newString[i];
            }
            buf[ctr] =  (char *)0;  // the last element should be "\0"

            // execute
            execvp(buf[0], buf);
            printf("couldn't execute: %s", buf[0]);
            exit(2);
        }
    }

    if (str)
    {
        free(str);
    }

    // do parent work
    int status;
    pid_t childPid;
    char message[500]; //output and error message

    while ((childPid = wait(&status)) >= 0){ // parent process
        // open pid.out/pid.err file
        char out[20],err[20];
        sprintf(out, "%d.out", childPid);
        sprintf(err, "%d.err", childPid);
        int outFile = open(out, O_RDWR | O_APPEND);
        int errFile = open(err, O_RDWR | O_APPEND);

        sprintf(message, "Finished child %d pid of parent %d\n", childPid, getpid());
        write(outFile, message, strlen(message));

        // Terminating normally with an exit code
        if (WIFEXITED(status))
        {
            sprintf(message, "Exited with exitcode = %d\n", WEXITSTATUS(status));
            write(errFile,message, strlen(message));
        }

        //Terminating abnormally with a signal
        else if (WIFSIGNALED(status))
        {
            sprintf(message, "Killed with signal %d\n", WTERMSIG(status));
            write(errFile,message, strlen(message));
        }
    }
    return 0;
}
