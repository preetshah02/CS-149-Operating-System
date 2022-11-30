/**
 * Description: reads file and counts same names in the multiple files and print them using mmap.
 * Author names: Preet LNU
 * Author emails: preet.lnu@sjsu.edu
 * Last modified date: 11/20/2022
 * Creation date: 11/29/2022
 **/

//References:
//https://www.youtube.com/watch?v=rPV6b8BUwxM&ab_channel=JacobSorber
//http://www.java2s.com/Code/C/Structure/Usingastructuretorecordthecountofoccurrencesofaword.htm

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/mman.h>

#define ALL_NAMES 100                                               //Defining ALL_NAMES that has the maximum number of names.
#define MAX_CHAR 30                                                 //Defining MAX_CHAR that has the maximum number of characters a name can have.

int compareNotEqual(char string[MAX_CHAR], char str[MAX_CHAR]);     //initializing compareNotEqual method
void count_names(char *fileName);                                   //initialzing the count_names method

struct Name
{
    char names[MAX_CHAR];
    int counts;                                                     //counts the occurence of the name
    int isUnique;                                                   //checks if the name is already repeated or is unique.
};

//Declaring Global Variables
int maximum;                                                        //maximum number of unique names
int emptyLine = 0;                                                  //counts empty lines
char str[MAX_CHAR];                                                 //reads the name one by one
struct Name *data;

/**
 * This function compares two char and finds if they are unique or not.
 * If they are not equal it returns 1(true) and if equal return 0(false).
 * Assumption: both the char are limited to 30 characters.
 * Input parameters: char str1[MAX_CHAR], char str2[MAX_CHAR]
 * Returns: int
**/
int compareNotEqual(char str1[MAX_CHAR], char str2[MAX_CHAR])
{
    //This for loop checks if the enter parameters are equal or not.
    for(int i = 0; i < MAX_CHAR && str1[i] != '\0'; i++)
    {
        if (str1[i] != str2[i])
        {
            return 1;
        }
    }
    return 0;
} //end compareNotEqual

/**
 * This function counts the number of time a name is repeated in a provided .txt file as a parameter.
 * Assumption: File has 100 lines and each name is limited to 30 character
 * Input parameters: fileName
 * Returns: nothing(void function)
**/
void count_names(char *fileName)
{
    //opening the files
    FILE *fp = fopen(fileName, "r");

    //Checks if the file is empty.
    if (fp == NULL)
    {
        fprintf(stderr, "range: cannot open file\n"); //if cannot open file prints the error message
        exit(1);
    }

    //This while loop reads the file line by line
    while (fgets(str, MAX_CHAR, fp))
    {
        emptyLine = emptyLine + 1;
        //Checking if the line is empty or not
        if (!strcmp(str, "\n"))
        {
            //When there is a empty line printing warning message
            fprintf(stderr, "Warning - Line %d in file %s is empty\n", emptyLine, fileName);
            continue;
        }


        int index = 0;
        while ((compareNotEqual(data[index].names, str)) && (data[index].isUnique))
        {
            index++;
        }

        //If the name is unique we store it
        if (!data[index].isUnique)
        {
            int j = 0;
            for(j = 0; str[j] != '\n'; j++)
            {
                data[index].names[j] = str[j];
            }
            data[index].names[j] = '\0';
            data[index].counts = 1;
            data[index].isUnique = 1;
            maximum++;                   //incermenting the maximum
        }
        else //When the name is not unique
        {
            data[index].counts++;     //incrementing counts
        }
    }
    //closing the file
    fclose(fp);
}   //end count_names

/**
 * This function counts the number of time a name is repeated in multiple provided .txt file as a parameter.
 * Assumption: File has 100 lines and each name is limited to 30 character
 * Input parameters: argc, **argc
 * Returns: int type 0 to exit the code.
**/
int main(int argc, char **argv)
{
    //Reference: provided example mmap_example.c
    data = mmap(NULL, sizeof(struct Name)*ALL_NAMES, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    maximum = 0;                //initializing maximum to 0


    //This for loop runs till the number of argument entered
    for (int i = 1; i < argc; i++)
    {
        int childid = fork();   //creating a process
        if(childid == 0)        //entering the child process
        {
            count_names(argv[i]);
            exit(0);            //exiting is important else child will continue to fork its own children

        }
    }

    //Waiting for all the child processes to finish.
    wait(NULL);
    //This for loop is to print all the unique names and the number of time they occur
    for (int k = 0; data[k].isUnique && k < ALL_NAMES; k++)
    {
        printf("%s: %d\n", data[k].names, data[k].counts);
    }

    munmap(data, sizeof(struct Name)*ALL_NAMES);    //stopping the mapping
    return 0;
} //end main
