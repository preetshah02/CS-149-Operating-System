/**
 * Description: This program will store the command lines in an array and trace the memory usage.
 * Author names: Preet LNU, Nahal Bagheri
 * Author emails: preet.lnu@sjsu.edu, nahal.bagheri@sjsu.edu
 * Last modified date: 10/28/2022
 * Creation date: 10/20/2022
 **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>

/**
 *CS149 assignment#4 helper code.
 *See the TODO's in the comments below! You need to implement those.
**/

/**
 *TRACE_NODE_STRUCT is a linked list of
 * pointers to function identifiers
 * TRACE_TOP is the head of the list is the top of the stack
**/
struct TRACE_NODE_STRUCT {
    char* functionid;// ptr to function identifier (a function name)
    int index;
    struct TRACE_NODE_STRUCT* next;  // ptr to next frama
};
typedef struct TRACE_NODE_STRUCT TRACE_NODE;
static TRACE_NODE* TRACE_TOP = NULL;       // ptr to the top of the stack


/* --------------------------------*/
/* function PUSH_TRACE */
/*
 * The purpose of this stack is to trace the sequence of function calls,
 * just like the stack in your computer would do.
 * The "global" string denotes the start of the function call trace.
 * The char *p parameter is the name of the new function that is added to the call trace.
 * See the examples of calling PUSH_TRACE and POP_TRACE below
 * in the main, make_extend_array, add_column functions.
**/
void PUSH_TRACE(char* p)          // push p on the stack
{
    TRACE_NODE* tnode;
    static char glob[]="global";

    if (TRACE_TOP==NULL) {

        // initialize the stack with "global" identifier
        TRACE_TOP=(TRACE_NODE*) malloc(sizeof(TRACE_NODE));

        // no recovery needed if allocation failed, this is only
        // used in debugging, not in production
        if (TRACE_TOP==NULL) {
            printf("PUSH_TRACE: memory allocation error\n");
            exit(1);
        }

        TRACE_TOP->functionid = glob;
        TRACE_TOP->next=NULL;
    }//if

    // create the node for p
    tnode = (TRACE_NODE*) malloc(sizeof(TRACE_NODE));

    // no recovery needed if allocation failed, this is only
    // used in debugging, not in production
    if (tnode==NULL) {
        printf("PUSH_TRACE: memory allocation error\n");
        exit(1);
    }//if

    tnode->functionid=p;
    tnode->next = TRACE_TOP;  // insert fnode as the first in the list
    TRACE_TOP=tnode;          // point TRACE_TOP to the first node

}/*end PUSH_TRACE*/

/* --------------------------------*/
/* function POP_TRACE */
/* Pop a function call from the stack */
void POP_TRACE()    // remove the op of the stack
{
    TRACE_NODE* tnode;
    tnode = TRACE_TOP;
    TRACE_TOP = tnode->next;
    free(tnode);

}/*end POP_TRACE*/



/* ---------------------------------------------- */
/* function PRINT_TRACE prints out the sequence of function calls that are on the stack at this instance */
/* For example, it returns a string that looks like: global:funcA:funcB:funcC. */
/* Printing the function call sequence the other way around is also ok: funcC:funcB:funcA:global */
char* PRINT_TRACE()
{
    int depth = 50; //A max of 50 levels in the stack will be combined in a string for printing out.
    int i, length, j;
    TRACE_NODE* tnode;
    static char buf[100];

    if (TRACE_TOP==NULL) {     // stack not initialized yet, so we are
        strcpy(buf,"global");   // still in the `global' area
        return buf;
    }

    /* peek at the depth(50) top entries on the stack, but do not
       go over 100 chars and do not go over the bottom of the
       stack */

    sprintf(buf,"%s",TRACE_TOP->functionid);
    length = strlen(buf);                  // length of the string so far
    for(i=1, tnode=TRACE_TOP->next;
        tnode!=NULL && i < depth;
        i++,tnode=tnode->next) {
        j = strlen(tnode->functionid);             // length of what we want to add
        if (length+j+1 < 100) {              // total length is ok
            sprintf(buf+length,":%s",tnode->functionid);
            length += j+1;
        }else                                // it would be too long
            break;
    }
    return buf;
} /*end PRINT_TRACE*/

