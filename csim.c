
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include<stdbool.h>

#include "cachelab.h"

// #define DEBUG_ON 
#define ADDRESS_LENGTH 64

/****************************************************************************/
/***** DO NOT MODIFY THESE VARIABLE NAMES ***********************************/

/* Globals set by command line args */
int verbosity = 0; /* print trace if set */
int s = 0; /* set index bits */
int b = 0; /* block offset bits */
int E = 0; /* associativity */
char* trace_file = NULL;

/* Derived from command line args */
int S; /* number of sets S = 2^s In C, you can use "pow" function*/
int B; /* block size (bytes) B = 2^b In C, you can use "pow" function*/

/* Counters used to record cache statistics */
int miss_count = 0;
int hit_count = 0;
int eviction_count = 0;

int count=0;
/*****************************************************************************/


/* Type: Memory address 
 * Use this type whenever dealing with addresses or address masks
  */
typedef unsigned long long int mem_addr_t;

/* Type: Cache line
 * TODO 
 * 
 * NOTE: 
 * You might (not necessarily though) want to add an extra field to this struct
 * depending on your implementation
 * 
 * For example, to use a linked list based LRU,
 * you might want to have a field "struct cache_line * next" in the struct 
 */
typedef struct cache_line {
    char valid;
    mem_addr_t tag;
    unsigned long long int last;
} cache_line_t;
//cache_line_t *next;


typedef cache_line_t* cache_set_t;
typedef cache_set_t* cache_t;


/* The cache we are simulating */
cache_t cache;  

/* TODO - COMPLETE THIS FUNCTION
 * initCache - 
 * Allocate data structures to hold info regrading the sets and cache lines
 * use struct "cache_line_t" here
 * Initialize valid and tag field with 0s.
 * calculate S = 2^s
 * use S and E while allocating the data structures here
 */
void initCache()
{
    S= pow(2,s); //2^s 
    B= pow(2,b);//""^b block size also needs to be adjusted 

    cache= malloc(S * sizeof(cache_set_t)); //use malloc to allocate number of cache set types in num of S

    if(cache==NULL) //if the return is NULL then exit bc nothig is there
    {
        return;//exit
    }

    for(int i=0; i<S; i++) //its not null so run through the sets returned
    {
        cache[i]= malloc(E * sizeof(cache_line_t));


        if(cache[i]==NULL)//return check again 
        {
            return;//exit if null
        }

        for(int j=0; j<E; j++)// each line we need to initialize 
        {
            cache[i][j].tag=0; //tag is 0
            cache[i][j].valid='0'; //valid is 0 
        }
    }

    
}


/* TODO - COMPLETE THIS FUNCTION 
 * freeCache - free each piece of memory you allocated using malloc 
 * inside initCache() function
 */
void freeCache()
{
    if(cache==NULL) //if its empty, then return
        return;


    for(int i=0; i < S; i++) //run through the Sets
    {
        free(cache[i]); //free the set arrays ie each part of the mem 
         cache[i]=NULL;//set it to null bc its free now 
    }

    free(cache); // free the cache array 
    cache=NULL;
}


/* TODO - COMPLETE THIS FUNCTION 
 * accessData - Access data at memory address addr.
 *   If it is already in cache, increase hit_count
 *   If it is not in cache, bring it in cache, increase miss count.
 *   Also increase eviction_count if a line is evicted.
 *   
 *   You will manipulate data structures allocated in initCache() here
 *   Implement Least-Recently-Used (LRU) cache replacement policy
 */
