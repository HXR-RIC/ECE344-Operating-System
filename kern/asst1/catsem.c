/*
 * catsem.c
 *
 * 30-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: Please use SEMAPHORES to solve the cat syncronization problem in 
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


/*
 * 
 * Function Definitions
 * 
 */
struct semophore *mouse;
struct semophore *cat;
struct semophore *lock;
struct semophore *bowl_lock;
struct semophore *bowl_control;
int num_bowl_available = 2;
int bowlA[] = {0, 0};

/* who should be "cat" or "mouse" */
static void
sem_eat(const char *who, int num, int bowl, int iteration) {
    kprintf("%s: %d starts eating: bowl %d, iteration %d\n", who, num,
            bowl, iteration);
    clocksleep(1);
    kprintf("%s: %d ends eating: bowl %d, iteration %d\n", who, num,
            bowl, iteration);
}

/*
 * catsem()
 
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long catnumber: holds the cat identifier from 0 to NCATS - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using semaphores.
 *
 */
// if the mice is eating, let catsem in wait queue. If the cat is eating, let mousesem in waiting queue.
// if the # of bowl is equal to 2, then let other cat wait for the bowl if it is available 

static
void
catsem(void * unusedpointer,
        unsigned long catnumber) {
    int iteration;
    (void) unusedpointer;
    const char *who = "cat";
    for (iteration = 0; iteration < 4; iteration++) {
        P(lock);//block the thread except the first coming one
        //num_bowl_available==2 means cat or mouse thread can come in 
        if (num_bowl_available == 2) {//if the cat thread is coming first, it will block the mouse thread by decreasing mouse count to 0
            P(mouse);
            P(mouse);
            //if cat thread let the lock semaphore open, assume mouse thread will come next, it will not check the bowl count, 
            //but check P(mouse) and find the semaphore value=0, then block the mouse thread
        }
        P(cat); //once the cat arrive, it will decreasing by 1 and thread continue and it also block the third cat come in
        //we need to let num_bowl count to decrease before call V() since we need to make sure no other mouse can come in
        //at this point, the cat is choosing the bowl 		
        P(bowl_control);//bowl_control is 2 since we only allow at most 2 same threads have meals at same time
        num_bowl_available--;//bowl count=2 -> 1 -> 0
        V(lock);//increment the lock count, and another count can come in
        P(bowl_lock);//we need to determine which bowl is available for the thread, therefore need to control it to change the value for our available bowl
        if (bowlA[0] == 0) {//this means the bowl 1 is available, the cat will eat the food in bowl1
            bowlA[0] = 1;//bowl 1 is not available for others since one cat is eating now
            //num_bowl_available--;
            V(bowl_lock);
            sem_eat(who, catnumber, 1, iteration);//in sem_cat, we assume the cat eat 1s per iteration, and will give the bowl to another available thread
            bowlA[0] = 0;//therefore, we need to set the bowl1 to be available for others
            num_bowl_available++;
            V(cat);//since one cat is leaving, we need to increment the cat available by 1
            V(bowl_control);
        } else if (bowlA[1] == 0) {//if the bowl1 is occupied by another cat, see bowl2 is available or not
            bowlA[1] = 1;
            //num_bowl_available--;
            V(bowl_lock);
            sem_eat(who, catnumber, 2, iteration);
            bowlA[1] = 0;         
            num_bowl_available++;
            V(cat);
            V(bowl_control);
        }
        //if the bowl is available=2, we need to call wake up all threads
        P(lock);
        if (num_bowl_available == 2) {
            V(mouse);
            V(mouse);
        }
        //V(cat);
        V(lock);
    }
}

/*
 * mousesem()
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
 *      Write and comment this function using semaphores.
 *
 */

static
void
mousesem(void * unusedpointer,
        unsigned long mousenumber) {
    (void) unusedpointer;
    int iteration;
    const char *who = "mouse";
    for (iteration = 0; iteration < 4; iteration++) {
        P(lock);
        if (num_bowl_available == 2) {//let the cat come in, therefore we need to make the mouse wait in the queue
            P(cat);
            P(cat);
        }
        P(mouse); //once the cat arrive, it will drecrement by 1 and thread continue		     
        P(bowl_control);
        num_bowl_available--;
        //For this V(lock), only the mouse can come in since the cat is all blocking 
        V(lock);//wake up the threads which call P(lock)
        P(bowl_lock);
        if (bowlA[0] == 0) {//this means the bowl 1 is avilable, the cat will eat the food in bowl1
            bowlA[1] = 1;
            //num_bowl_available--;
            V(bowl_lock);
            sem_eat(who, mousenumber, 2, iteration);
            bowlA[1] = 0;
            num_bowl_available++;
            V(mouse);        
            V(bowl_control);
        } else if (bowlA[1] == 0) {//if the bowl1 is occupied by another cat, see bowl2 is avilable or not
            bowlA[1] = 1;
            //num_bowl_available--;
            V(bowl_lock);
            sem_eat(who, mousenumber, 2, iteration);
            bowlA[1] = 0; 
            num_bowl_available++;
            V(mouse);          
            V(bowl_control);
        }
        P(lock);
        if (num_bowl_available == 2) {
            V(cat);
            V(cat);
        }
        //V(mouse);
        V(lock);
    }
}

/*
 * catmousesem()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up catsem() and mousesem() threads.  Change this 
 *      code as necessary for your solution.
 */

int
catmousesem(int nargs,
        char ** args) {
    int index, error;

    /*
     * Avoid unused variable warnings.
     */

    (void) nargs;
    (void) args;

    /*
     * Start NCATS catsem() threads.
     */
    mouse = sem_create("mouse", 2);
    cat = sem_create("cat", 2);
    lock = sem_create("lock", 1);
    bowl_control = sem_create("bowl_control", 2);
    bowl_lock = sem_create("bowl_lock", 1);
    for (index = 0; index < NCATS; index++) {

        error = thread_fork("catsem Thread",
                NULL,
                index,
                catsem,
                NULL
                );

        /*
         * panic() on error.
         */

        if (error) {

            panic("catsem: thread_fork failed: %s\n",
                    strerror(error)
                    );
        }
    }

    /*
     * Start NMICE mousesem() threads.
     */

    for (index = 0; index < NMICE; index++) {

        error = thread_fork("mousesem Thread",
                NULL,
                index,
                mousesem,
                NULL
                );

        /*
         * panic() on error.
         */

        if (error) {

            panic("mousesem: thread_fork failed: %s\n",
                    strerror(error)
                    );
        }
    }
    return 0;
}


/*
 * End of catsem.c
 */