// -----------------------------------------
// function REALLOC calls realloc
// TODO REALLOC should also print info about memory usage.
// TODO For this purpose, you need to add a few lines to this function.
// For instance, example of print out:
// "File tracemem.c, line X, function F reallocated the memory segment at address A to a new size S"
// Information about the function F should be printed by printing the stack (use PRINT_TRACE)
void* REALLOC(void* p,int t,char* file,int line)
{
    p = realloc(p,t);
    printf("File %s, line %d, function %s reallocated the memory segment at address %p to a new size %d\n", file, line, PRINT_TRACE(), p, t);
    return p;
}

// -------------------------------------------
// function MALLOC calls malloc
// TODO MALLOC should also print info about memory usage.
// TODO For this purpose, you need to add a few lines to this function.
// For instance, example of print out:
// "File tracemem.c, line X, function F allocated new memory segment at address A to size S"
// Information about the function F should be printed by printing the stack (use PRINT_TRACE)
void* MALLOC(int t,char* file,int line)
{
    void* p;
    p = malloc(t);
    printf("File %s, line %d, function %s allocated the memory segment at address %p to a new size %d\n", file, line, PRINT_TRACE(), p, t);
    return p;
}

// ----------------------------------------------
// function FREE calls free
// TODO FREE should also print info about memory usage.
// TODO For this purpose, you need to add a few lines to this function.
// For instance, example of print out:
// "File tracemem.c, line X, function F deallocated the memory segment at address A"
// Information about the function F should be printed by printing the stack (use PRINT_TRACE)
void FREE(void* p,char* file,int line) {
    printf("File %s, line %d, function %s deallocated the memory segment at address %p\n", file, line, PRINT_TRACE(), p);
    free(p);
}

#define realloc(a,b) REALLOC(a,b,__FILE__,__LINE__)
#define malloc(a) MALLOC(a,__FILE__,__LINE__)
#define free(a) FREE(a,__FILE__,__LINE__)


// -----------------------------------------
// function add_column will add an extra column to a 2d array of ints.
// This function is intended to demonstrate how memory usage tracing of realloc is done
// Returns the number of new columns (updated)
int add_column(int** array,int rows,int columns)
{
    PUSH_TRACE("add_column");
    int i;

    for(i=0; i<rows; i++) {
        array[i]=(int*) realloc(array[i],sizeof(int)*(columns+1));
        array[i][columns]=10*i+columns;
    }
    POP_TRACE();
    return (columns+1);
}// end add_column


// // ------------------------------------------
// // function make_extend_array
// // Example of how the memory trace is done
// // This function is intended to demonstrate how memory usage tracing of malloc and free is done
// void make_extend_array()
// {
//        PUSH_TRACE("make_extend_array");
// 	int i, j;
//         int **array;
//         int ROW = 4;
//         int COL = 3;

//         //make array
// 	array = (int**) malloc(sizeof(int*)*4);  // 4 rows
// 	for(i=0; i<ROW; i++) {
// 	 array[i]=(int*) malloc(sizeof(int)*3);  // 3 columns
// 	 for(j=0; j<COL; j++)
// 	  array[i][j]=10*i+j;
// 	}

//         //display array
// 	for(i=0; i<ROW; i++)
// 	 for(j=0; j<COL; j++)
// 	  printf("array[%d][%d]=%d\n",i,j,array[i][j]);

// 	// and a new column
// 	int NEWCOL = add_column(array,ROW,COL);

// 	// now display the array again
//         for(i=0; i<ROW; i++)
// 	 for(j=0; j<NEWCOL; j++)
// 	  printf("array[%d][%d]=%d\n",i,j,array[i][j]);

// 	 //now deallocate it
// 	 for(i=0; i<ROW; i++)
// 		 free((void*)array[i]);
// 	 free((void*)array);

// 	 POP_TRACE();
//          return;
// }//end make_extend_array

#define ROW 3
#define COLUMN 10

/**
 * This function creates the array memory by using malloc.
 * Input parameters: char ***arr, int row, int col
**/
void createArray(char ***arr, int row, int col)
{

    PUSH_TRACE("createArray");      //push trace
    char **array = NULL;            //Initializing an array
    array = (char **) malloc(sizeof(char *) * row);     //Initializing row

    //This for loop is used to initialize col
    for (int i = 0; i < row; i++)
    {
        array[i] = (char *) malloc(sizeof(char) * col);
    }
    *arr = array;       //assigning the array to the pointer
    POP_TRACE();
} //creatArray ends

