#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "rthread.h"

#define NGPUS 10

rthread_lock_t print_lock;

struct gpu_info {
	int allocated[NGPUS];
	unsigned int nfree;
    rthread_sema_t mutex_procure;  
    rthread_sema_t mutex_track; //to keep track of the state
    rthread_sema_t overallocation_sema;
	rthread_sema_t sema;
};

void gi_init(struct gpu_info *gi){
	memset(gi->allocated, 0, sizeof(gi->allocated));
	gi->nfree = NGPUS;
	rthread_sema_init(&gi->mutex_procure, 1);
	rthread_sema_init(&gi->mutex_track, 1);
	rthread_sema_init(&gi->sema, 1);
	rthread_sema_init(&gi->overallocation_sema, 0);
}

void gi_alloc(struct gpu_info *gi, unsigned int ngpus, /* OUT */ unsigned int gpus[]){
	// if (ngpus > 10){
	// 	//cause wait forever without having a busy wait 
	// 	rthread_sema_procure(&gi->overallocation_sema); //block forever, do not have a vacate 
	// }

	rthread_sema_procure(&gi->mutex_procure);

	for (unsigned int i = 0; i < ngpus; i++) {
		rthread_sema_procure(&gi->sema);
	}
	rthread_sema_vacate(&gi->mutex_procure);

	rthread_sema_procure(&gi->mutex_track);
	assert(ngpus <= gi->nfree);
	gi->nfree -= ngpus;
	unsigned int g = 0;

	for (unsigned int i = 0; i < ngpus; i++) {
		assert(i < NGPUS);
		if (!gi->allocated[i]) {
			gi->allocated[i] = 1;
			gpus[g++] = i;
		}
	}
	rthread_sema_vacate(&gi->mutex_track);
}

void gi_release(struct gpu_info *gi, unsigned int ngpus, /* IN */ unsigned int gpus[]){
	
	for (unsigned int g = 0; g < ngpus; g++) {
		rthread_sema_vacate(&gi->sema);
	}

	rthread_sema_procure(&gi->mutex_track); 

	for (unsigned int g = 0; g < ngpus; g++) {
		assert(gpus[g] < NGPUS);
		assert(gi->allocated[gpus[g]]);
		gi->allocated[gpus[g]] = 0;
	}
	gi->nfree += ngpus;
	assert(gi->nfree <= NGPUS);

	rthread_sema_vacate(&gi->mutex_track); 
}

void gi_free(struct gpu_info *gi){
}


void gpu_user(void *shared, void *arg) {
	struct gpu_info *gi = shared;
	unsigned int gpus[NGPUS];
	for (int i = 0; i < 5; i++) {
		rthread_delay(random() % 3000);
		unsigned int n = 1 + (random() % NGPUS);
		rthread_with(&print_lock)
		printf("%s %d wants %u gpus\n", arg, i, n);
		gi_alloc(gi, n, gpus);
		rthread_with(&print_lock) {
			printf("%s %d allocated:", arg, i);
			for (int i = 0; i < n; i++)
				printf(" %d", gpus[i]);
			printf("\n");
		}
		rthread_delay(random() % 3000);
		rthread_with(&print_lock)
		printf("%s %d releases gpus\n", arg, i);
		gi_release(gi, n, gpus);
	}
}

int main() {
	rthread_lock_init(&print_lock);
	struct gpu_info gi;
	gi_init(&gi);
	rthread_create(gpu_user, &gi, "Jane");
	rthread_create(gpu_user, &gi, "Joe");
	rthread_run();
	gi_free(&gi);
	return 0;
}
