#include <pthread.h>

#define rthread_lock_t		pthread_mutex_t

#define rthread_with(lock) \
	for (int _x_ = pthread_mutex_lock((lock)); _x_ < 1; pthread_mutex_unlock((lock)), _x_++) 

typedef struct {
	pthread_cond_t cond;
	rthread_lock_t *lock;
} rthread_cv_t;

typedef struct {
	rthread_lock_t lock;
	rthread_cv_t cv;
	unsigned int value;
} rthread_sema_t;

void rthread_create(void (*start_routine)(void *, void *), void *, void *);
void rthread_lock_init(rthread_lock_t *lock);
void rthread_lock_acquire(rthread_lock_t *lock);
void rthread_lock_release(rthread_lock_t *lock);
void rthread_cv_init(rthread_cv_t *cv, rthread_lock_t *lock);
void rthread_cv_wait(rthread_cv_t *cv);
void rthread_cv_signal(rthread_cv_t *cv);
void rthread_cv_broadcast(rthread_cv_t *cv);
void rthread_sema_init(rthread_sema_t *sema, unsigned int init_val);
void rthread_sema_procure(rthread_sema_t *sema);
void rthread_sema_vacate(rthread_sema_t *sema);
void rthread_delay(unsigned int msecs);
void rthread_run(void);