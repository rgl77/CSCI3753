#include <stdio.h> 
#include <stdlib.h>
// Assertion 
#include <assert.h>
// Used it to bring INTMAX 
#include <limits.h>
#include "simulator.h"
//Boolean header
#include <stdbool.h>

// Creating struct to count the frequency of page s
struct stat{
    int page;
    int frequency;
    int *timestamp;
};

typedef struct stat Stat;

// function where we swap 2 values
// it is only used in the sorting function
void swapppper(Stat *l, Stat* r){
    Stat tmp = *l;
    *l = *r;
    *r = tmp;
}

// function biggie is where I sort the prediction array
// The sort function is based on bubble-ish sorting algorithm 
// where we use swappper function to swap values until everything is in order
//Descending order

void biggie(Stat *predictions){
    // get the size of the input variable
    int size = 0;
    while(predictions[size].page != -1 && size<MAXPROCPAGES){
        size++;
    }
    // declare temp variable and y is just binary value 
    // to check whether we did some job or not 
    // and condition for the while loop
    int x;
    int y = 0;
    // loop where we do the sorting
    // loop will run until there are no values that are needed to swap
    // no so efficient but will do the job
    do{
        y = 0;
        // start the indexing at x=1 because we will try to access x-1 throughout the loop
        for(x=1;x<size;x++){
            // using assert to do some diagnostics because we might get a segfault by accessing NULL value
            //assert(predictions[x-1].timestamp != NULL && predictions[x].timestamp != NULL);
	    assert(predictions[x-1].frequency!=-1 && predictions[x].frequency != -1);
            // if previous value is less than current value then we change it
            // in order to put the array in descending order
	    if(predictions[x-1].frequency > predictions[x].frequency){
		swapppper(predictions+(x-1),predictions+x);
	  	y= 1;
	    }
	    /*
            if(*(predictions[x-1].timestamp) < *(predictions[x].timestamp)){
                // using the function previously declared
                // we will swap those 2 values
                swapppper(predictions+(x-1),predictions+x);
                y = 1;
            }*/
        }
    }while(y);
}
void pageit(Pentry q[MAXPROCESSES]) { 
    
    /* This file contains the stub for a predictive pager */
    /* You may need to add/remove/modify any part of this file */

    /* Static vars */
    // initialized variable, just a binary marker
    static int initialized = 0;
    // artificial time
    static int tick = 1; 
    // timestamps array with size of MAXPROCESSES*MAXPROCPAGES 
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];
    // to keep in track of process status
    static int process[MAXPROCESSES];
    // to keep track of program counter
    static int program_counter[MAXPROCESSES];
    // initialize a 3 dimensional array, you'll see the usage 
    // in the following codes below.
    static Stat history[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES];

    /* Local vars */
    // loop iterators
    int temp_process;
    int temp_page;
    // last page
    int last;
    // lru page
    int lru;
    // current page
    int cur;

    /* initialize static vars on first run */
    if(!initialized){
    /* Initialize complex static vars here */
        int x,y,z;
        for(x=0;x<MAXPROCESSES;x++){
            for(y=0;y<MAXPROCPAGES;y++){
                for(z=0;z<MAXPROCPAGES;z++){
                    // give them some value that should not be occur during the period of run-time
                    history[x][y][z].page = -1;
                    history[x][y][z].frequency = -1;
                    history[x][y][z].timestamp = NULL;
                }
            }
        }
        // set the timestamps for each process page to 0
        for(temp_process=0;temp_process < MAXPROCESSES; temp_process++){
            for(temp_page=0;temp_page<MAXPROCPAGES;temp_page++){
                timestamps[temp_process][temp_page] = 0;
            }
            process[temp_process] = 0;
        }
        // say that the back-bone dimensions are initialized
        initialized = 1;
    }

    // loop through every process
    for(temp_process=0;temp_process<MAXPROCESSES;temp_process++){
        // skip if the process is inactive
        if(!q[temp_process].active)
            continue;
        // skip if the last page is still not stored yet
        if(last == -1)
            continue;
        // record the last page of current process from the program_counter
        last = program_counter[temp_process]/PAGESIZE;
        // update the program counter of the current process in other array that we will be using 
        // in next iteration to see what page is used for that process
        program_counter[temp_process] = q[temp_process].pc;
        // get the current page in use
        cur = q[temp_process].pc/PAGESIZE;
        // if last used page is equal to current page 
        // no labor is needed
        if(last == cur)
            continue;
        // if the last page is different than current page
        // then pageout that last page
        pageout(temp_process,last);
        //loop counter
        int x;
        // duplicate pointer
        Stat *index;
        // index is the third dimension for the last used page for the current process
        index = history[temp_process][last];
        // loop through the 3rd dimension we talked about earlier
        for(x=0;x<MAXPROCPAGES;x++){
            // if the page is equal to current page
            // increase the frequency of currently used page
            // meaning that, CURRENT page is most likely to occur after the LAST page
            // we're trying to understand the pattern during the runtime
            // Break the loop after incrementing the frequency of this matter
            if(index[x].page == cur){
                index[x].frequency++;
                break;
            }
            // if the last page is never faced this situation before
            // and the variables are at initial values
            // then we record current state ,page and timestamp
            // After all that we will break the loop.
            else if(index[x].page == -1){
                index[x].page = cur;
                index[x].frequency = 1;
                index[x].timestamp = &(timestamps[temp_process][x]);
                break;
            }
        }   
    }


    // updating the timestamp of every active process to the current tick
    // because we need the latest update for each iteration to find the optimal prediction
    for(temp_process=0;temp_process<MAXPROCESSES;temp_process++){
        if(!q[temp_process].active)
            continue;
        temp_page = (q[temp_process].pc)/PAGESIZE;
        timestamps[temp_process][temp_page] = tick;
    }

    // loop through every process again
    for(temp_process=0;temp_process<MAXPROCESSES;temp_process++){
        // if process is inactive then pageout it's pages
        // like a cleaning service and skip the ahead
        if(!q[temp_process].active){
            for(temp_page=0;temp_page<MAXPROCPAGES;temp_page++){
                pageout(temp_process,temp_page);
            }
            continue;
        }
        // if process is not inactive then get the current page
        temp_page = q[temp_process].pc/PAGESIZE;
        // if the page is already allocated in the memory the skip the ahead
        if(q[temp_process].pages[temp_page]==1)
            continue;
        // if the page is not allocated in the memory
        // then try to allocate it in memory
        if(pagein(temp_process,temp_page)){
            // say that process is no longer waiting for anything because the pagein is successful
            process[temp_process] = 0;
            continue;
        }
        // process is waiting for pageout 
        if(process[temp_process] == 1)
            continue;
        
        // let's try to find the current least recently used page 
        // initialize the loop counters which should be different than the current ones
        // status is variable where it says that we found lru page and that is most of the times
        int temp_process_1,temp_page_1;
        int recent = INT_MAX;
        bool status = FALSE;
        for(temp_process_1=0;temp_process_1<MAXPROCESSES;temp_process_1++){
            for(temp_page_1=0;temp_page_1<MAXPROCPAGES;temp_page_1++){
                // if the page is allocated and timestamp is lower than previous checked page
                if(q[temp_process_1].pages[temp_page_1] == 1 && timestamps[temp_process_1][temp_page_1] < recent)
                    // then the recent should be 
                    recent = timestamps[temp_process_1][temp_page_1];
                    // save that LRU
                    lru = temp_page_1;
                    // Status is found the LRU
                    status = TRUE;
            }
        }
        // if we found the LRU then just ignore the following
        if(status == TRUE)
            continue;
        // If we did not found the LRU page then
        // try to page out the previously saved lru
        if(!pageout(temp_process,lru)){
            fprintf(stderr,"ERROR: Paging out the LRU page failed!\n");
            exit(EXIT_FAILURE);
        }
        // if previous lru is failed too
        // then say that current process should wait for some more 
        process[temp_process]=1;
        
    }
    
    // for every process which are still active
    for(temp_process = 0;temp_process<MAXPROCESSES;temp_process++){
        // where the magic happens and we store the page, frequency and timestamp of a particular page
        Stat *magic;
        // looper
        int x;
        // if not active then just continue the loop
        if(!q[temp_process].active)
            continue;
        // current program counter from the runtime queue
        int cur_pc = q[temp_process].pc;
        // magic page time travels by 101 clicks because that was the optimal tick for me
        magic = history[temp_process][(cur_pc+101)/PAGESIZE];
        // sort the previous behaviours by the timestamp
        // in order to get the most likely outcome
        biggie(magic);
        // get the size of the current predictions
        int size = 0;
        while(magic[size].page!= -1 && size<MAXPROCPAGES){
            size++;
        }
        // page in the most likely pages
        for(x = 0; x<size;x++){
            pagein(temp_process,magic[x].page);
        }
    }
    /* advance time for next pageit iteration */
    tick++;
} 
