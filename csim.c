#include <stdio.h>
#include <stdlib.h>
#include "cachelab.h"
#include <unistd.h> //to use getopt() to parse elements from command line
#include <getopt.h> //because we're not working on a shark machine
int time = 0 , hits = 0 , miss = 0, evict = 0;

typedef struct cache_line
{
    int tag, valid, counter;

} cache_line;

/*************** functions prototype ****************/

/*this function parses the flags which contains values of tagbits ,setbits , associatevity , block offsets and the file name to the program */
void parse_elements(int *opt, int *s, int *E, int *b, char **file_name, int *argc, char **argv);
/*calculates the number of sets and blocks from the number of bits */
void calculateSB(int *S, int *B, int s, int b);
/*extracts the tag and the set number from the instruction */
void decode(int address, int *tagbits, int *setbits, int b, int s,int S);
/*checks the hits ,misses and evicts */
void execute(char identifier,int tagbits,int setbits,cache_line* cache, int E);
/*check if the passed set contains the same tag*/
int check_hits(cache_line*current_set , int E,int tagbits);
/*checks if the passed tag bits in the needed cache line is the same*/
int check_tagbits(cache_line*current_line , int tagbits );
/*returns and address to the needed set*/
cache_line* get_current_set(cache_line*cache, int setbits, int E);
/*checks if there is an empty line in the needed set and if so it uses it for the needed data and returns 1 if there is not empty line on the set it returns zero*/
int check_available_lines_and_insert(cache_line* current_set,int E , int tagbits);
/*takes the victim address and updated the time stamp and tag bits */
void evict_and_insert(cache_line* victim,int tagbits);
/*this function takes an address of a full cache set and returns a pointer to 
a victim */
cache_line* find_victim(cache_line* current_set ,int E);
/*this function prints the whole contents of the cache*/
void debug(cache_line* cache,int S,int E);
/***************************************************/


int main(int argc, char **argv)
{
    int opt, s = 0, E = 0, b = 0, S, B;
    char identifier;
    int address, size;
    char *file_name = NULL;
    FILE *pFile;
    int tagbits, setbits;

    parse_elements(&opt, &s, &E, &b, &file_name, &argc, argv);
    calculateSB(&S, &B, s, b);
    /* creating the memory that the cache will be stored into
    and initializing it to zero*/
    cache_line *cache = calloc(S*E, sizeof(cache_line));
    debug(cache,S,E);
    // opening the file and handling the data on it
    pFile = fopen(file_name, "r");

    // the loop fetches the instructions from the file 
    while (fscanf(pFile, " %c %x,%d", &identifier, &address, &size) > 0)
    {
        if(identifier == 'I')
        {
            continue;
        }
        else{
        decode(address, &tagbits, &setbits, b, s,S);
        execute(identifier,tagbits,setbits,cache,E);
        }
    }
    
    
    printSummary(hits, miss, evict);
    debug(cache,S,E);
    free(cache);
    fclose(pFile);
    
    return 0;
}

void parse_elements(int *opt, int *s, int *E, int *b, char **file_name, int *argc, char **argv)
{
    while (-1 != (*opt = getopt(*argc, argv, "s:E:b:t:")))
    {
        switch (*opt)
        {
        case 's':
            *s = atoi(optarg);
            break;
        case 'E':
            *E = atoi(optarg);
            break;
        case 'b':
            *b = atoi(optarg);
            break;
        case 't':
            *file_name = optarg;
            break;
        default:
            printf("unknown operation");
            exit(1);
        }
    }
}

void calculateSB(int *S, int *B, int s, int b)
{
    *S = (1 << s);
    *B = (1 << b);
}

void decode(int address, int *tagbits, int *setbits, int b, int s,int S)
{
    //this function extracts the tag and set bits from the address
    *tagbits = address >> (b + s);
    *setbits = (address>>b)&(S-1);
}

void execute(char identifier,int tagbits,int setbits,cache_line* cache, int E)
{
    cache_line* current_set = get_current_set(cache,setbits,E);
    int hits_flag = check_hits(current_set,E , tagbits);
    if(hits_flag)
    {  
        hits ++;
    }
    else if (!hits_flag)
    {
       int available_places = check_available_lines_and_insert(current_set,E ,tagbits);
       if (available_places)
       {
           miss ++;
       }
       else if (!available_places)
       {
           cache_line * victim = find_victim(current_set,E);
           evict_and_insert(victim, tagbits);
           miss ++;
           evict ++;
       }
    }
    if(identifier == 'M')
        {
            hits ++;
        }
    //updating the time stamp for the new round
    time ++;
}

int check_hits(cache_line*current_set , int E,int tagbits)
{
    cache_line* current_line = current_set;
    for(int i =0 ; i < E; i++)
    {
        current_line += i;
        if((current_line) -> valid ==1)
        {
            if (check_tagbits(current_line,tagbits))
            {
                //update the time stamp if there is a hit
                current_line->counter = time;
                return 1;
            }
        }
    }
    return 0;
}

int check_tagbits(cache_line*current_line , int tagbits )
{
    if (current_line ->tag == tagbits)
    {
        return 1;
    }
    else {
        return 0;
    }
}

cache_line* get_current_set(cache_line*cache, int setbits, int E)
{
    return (cache+(setbits*E));
}

int check_available_lines_and_insert(cache_line* current_set,int E , int tagbits)
{
    cache_line* current_line = current_set ;
    for(int i = 0; i< E; i++)
    {
        current_line += i;
        if (current_line->valid !=1 )
        {
            current_line ->tag = tagbits;
            current_line ->counter = time;
            current_line ->valid =1;
            return 1;
        }
    }
    return 0;
}

cache_line* find_victim(cache_line* current_set ,int E)
{
    /*returns a poiter to the victim by checking the minimum value of counter */
    int min = current_set->counter;
    cache_line *victim = current_set;
    for(int i=0 ; i < E ;i++)
    {
        current_set += i;
        if (current_set ->counter < min)
        {
            min = current_set->counter;
            victim = current_set;
        }
    }
    return victim;
}

void evict_and_insert(cache_line* victim,int tagbits)
{
    victim ->counter = time;
    victim ->tag = tagbits;
}

void debug(cache_line* cache,int S,int E)
{
    cache_line* current_line = cache;
    for (int i = 0 ; i<(S*E) ; i++)
    {
        current_line+=i ;
        printf("%d\t%d\t%d\n",current_line->tag,current_line->counter,current_line->valid);
        if((i+1)%E == 0)
        {
            printf("****************************************\n");
        }
    }

}