/**
 * This function is used to print the array.
 * Input parameters: char **arr, int row
**/
void printArray(char **arr, int row)
{
    PUSH_TRACE("printArray");   //push trace

    //This for loop is used to print out the array for each row
    for (int i = 0; i < row; i++)
    {
        printf("array[%d] = %s\n", i, arr[i]);
    }
    POP_TRACE();
} //printArray ends

/**
 * This function is used to add nodes to the LinkedList.
 * Input parameters: TRACE_NODE **head, TRACE_NODE *node
**/
void addNode(TRACE_NODE **head, TRACE_NODE *node)
{
    PUSH_TRACE("addNode");

    // If the head is null new node becomes the head else it will go through the LinkedList and insert the node at the end
    if (*head == NULL)
    {
        *head = node;
    }
    else
    {
        TRACE_NODE *current = *head;
        while (current->next != NULL)
            current = current->next;
        current->next = node;
    }
    POP_TRACE();
} //addNode ends

/**
 * This function is used to store each line from the stdin by creating a new node.
 * Input parameters: TRACE_NODE **head, char *line, int index
**/
void createNode(TRACE_NODE **head, char *line, int index)
{
    PUSH_TRACE("createNode");
    TRACE_NODE *node = (TRACE_NODE *) malloc(sizeof(TRACE_NODE));
    node->functionid = (char *) malloc(strlen(line) + 1);
    strncpy(node->functionid, line, strlen(line) + 1);
    node->index = index;
    node->next = NULL;
    POP_TRACE();
    addNode(head, node);
}

/**
 * This function is used to recursively print all the nodes in the LinkedList.
 * Input parameters: TRACE_NODE *node
**/
void printNode(TRACE_NODE *node)
{
    PUSH_TRACE("printNode");
    if (node != NULL)
    {
        printf("LinkedList[%d] = %s\n", node->index, node->functionid);     //printing the nodes in the list
        printNode(node->next);  //recurrence
    }
    POP_TRACE();
} // printNode ends

/**
 * This function is used to deallocate nodes in the LinkedList.
 * Input parameters: TRACE_NODE **head, char *line, int index
**/
void deallocateList(TRACE_NODE **head)
{
    PUSH_TRACE("deallocateList");
    TRACE_NODE *current = *head;        //node we want to deallocate.
    while (current != NULL)
    {
        TRACE_NODE *temp = current->next;       //keeping track of the node
        free(current->functionid);
        free(current);                          //deallocating the node
        current = temp;                         //putting the next node in the list in place of the deleted node.
    }
    POP_TRACE();
} // deallocateList ends

/**
 * This is the main function of the program.
 * Return: int
**/
int main()
{
    PUSH_TRACE("main");

    // create .out file using dup2
    int fdout = open("memtrace.out", O_WRONLY | O_CREAT | O_TRUNC, 0777);
    dup2(fdout, STDOUT_FILENO);

    char **arr = NULL;      //initializing array
    createArray(&arr, ROW, COLUMN);     //creating array memory using malloc

    //Variables to read from stdin
    ssize_t read;
    size_t length;
    char *line = NULL;

    // filling the array with lines from stdin
    int row = 0;
    while ((read = getline(&line, &length, stdin)) >= 0)
    {
        row++;

        if (line[strlen(line) - 1] == '\n')
        {
            line[strlen(line) - 1] = '\0';
        }

        int col = strlen(line) + 1;

        //expanding array if the initial size turns out not to be big enough for the lines in the input.
        if (ROW < row)
        {
            arr = (char **) realloc(arr, sizeof(char *) * row);
            arr[row - 1] = (char *) malloc(sizeof(char) * col);
        }
        if (COLUMN < col)
        {
            arr[row - 1] = (char *) realloc(arr[row - 1], sizeof(char) * col);
        }

        strncpy(arr[row - 1], line, col);       //copying current line to the array
    }

    printArray(arr, row);   //printing array

    //Creating LinkedList
    TRACE_NODE *head = NULL;

    //This for loop creates nodes and adds it to the LinkedList
    for (int i = 0; i < row; i++)
    {
        createNode(&head, arr[i], i);
    }

    printNode(head);    //printing LinkedList
    deallocateList(&head);  //deallocating LinkedList

    int larger = 0;
    if(ROW < row)
        larger = row;
    else if(ROW > row)
        larger = ROW;

    //Freeing the array
    for (int i = 0; i < larger; i++)
    {
        free(arr[i]);
    }
    free(arr);

    //Freeing the lines
    if (line)
    {
        free(line);
    }

    close(fdout);

    POP_TRACE();
    return(0);
} // main end