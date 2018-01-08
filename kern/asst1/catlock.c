/*
 * catlock.c
 *
 * 30-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: Please use LOCKS/CV'S to solve the cat syncronization problem in 
 * this file.
 */


/*
 * 
 * Includes
 *
 */

#include <types.h>
#include <lib.h>
#include <test.h>
#include <thread.h>
#include <synch.h>
/*
 * 
 * Constants
 *
 */

/*
 * Number of food bowls.
 */

#define NFOODBOWLS 2

/*
 * Number of cats.
 */

#define NCATS 6

/*
 * Number of mice.
 */

#define NMICE 2

struct lock* lock1;
struct lock* Lock;
struct lock *lock2;
struct cv* catEat;
struct cv* mouseEat;
int num_cat_eat = 0;
int num_mouse_eat = 0;
int nbowl_available = 2;
int bowlB[] = {0, 0};
/*
 * 
 * Function Definitions
 * 
 */

/* who should be "cat" or "mouse" */
static void
lock_eat(const char *who, int num, int bowl, int iteration) {

    kprintf("%s: %d starts eating: bowl %d, iteration %d\n", who, num,
            bowl, iteration);
    clocksleep(1);
    kprintf("%s: %d ends eating: bowl %d, iteration %d\n", who, num,
            bowl, iteration);
}

/*
 * catlock()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.sss
 *      unsigned long catnumber: holds the cat identifier from 0 to NCATS -
 *      1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using locks/cv's.
 *
 */

static
void
catlock(void * unusedpointer,
        unsigned long catnumber) {

    int iteration;
    const char *who = "cat";
    for (iteration = 0; iteration < 4; iteration++) {
        lock_acquire(Lock); //it blocks others to enter the critical region
        
        while (nbowl_available == 0 || num_mouse_eat > 0) {//if num_mouse=0 and cat will reach at this point. First need to check the avilable number of bowl.If not, wait
            cv_wait(catEat, Lock);
            //lock_release(lock);
        }
        //reach this point, some value need to change 
        num_cat_eat++;
        nbowl_available--;

        if (bowlB[0] == 0) {//bowl1 is avilable                       
            bowlB[0] = 1;
            lock_release(Lock);
            lock_acquire(lock1);
            lock_eat(who, catnumber, 1, iteration);
            lock_release(lock1);
            lock_acquire(Lock);
            bowlB[0] = 0;
            nbowl_available++;
            num_cat_eat--;
            cv_broadcast(catEat,Lock);
            cv_broadcast(mouseEat,Lock);
            lock_release(Lock);
        }
        else if (bowlB[1] == 0) {
            bowlB[1] = 1;     
            lock_release(Lock);
            lock_acquire(lock2);
            lock_eat(who, catnumber, 2, iteration);
            lock_release(lock2);
            lock_acquire(Lock);
            bowlB[1] = 0;
            nbowl_available++;
            num_cat_eat--;
            cv_broadcast(catEat, Lock);
            cv_broadcast(mouseEat, Lock);
            lock_release(Lock);
        }
    }
}

/*
 * mouselock()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long mousenumber: holds the mouse identifier from 0 to 
 *              NMICE - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using locks/cv's.
 *
 */

static
void
mouselock(void * unusedpointer,
        unsigned long mousenumber) {
    int iteration;
    const char *who = "mouse";
    for (iteration = 0; iteration < 4; iteration++) {
        lock_acquire(Lock); //it blocks others to enter the critical region
        
        while (nbowl_available == 0 || num_cat_eat > 0) {//if num_mouse=0 and cat will reach at this point. First need to check the avilable number of bowl.If not, wait
            cv_wait(mouseEat, Lock);       
        }
        //reach this point, some value need to change 
        num_mouse_eat++;
        nbowl_available--;
        //at this point we need to check which bowl is available for the cat
        if (bowlB[0] == 0) {//bowl1 is avilable
            bowlB[0] = 1;
            lock_release(Lock);
            lock_acquire(lock1);
            lock_eat(who, mousenumber, 1, iteration);
            lock_release(lock1);         
            lock_acquire(Lock);
            bowlB[0] = 0;
            nbowl_available++;
            num_mouse_eat--;
            cv_broadcast(catEat, Lock);
            cv_broadcast(mouseEat, Lock);
            lock_release(Lock);

        } else if (bowlB[1] == 0) {
            bowlB[1] = 1;
            lock_release(Lock);
            lock_acquire(lock2);
            lock_eat(who, mousenumber, 2, iteration);
            lock_release(lock2);
            lock_acquire(Lock);
            bowlB[1] = 0;
            nbowl_available++;
            num_mouse_eat--;
            cv_broadcast(catEat, Lock);
            cv_broadcast(mouseEat, Lock);
            lock_release(Lock);

        }
    }
}

/*
 * catmouselock()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up catlock() and mouselock() threads.  Change
 *      this code as necessary for your solution.
 */

int
catmouselock(int nargs,
        char ** args) {
    int index, error;

    /*
     * Avoid unused variable warnings.
     */

    (void) nargs;
    (void) args;

    /*
     * Start NCATS catlock() threads.
     */
    lock1 = lock_create("lock1");
    Lock = lock_create("Lock");
    lock2 = lock_create("lock2");
    catEat = cv_create("catEat");
    mouseEat = cv_create("mouseEat");
    for (index = 0; index < NCATS; index++) {

        error = thread_fork("catlock thread",
                NULL,
                index,
                catlock,
                NULL
                );

        /*
         * panic() on error.
         */

        if (error) {

            panic("catlock: thread_fork failed: %s\n",
                    strerror(error)
                    );
        }
    }

    /*
     * Start NMICE mouselock() threads.
     */

    for (index = 0; index < NMICE; index++) {

        error = thread_fork("mouselock thread",
                NULL,
                index,
                mouselock,
                NULL
                );

        /*
         * panic() on error.
         */

        if (error) {

            panic("mouselock: thread_fork failed: %s\n",
                    strerror(error)
                    );
        }
    }
    lock_destroy(lock1);
    lock_destroy(Lock);
    lock_destroy(lock2);
    cv_destroy(catEat);
    cv_destroy(mouseEat);
    return 0;
}

/*
 * End of catlock.c
 */
