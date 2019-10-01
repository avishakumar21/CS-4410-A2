#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include "rthread.h"

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t start_cond = PTHREAD_COND_INITIALIZER;
static pthread_cond_t finish_cond = PTHREAD_COND_INITIALIZER;
static int started;
static int nthreads;

struct thread {
	void (*func)(void *monitor, void *arg);
	void *monitor, *arg;
};

static void *wrapper(void *arg){
	struct thread *t = arg;

	rthread_with(&mutex) {
		while (!started) {
			pthread_cond_wait(&start_cond, &mutex);
		}
	}

	(*t->func)(t->monitor, t->arg);

	rthread_with(&mutex) {
		if (--nthreads == 0) {
			pthread_cond_signal(&finish_cond);
		}
	}
	return 0;

}

void rthread_run(void){
	pthread_mutex_lock(&mutex);
	started = 1;
	pthread_cond_broadcast(&start_cond);
	while (nthreads > 0) {
		pthread_cond_wait(&finish_cond, &mutex);
	}
	pthread_mutex_unlock(&mutex);
}

void rthread_create(void (*func)(void *monitor, void *arg), void *monitor, void *arg){
	pthread_t tid;

	struct thread *t = malloc(sizeof(*t));
	t->func = func;
	t->monitor = monitor;
	t->arg = arg;
	int r = pthread_create(&tid, 0, wrapper, t);
	assert(r == 0);
	r = pthread_detach(tid);
	assert(r == 0);
	pthread_mutex_lock(&mutex);
	nthreads++;
	pthread_mutex_unlock(&mutex);
}

void rthread_lock_init(rthread_lock_t *lock){
	int r = pthread_mutex_init(lock, 0);
	assert(r == 0);
}

void rthread_lock_acquire(rthread_lock_t *lock){
	pthread_mutex_lock(lock);
}

void rthread_lock_release(rthread_lock_t *lock){
	pthread_mutex_unlock(lock);
}

void rthread_cv_init(rthread_cv_t *cv, rthread_lock_t *lock){
	int r = pthread_cond_init(&cv->cond, 0);
	assert(r == 0);
	cv->lock = lock;
}

void rthread_cv_wait(rthread_cv_t *cv){
	/* Simulate spurious wakeups to simplify testing.
	 */
	if (random() % 5 == 0) {
		return;
	}

	int r = pthread_cond_wait(&cv->cond, cv->lock);
	assert(r == 0);
}

void rthread_cv_signal(rthread_cv_t *cv){
	/* Simulate spurious wakeups to simplify testing.
	 */
	if (random() % 5 == 0) {
		int r = pthread_cond_broadcast(&cv->cond);
		assert(r == 0);
		return;
	}

	int r = pthread_cond_signal(&cv->cond);
	assert(r == 0);
}

void rthread_cv_broadcast(rthread_cv_t *cv){
	int r = pthread_cond_broadcast(&cv->cond);
	assert(r == 0);
}

void rthread_sema_init(rthread_sema_t *sema, unsigned int init_val){
	rthread_lock_init(&sema->lock);
	rthread_cv_init(&sema->cv, &sema->lock);
	sema->value = init_val;
}

void rthread_sema_procure(rthread_sema_t *sema){
	rthread_with(&sema->lock) {
		while (sema->value == 0) {
			rthread_cv_wait(&sema->cv);
		}
		sema->value--;
	}
}

void rthread_sema_vacate(rthread_sema_t *sema){
	rthread_with(&sema->lock) {
		sema->value++;
		rthread_cv_signal(&sema->cv);
	}
}

void rthread_delay(unsigned int msecs){
	usleep(1000L * msecs);
}