void accessData(mem_addr_t addr)
{
 mem_addr_t cachetag=addr >> (s+b); //getting the bits from the addy
 //shift right by excess bits ie s+b to get the tag
 mem_addr_t cacheset = (addr-(cachetag<<(s+b)))>>b;
 //mem_addr_t cacheset= (addr >> b)(& (pow(2,s)-1));// lab handout explained 
 //mask! all 1's # in set ie 2^s (-1) 
 //shift to right by b to get index on da right then same thing as tag but to get index bits using mask

int curr=0;
int temp= INT_MAX;
//j a large number for evicition...scroll down..we need it to compare for eviction 

for(int i=0; i<E; i++) //loop through blocks
{
    if(cache[cacheset][i].valid==1 && cache[cacheset][i].tag==cachetag) //if the one is there we want to access STOP
    {
        hit_count++; //increase counters 
        cache[cacheset][i].last=count++;
        return; //exit(1); //return
    }
}

miss_count++; //if its not there then we increment miss counter bc we missed obv

for(int i=0; i<E; i++) // cache line we evicting from
{
    if(cache[cacheset][i].last< temp) //greatest value 
    {
        curr=i; //this line needs to be evicted 
        temp=cache[cacheset][i].last; //temporary eviction variable 
    }
}

if(cache[cacheset][curr].valid==1) //if its valid then we evict if its not do nothing
{
    eviction_count++;
}


//set new equals to the new blcok 
cache[cacheset][curr].valid=1;
cache[cacheset][curr].tag=cachetag;
cache[cacheset][curr].last=count++;


//done i think? 

}


/* TODO - FILL IN THE MISSING CODE
 * replayTrace - replays the given trace file against the cache 
 * reads the input trace file line by line
 * extracts the type of each memory access : L/S/M
 * YOU MUST TRANSLATE one "L" as a load i.e. 1 memory access
 * YOU MUST TRANSLATE one "S" as a store i.e. 1 memory access
 * YOU MUST TRANSLATE one "M" as a load followed by a store i.e. 2 memory accesses 
 * Ignore instruction fetch "I"
 */
void replayTrace(char* trace_fn)
{
    char buf[1000];
    mem_addr_t addr=0;
    unsigned int len=0;
    FILE* trace_fp = fopen(trace_fn, "r");

    if(!trace_fp){
        fprintf(stderr, "%s: %s\n", trace_fn, strerror(errno));
        exit(1);
    }

    while( fgets(buf, 1000, trace_fp) != NULL) {
        if(buf[1]=='S' || buf[1]=='L' || buf[1]=='M') {
            sscanf(buf+3, "%llx,%u", &addr, &len);
      
            if(verbosity)
                printf("%c %llx,%u ", buf[1], addr, len);

           // TODO - MISSING CODE
           // now you have: 
           // 1. address accessed in variable - addr 
           // 2. type of acccess(S/L/M)  in variable - buf[1] 
           // call accessData function here depending on type of access
            /*
            if(buf[1]=='L')
            {
                accessData(addr);
            }
            else if(buf[1]=='S')
            {
                accessData(addr);
            }
            else
            {
                accesssData(addr);
                accessData(addr);
            }
            */
            accessData(addr); // access data obv bc they all use one mem access 

            if(buf[1]=='M')//modify 
            {
               accessData(addr); //gotta access data mem 2 times
            }
            



            if (verbosity)
                printf("\n");
        }
    }

    fclose(trace_fp);
}

/*
 * printUsage - Print usage info
 */
void printUsage(char* argv[])
{
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\nExamples:\n");
    printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
    printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
    exit(0);
}

/*
 * main - Main routine 
 */
int main(int argc, char* argv[])
{
    char c;
    
    // Parse the command line arguments: -h, -v, -s, -E, -b, -t 
    while( (c=getopt(argc,argv,"s:E:b:t:vh")) != -1){
        switch(c){
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            trace_file = optarg;
            break;
        case 'v':
            verbosity = 1;
            break;
        case 'h':
            printUsage(argv);
            exit(0);
        default:
            printUsage(argv);
            exit(1);
        }
    }

    /* Make sure that all required command line args were specified */
    if (s == 0 || E == 0 || b == 0 || trace_file == NULL) {
        printf("%s: Missing required command line argument\n", argv[0]);
        printUsage(argv);
        exit(1);
    }


    /* Initialize cache */
    initCache();

#ifdef DEBUG_ON
    printf("DEBUG: S:%u E:%u B:%u trace:%s\n", S, E, B, trace_file);
#endif
 
    replayTrace(trace_file);

    /* Free allocated memory */
    freeCache();

    /* Output the hit and miss statistics for the autograder */
    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}
