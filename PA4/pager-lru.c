#include <stdio.h> 
#include <stdlib.h>
#include "simulator.h"

void pageit(Pentry q[MAXPROCESSES]) { 
    
    /* This file contains the stub for an LRU pager */
    /* You may need to add/remove/modify any part of this file */

    /* Static vars */
    static int initialized = 0;
    static int tick = 1; // artificial time
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];

    /* Local vars */
    int proctmp;
    int pagetmp;

    /* initialize static vars on first run */
    if(!initialized){
	for(proctmp=0; proctmp < MAXPROCESSES; proctmp++){
	    for(pagetmp=0; pagetmp < MAXPROCPAGES; pagetmp++){
		timestamps[proctmp][pagetmp] = 0; 
	    }
	}
	initialized = 1;
    }
    
    /* TODO: Implement LRU Paging */
    //fprintf(stderr, "pager-lru not yet implemented. Exiting...\n");
    // exit(EXIT_FAILURE);
	
	// Page value computed by pc/Pagesize	
	int page;
	// process # in runqueue 0-19
	int process;
	// the temp value where we store the timestamp of the lru page
	int lru;
	// page in process
	int current_page;
	// the one we're gonna evict. Sad... / the oldest one /
	int the_one;
	// current time/tick minus the timestamp of the current checking page
	int time;

	//running through processes in runqueue
	for(process=0; process <MAXPROCESSES; process++){
		// if the process is active then we do our job
//		printf(q[process].pages);
		if(q[process].active){
			// getting the page count using program counter
			page = q[process].pc / PAGESIZE	;
//			printf(q[process].pc);
			// recording the current timestamp for the current process
			timestamps[process][page] = tick;
			// if the page is not allocated then we have to pagein
			if(q[process].pages[page] == 0){
				// if pagein is not in progress for the missing pages
				if(pagein(process,page)==0){
					// initialize the lru into value less than 0 to keep the track of the oldest time in the page sequence
					lru = -1;
					// loop through pages in current process
					for(current_page = 0; current_page < MAXPROCPAGES; current_page++){
						// calculating the time since the last usage
						time = tick - timestamps[process][current_page];
						// if the calculated time is older than recorded lru from beforehand and it's in the queue
						// record that bad boy
						if(time > lru && q[process].pages[current_page]){
							// currently the page to evict but might find something useful later on
							the_one = current_page;
							// record the lru value of the_one page in the runtime queue
							lru = tick - timestamps[process][current_page];
						}
					}
					// evict that oldest page.
					pageout(process,the_one);
				}
			}
		}
		// increment tick after pageit 
		//tick++;
	}
    /* advance time for next pageit iteration */
        tick++;
} 
