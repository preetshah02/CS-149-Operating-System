/**
 * Description: reads file and counts same names in the file and print them
 * Author names: Preet LNU, Nahal Bagheri
 * Author emails: preet.lnu@sjsu.edu, nahal.bagheri@sjsu.edu
 * Last modified date: 09/12/2022
 * Creation date: 09/07/2022
 **/
#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>
/**
 * This function counts the number of time a name is repeated in a provided .txt file as a parameter.
 * Assumption: File has 100 lines and each name is limited to 30 character
 * Input parameters: argc, *argv[1]
 * Returns: int type 0 to exit the code.
**/

#define ALL_NAMES 100   //Defining ALL_NAMES that has the maximum number of names.

int main(int argc, char *argv[])
{
    int i;
    int maximum;                    //maximum number of unique names
    int emptyLine = 0;
    int count[ALL_NAMES] = {0};     //has the count of names and setting it to 0.
    char names[ALL_NAMES][30];      //has the names
    char str[30];                   //reads the name one by one
    int isUnique;   //checks if the name is already repeated or is unique.


    //Checking if the file name has provided in the parameter
    if(argc != 2)
    {
        printf("File not provided\n");
    }
    else
    {
        //opening the file
        FILE *fp = fopen(argv[1], "r");


        //Checks if the file is empty.
        if(fp == NULL)
        {
            fprintf(stderr, "error: cannot open file\n"); //if cannot open file prints the error message
            exit(1);
        }
        maximum = 0;


        //Reading the file line by line
        while(fgets(str, 30, fp))
        {
            emptyLine = emptyLine + 1;
            //Checking if the line is empty or not
            if(!strcmp(str, "\n"))
            {
                //When there is a empty line printing warning message
                fprintf(stderr, "Warning - Line %d is empty\n", emptyLine);
            }
            else
            {
                str[strcspn(str, "\n")] = 0;    //scanning str to find blank lines
                isUnique = 0;

                //This for loop checks if the line is unique
                for(i = 0; i < maximum ; i++)
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
                    count[maximum]++;            //incrementing the count
                    maximum++;                   //incermenting the maximum
                }
                else
                {
                    count[i]++;     //incrementing count
                }
            }
        }
        //closing the file
        fclose(fp);

        //Printing the names and the number of times it occured in the file.
        for(i = 0; i < maximum; i++)
            printf("%s: %d\n", names[i], count[i]);
    }
    return 0;
}

