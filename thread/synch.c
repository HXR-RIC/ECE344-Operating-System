/*
 * Synchronization primitives.
 * See synch.h for specifications of the functions.
 */

#include <types.h>
#include <lib.h>
#include <synch.h>
#include <thread.h>
#include <curthread.h>
#include <machine/spl.h>

////////////////////////////////////////////////////////////
//
// Semaphore.

struct semaphore *
sem_create(const char *namearg, int initial_count)
{
	struct semaphore *sem;

	assert(initial_count >= 0);

	sem = kmalloc(sizeof(struct semaphore));
	if (sem == NULL) {
		return NULL;
	}

	sem->name = kstrdup(namearg);
	if (sem->name == NULL) {
		kfree(sem);
		return NULL;
	}

	sem->count = initial_count;
	return sem;
}

void
sem_destroy(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);

	spl = splhigh();
	assert(thread_hassleepers(sem)==0);
	
	/*
	 * Note: while someone could theoretically start sleeping on
	 * the semaphore after the above test but before we free it,
	 * if they're going to do that, they can just as easily wait
	 * a bit and start sleeping on the semaphore after it's been
	 * freed. Consequently, there's not a whole lot of point in 
	 * including the kfrees in the splhigh block, so we don't.
	 */

	kfree(sem->name);
	kfree(sem);
	splx(spl);
}

void 
P(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);

	/*
	 * May not block in an interrupt handler.
	 *
	 * For robustness, always check, even if we can actually
	 * complete the P without blocking.
	 */
	assert(in_interrupt==0);

	spl = splhigh();
	while (sem->count==0) {
		thread_sleep(sem);
	}
	assert(sem->count>0);
	sem->count--;
	splx(spl);
}

void
V(struct semaphore *sem)
{
  //  kprintf("id: %d\n",curthread->pid);
	int spl;
	assert(sem != NULL);
	spl = splhigh();
	sem->count++;
       // kprintf("sem count is %d\n",sem->count);
        if(sem->count<=0)
            sem->count=1;
	assert(sem->count>0);
	thread_wakeup(sem);
	splx(spl);
}

////////////////////////////////////////////////////////////
//
// Lock.

struct lock *
lock_create(const char *name)
{
	struct lock *lock;

	lock = kmalloc(sizeof(struct lock));
	if (lock == NULL) {
		return NULL;
	}

	lock->name = kstrdup(name);//kstrdup() duplicate strings
	if (lock->name == NULL) {
		kfree(lock);
		return NULL;
	}
	//lock->name exist and return it	
	lock->held=0;
	lock->curthread_holder=NULL;	
	lock->held = 0;

	return lock;
}

void
lock_destroy(struct lock *lock)
{
	assert(lock != NULL);
	int spl;
	spl=splhigh();
	// add stuff here as needed
	
	kfree(lock->name);
	kfree(lock);
	splx(spl);
}

void
lock_acquire(struct lock *lock)
{
	// Write this
	
	//disable interrupt
	int spl = splhigh();
        assert(in_interrupt==0);
	while (lock->held==1)
	{
		thread_sleep(lock);
	}

	lock->held = 1;
	lock->curthread_holder = curthread;
	splx(spl);
	
	//(void)lock;  // suppress warning until code gets written
}





void
lock_release(struct lock *lock)
{
	// Write this
	
	int spl = splhigh();
	
	thread_wakeup(lock);
	lock->held = 0;
	lock->curthread_holder = NULL;
	
	
	splx(spl);

	(void)lock;  // suppress warning until code gets written
}


int
lock_do_i_hold(struct lock *lock)
{
	// Write this

	(void)lock;  // suppress warning until code gets written
	if (lock->curthread_holder == curthread)
		return 1;    // dummy until code gets written
	else
		return 0;
}	

////////////////////////////////////////////////////////////
//
// CV


struct cv *
cv_create(const char *name)
{
	struct cv *cv;

	cv = kmalloc(sizeof(struct cv));
	if (cv == NULL) {
		return NULL;
	}

	cv->name = kstrdup(name);
	if (cv->name==NULL) {
		kfree(cv);
		return NULL;
	}
	
	// add stuff here as needed
	
	return cv;
}

void
cv_destroy(struct cv *cv)
{
	assert(cv != NULL);

	// add stuff here as needed
	int spl;
	spl=splhigh();
	kfree(cv->name);
	kfree(cv);
	splx(spl);
}

void
cv_wait(struct cv *cv, struct lock *lock)
{	
	int spl = splhigh();

	lock_release(lock);
	thread_sleep(cv);
	
	splx(spl);
	lock_acquire(lock);	
	// Write this
       //Release the supplied lock, go to sleep, and, after waking up again, re-acquire the lock.
	//(void)cv;    // suppress warning until code gets written
	//(void)lock;  // suppress warning until code gets written
}

void
cv_signal(struct cv *cv, struct lock *lock)
{
	// Write this
	//cv_signal    - Wake up one thread that's sleeping on this CV.
	int spl;
	spl=splhigh();
	if(thread_hassleepers(cv))//check if thread on this condtion is not empty
		thread_wakeup_signal(cv);
		
//this function only wake up one thread
	splx(spl);
	//(void)cv;    // suppress warning until code gets written
	//(void)lock;  // suppress warning until code gets written
}

void
cv_broadcast(struct cv *cv, struct lock *lock)
{
	 // Write this
        //  Wake up all threads sleeping on this CV.
	int spl;
	spl=splhigh();
	thread_wakeup(cv);
	splx(spl);
	//(void)cv;    // suppress warning until code gets written
	//(void)lock;  // suppress warning until code gets written
}
