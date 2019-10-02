#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "rthread.h"

#define NGPUS 10
struct gpu_info {
	int allocated[NGPUS];
	unsigned int nfree;
    rthread_sema_t mutex_procure;  
    rthread_sema_t mutex_track; //to keep track of the state
    //if there is gpu is equal to 20, block
    //need another mutex for 20 gpus
	rthread_sema_t sema;
};

void gi_init(struct gpu_info *gi){
	memset(gi->allocated, 0, sizeof(gi->allocated));
	gi->nfree = NGPUS;
	rthread_sema_init(&gi->mutex_procure, 1);
	rthread_sema_init(&gi->mutex_track, 1);
	rthread_sema_init(&gi->lock, 0);
}

void gi_alloc(struct gpu_info *gi, unsigned int ngpus, /* OUT */ unsigned int gpus[]){
	rthread_sema_procure(&gi->mutex_procure);

	for (unsigned int i = 0; i < ngpus; i++) {
		rthread_sema_procure(&gi->sema);
	}
	assert(ngpus <= gi->nfree);
	gi->nfree -= ngpus;
	unsigned int g = 0;

	for (unsigned int i = 0; i < ngpus; i++) {
		rthread_sema_procure(&gi->mutex_state); //does this go here 
		assert(i < NGPUS);
		if (!gi->allocated[i]) {
			gi->allocated[i] = 1;
			gpus[g++] = i;
		}
	}

}

void gi_release(struct gpu_info *gi, unsigned int ngpus, /* IN */ unsigned int gpus[]){
	for (unsigned int g = 0; g < ngpus; g++) {
		rthread_sema_vacate(&gi->sema);
	}

	for (unsigned int g = 0; g < ngpus; g++) {
		assert(gpus[g] < NGPUS);
		assert(gi->allocated[gpus[g]]);
		gi->allocated[gpus[g]] = 0;
	}
	gi->nfree += ngpus;
	assert(gi->nfree <= NGPUS);

	rthread_sema_vacate(&gi->mutex_procure);

}

void gi_free(struct gpu_info *gi){
}
