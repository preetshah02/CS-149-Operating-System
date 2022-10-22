/**
 * Description: reads file and counts same names in the multiple files and print them
 * Author names: Preet LNU, Nahal Bagheri
 * Author emails: preet.lnu@sjsu.edu, nahal.bagheri@sjsu.edu
 * Last modified date: 09/28/2022
 * Creation date: 09/20/2022
 **/
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include<ctype.h>
#include<string.h>

void count_names(char *fileName);       //initialzing the count_names method(Assignmet1 Code)
#define ALL_NAMES 100               //Defining ALL_NAMES that has the maximum number of names.

//Declaring Global Variables
int maximum = 0;                    //maximum number of unique names
int emptyLine = 0;
int counts[ALL_NAMES] = {0};     //has the count of names and setting it to 0.
char names[ALL_NAMES][30];      //has the names
char str[30];                   //reads the name one by one
int isUnique;   //checks if the name is already repeated or is unique.

/**
 * This function counts the number of time a name is repeated in multiple provided .txt file as a parameter.
 * Assumption: File has 100 lines and each name is limited to 30 character
 * Input parameters: argc, **argc
 * Returns: int type 0 to exit the code.
**/
int main(int argc, char **argv)
{
    int fd[2];
    pipe(fd);       //initializing the part

    //Prints the error if the pipe fails
    if(pipe(fd) < 0)
        printf("Pipe Failed!");

    //This for loop runs till the number of argument entered
    for (int i = 1; i < argc; i++)
    {
        int childid = fork();   //creating a process
        if(childid == 0)        //entering the child process
        {
            count_names(argv[i]);       //assignment1 Code which take multiple files.

            //closing child
            close(fd[0]);

            //writing the child into the pipe
            for(int k = 0; k < maximum; k++) {
                write(fd[1], names[k], 30);                 //writing names into the pipe
                write(fd[1], &counts[k], sizeof(int));      //writing counts into the pipe
            }
            exit(0);        //exiting is important else child will continue to fork its own children
        }
    }


    //the parent waits for each child to finish
    while(wait(NULL) > 0)               //parent gets child result via a pipe
    {
        //closing the writing end of the pipe
        close(fd[1]);
        //This while loop reads until there's nothing left in the pipe.
        while(read(fd[0], str, 30) > 0)
        {
            int counter = 0;
            read(fd[0], &counter, sizeof(int));
            isUnique = 0;

            int i;

            //This for loop compares the name and checks if the names are unique or not
            for(i = 0; i < 100; i++)
            {
                if(strcasecmp(names[i], str) == 0)  //comparing the names to determine if its same
                {
                    isUnique = 1;
                    break;
                }
            }

            //if the line is not unique add the name to str
            if(!isUnique)
            {
                strcpy(names[maximum], str); //adding name to the str
                counts[maximum] = counter; //incrementing the counter
                maximum++;                   //incermenting the maximum
            }
            else
            {
                counts[i] = counts[i] + counter;
            }
        }
    }

    //Printing out the names and the number of times a names occurs.
    for(int p = 0; p < maximum; p++)
        printf("%s: %d\n", names[p], counts[p]);
    return 0;
}

/**
 * This function counts the number of time a name is repeated in a provided .txt file as a parameter.
 * Assumption: File has 100 lines and each name is limited to 30 character
 * Input parameters: fileName
 * Returns: nothing(void function)
**/
void count_names(char *fileName)
{
    int i;
    //opening the files
    FILE *fp = fopen(fileName, "r");

    //Checks if the file is empty.
    if (fp == NULL) {
        fprintf(stderr, "range: cannot open file\n"); //if cannot open file prints the error message
        exit(1);
    }
    maximum = 0;

    while (fgets(str, 30, fp)) {
        emptyLine = emptyLine + 1;
        //Checking if the line is empty or not
        if (!strcmp(str, "\n")) {
            //When there is a empty line printing warning message
            fprintf(stderr, "Warning - Line %d in file %s is empty\n", emptyLine, fileName);
        }
        else
        {
            str[strcspn(str, "\n")] = 0;    //scanning str to find blank lines
            isUnique = 0;

            //This for loop checks if the line is unique
            for (i = 0; i < 100; i++) {
                if (strcasecmp(names[i], str) == 0)  //comparing the names to determine if its same
                {
                    isUnique = 1;
                    break;
                }
            }
            //if the line is not unique add the name to str
            if (!isUnique) {
                strcpy(names[maximum], str); //adding name to the str
                counts[maximum]++;            //incrementing the count
                maximum++;                   //incermenting the maximum
            }
            else
            {
                counts[i]++;     //incrementing count
            }
        }
    }
    //closing the file
    fclose(fp);
}