#include "multi-lookup.h"
#include "queue.h"
#include <sys/types.h>
#include <sys/syscall.h>
#include <time.h>

void* request(void* p){
	char hostname[MAX_NAME_LENGTH];
	inP* inputs = p;
	FILE* fn    = inputs->file_name;
	pthread_mutex_t* qlock = inputs->qLock;
	pthread_mutex_t* slock = inputs->sLock;
	queue* buffer = inputs->q;
	FILE* service = inputs->service_name;
	char* pload ;
	int success = 0;
	int err = 0;
	pid_t tid = syscall(SYS_gettid);
	pthread_mutex_lock(slock);
	fprintf(service,"thread with id %d is done working with names%d.txt\n",tid,inputs->num);
	pthread_mutex_unlock(slock);
	while(fscanf(fn, "%1024s",hostname) > 0){
		while(success==0){
			err = pthread_mutex_lock(qlock);
			if(err){
				fprintf(stderr, "Mutex lock error # %d in request\n",err);
			}
			if(qfull(buffer)){
				err = pthread_mutex_unlock(qlock);
				if(err){
					fprintf(stderr, "Mutex unlock error # %d in request\n",err);
				}
				usleep((rand()%100)*1000);
			}else{
				pload = malloc(1025);
				if(pload == NULL){
					fprintf(stderr, "Malloc returned error\n");
				}
				pload = strncpy(pload, hostname, 1025);
				if(q_push(buffer,pload) == -1){
					fprintf(stderr,"push error\n");
				}
				err = pthread_mutex_unlock(qlock);
				if(err){
					fprintf(stderr,"unlock error %d",err);
				}
				success=1;
			}
		}
		success = 0;
	}
//	free(pload);
	fclose(fn);
	return NULL;
}


void *resolve(void*p){
	outP* inputs = p;
	FILE* outfile = inputs->file_name;
	pthread_mutex_t* qlock = inputs->qLock;
	pthread_mutex_t* olock = inputs->oLock;
	queue* buffer = inputs->q;
	int* req_exist = inputs->request_exist;
	char* pload;
	char ip[INET6_ADDRSTRLEN];
	int err = 0;
	while( *req_exist || !qempty(buffer)){
		err = pthread_mutex_lock(qlock);
		if(err){
			fprintf(stderr, "q mutex lock err %d\n", err);
		}
		pload = q_pop(buffer);
		if(pload==NULL){
			err = pthread_mutex_unlock(qlock);
			if(err){
				fprintf(stderr, "q mutex unlock err %d\n",err);
			}
			usleep((rand()%100)*1000);
		}else{
			err = pthread_mutex_unlock(qlock);
			if(err){
				fprintf(stderr, "q mutex unlock err %d\n",err);
			}
			if(dnslookup(pload,ip,sizeof(ip)) == -1){
				fprintf(stderr, "lookup err: %s\n",pload);	
				strncpy(ip,"",sizeof(ip));
			}
//			printf("%s, %s",pload,ip);
			err = pthread_mutex_lock(olock);
			if(err){
				fprintf(stderr,"output file mutex lock err%d\n",err);		
			}
//			printf("%s, %s\n",pload,ip);
			err=fprintf(outfile,"%s,%s\n",pload,ip);
			if(err<0){
				fprintf(stderr, "output file write error %d\n",err);
			}	
			err = pthread_mutex_unlock(olock);
			if(err){
				fprintf(stderr, "output file mutex unlock err %d\n",err);
			}
			free(pload);
			pload=NULL;
		}
	}
	return NULL;
}

int main(int argc, char* argv[]){
	printf("entering the main\n");
	int inputnum = argc-5;
	int input_available = 1;
	clock_t t;
	t = clock();
	double cpu_time;	
	FILE* inputFiles[inputnum];
	FILE* outputFile = NULL;
	FILE* serviceFile = NULL;	
        printf("yo");	
	int argv1 = atoi(argv[1]);
	int argv2 = atoi(argv[2]);
	pthread_t in_th[argv1];
	pthread_t o_th[argv2];
	int filenumber;	

	pthread_mutex_t qlock;
	pthread_mutex_t olock;
	pthread_mutex_t slock;
	queue buffer;
	
	inP inputs[argv1];
	outP outputs[argv2];
	int err=0;
	if(queue_i(&buffer,200)==-1){
		fprintf(stderr, "error creating the shared array\n");
		return -1;
	}
	err = pthread_mutex_init(&qlock,NULL);
	if(err){
		fprintf(stderr, "error creating the q mutex %d\n",err);
		return -1;
	}
	err = pthread_mutex_init(&olock,NULL);
	if(err){
		fprintf(stderr, "error creating output file mutex %d\n",err);
		return -1;
	}
	err = pthread_mutex_init(&slock,NULL);
	if(err){
		fprintf(stderr,"error creating service file mutex %d\n",err);
	}
	
	serviceFile = fopen(argv[3],"w");
	if(!serviceFile){
		fprintf(stderr,"error opening the bogus service file\n");
		return -1;
	}

	outputFile = fopen(argv[4],"w");
	if(!outputFile){
		fprintf(stderr, "error opening the bogus output file\n");
		return -1;
	}
	int i;
	int j;
	for(i=5;i<argc;i++){
		j = i%5;
		inputFiles[j]=fopen(argv[i],"r");
		if(!inputFiles[j]){
			fprintf(stderr, "error opening bogus input file%s\n",argv[i]);
		}
	}	
	int file_index=0;
	int w;

	while(file_index<inputnum){
		for(w=0;w<argv1;w++){
			if(file_index<inputnum){
				FILE *curr= inputFiles[file_index];
				inputs[w].sLock=&slock;
				inputs[w].qLock=&qlock;
				inputs[w].file_name=curr;
				inputs[w].q = &buffer;
				inputs[w].service_name = serviceFile;
				inputs[w].num = file_index+1;
				file_index++;
				err=pthread_create(&(in_th[w]),NULL,request,&inputs[w]);
				if(err){
					fprintf(stderr, "error in req thread cre %d\n",err);
					exit(-1);
				}
			}
		}
		for(w=0;w<argv1;w++){
			if(in_th[w]){
			pthread_join(in_th[w],NULL);
			}
		}
		//usleep((rand()%100)*1000); 
	}

	for(w=0; w<argv2;w++){
                outputs[w].qLock = &qlock;
                outputs[w].file_name = outputFile;      
                outputs[w].oLock = &olock;
                outputs[w].q = &buffer;
                outputs[w].request_exist=&input_available;
                err = pthread_create(&(o_th[w]),NULL,resolve, &(outputs[w]));
                if(err){
                        fprintf(stderr,"error in res thread creation %d\n",err);
                        exit(-1);
                }
        }
	printf("closing the input threads\n");
/*
        for(w=0;w<argv1;w++){
		if(in_th[w]){
                pthread_join(in_th[w],NULL);
        	}
	}
*/
	printf("closed the input threads, now closing the output threads\n");
        input_available = 0;
        for(w =0; w<argv2; w++){
		if(o_th[w]){
                pthread_join(o_th[w],NULL);
        	}
	}
	t = clock() - t;
        cpu_time = ((double) t)/CLOCKS_PER_SEC;
        printf("%d\n",t);
	printf("closed the output threads, cleaning up the mutexes and buffer\n");
	fclose(outputFile);
	fclose(serviceFile);
	q_clean(&buffer);
	fcloseall();
	pthread_mutex_destroy(&qlock);
	pthread_mutex_destroy(&olock);
	pthread_mutex_destroy(&slock);
	
	
	return 0;
}	
