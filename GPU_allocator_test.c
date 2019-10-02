#include <stdio.h>
#include <stdlib.h>
#include "rthread.h"

#define NGPUS 10
rthread_lock_t print_lock;
// YOUR CODE GOES HERE
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
