#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "rthread.h"

#define WHISTLER 0 // multiple applications may play sounds 
#define LISTENER 1 // multiple applications may record

struct device {

    rthread_sema_t mutex;
    rthread_sema_t whistlerSema;
    rthread_sema_t listenerSema;

    // Variables to keep track of the state.

    int nWhistlersEntered, nWhistlersWaiting;
    int nListenersEntered, nListenersWaiting;
};

void dev_init(struct device *dev)
{
    rthread_sema_init(&dev->mutex, 1);
    rthread_sema_init(&dev->whistlerSema, 0);
    rthread_sema_init(&dev->listenerSema, 0);
    dev->nWhistlersEntered = dev->nWhistlersWaiting = 0;
    dev->nListenersEntered = dev->nListenersWaiting = 0;
}

void dev_vacateOne(struct device *dev){
    /* If there are no listeners in the critical section and there are whistlers waiting,
       release one of the waiting whistlers. */

    if (dev->nListenersEntered == 0 && dev->nWhistlersWaiting > 0) 
    {
        dev->nWhistlersWaiting--;
        rthread_sema_vacate(&dev->whistlerSema);
    }

    /* If there are no whistlers in the critical section and there are listeners waiting,
       release one of the listeners. */

    else if (dev->nWhistlersEntered == 0 && dev->nListenersWaiting > 0)
    {
        dev->nListenersWaiting--;
        rthread_sema_vacate(&dev->listenerSema);
    }

    // If neither is the case, just stop protecting the shared variables.
     
    else
    {
        rthread_sema_vacate(&dev->mutex);
    }
}

void dev_enter(struct device *dev, int which)
{
	rthread_sema_procure(&dev->mutex);
	
	if (which == 0) // whistler - corresponds to reader
	{
		assert(dev->nListenersEntered == 0 || (dev->nListenersEntered == 1 && dev->nWhistlersEntered == 0));

    	if (dev->nListenersEntered > 0) 
    	{
	        dev->nWhistlersWaiting++;
	        dev_vacateOne(dev);
	        rthread_sema_procure(&dev->whistlerSema);
    	}

	    assert(dev->nListenersEntered == 0);
	    dev->nWhistlersEntered++;
	    dev_vacateOne(dev);

	}
	else if (which == 1) // listening
	{
		assert(dev->nWhistlersEntered == 0 || (dev->nWhistlersEntered == 1 && dev->nListenersEntered == 0));

    	if (dev->nWhistlersEntered > 0) 
    	{
	        dev->nListenersWaiting++;
	        dev_vacateOne(dev);
	        rthread_sema_procure(&dev->listenerSema);
    	}

	    assert(dev->nWhistlersEntered == 0);
	    dev->nListenersEntered++;
	    dev_vacateOne(dev);
	}
	else
	{
		// throw error that which is not a valid parameter 
		printf("which is not a valid number (0 or 1), which is %d\n", which);
	}
}

void dev_exit(struct device *dev, int which)
{
	rthread_sema_procure(&dev->mutex);

	if (which == 0)
	{
	    assert(dev->nListenersEntered == 0);
	    assert(dev->nWhistlersEntered > 0);
	    dev->nWhistlersEntered--;
	    dev_vacateOne(dev);
	}
	else if (which == 1)
	{
    	assert(dev->nWhistlersEntered == 0);
    	assert(dev->nListenersEntered > 0);
	    dev->nListenersEntered--;
	    dev_vacateOne(dev);
	}
	else
	{
		//throw error that which is not a valid parameter 
		printf("which is not a valid number (0 or 1), which is %d\n", which);
	}
}

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