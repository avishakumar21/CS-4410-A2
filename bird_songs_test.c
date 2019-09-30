#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "rthread.h"
#define NWHISTLERS 3
#define NLISTENERS 3
#define NEXPERIMENTS 2
char *whistlers[NWHISTLERS] = { "w1", "w2", "w3" };
char *listeners[NLISTENERS] = { "l1", "l2", "l3" };
void worker(void *shared, void *arg){
struct device *dev = shared;
char *name = arg;
for (int i = 0; i < NEXPERIMENTS; i++) {
printf("worker %s waiting for device\n", name);
dev_enter(dev, name[0] == ’w’);
printf("worker %s has device\n", name);
rthread_delay(random() % 3000);
printf("worker %s releases device\n", name);
dev_exit(dev, name[0] == ’w’);
rthread_delay(random() % 3000);
}
printf("worker %s is done\n", name);
}
int main(){
struct device dev;
dev_init(&dev);
for (int i = 0; i < NWHISTLERS; i++) {
rthread_create(worker, &dev, whistlers[i]);
}
for (int i = 0; i < NLISTENERS; i++) {
rthread_create(worker, &dev, listeners[i]);
}
rthread_run();
return 0;
}
