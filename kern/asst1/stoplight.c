/* 
 * stoplight.c
 *
 * 31-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: You can use any synchronization primitives available to solve
 * the stoplight problem in this file.
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
 * Number of cars created.
 */

#define NCARS 20
#define north 0
#define east 1
#define south 2
#define west 3

struct semaphore*NW, *NE, *SW, *SE, *totalcarnumber;

/*
 *
 * Function Definitions
 *
 */

static const char *directions[] = { "N", "E", "S", "W" };

static const char *msgs[] = {
        "approaching:",
        "region1:    ",
        "region2:    ",
        "region3:    ",
        "leaving:    "
};

/* use these constants for the first parameter of message */
enum { APPROACHING, REGION1, REGION2, REGION3, LEAVING };

static void
message(int msg_nr, int carnumber, int cardirection, int destdirection)
{
        kprintf("%s car = %2d, direction = %s, destination = %s\n",
                msgs[msg_nr], carnumber,
                directions[cardirection], directions[destdirection]);
}
 
/*
 * gostraight()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement passing straight through the
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
gostraight(unsigned long cardirection,
           unsigned long carnumber)
{
    /*
     * Avoid unused variable warnings.
     */
    
    (void) cardirection;
    (void) carnumber;
    
    
    //cardirection is north
    if (cardirection == north)
    {
        message(APPROACHING, carnumber, cardirection, south);
        
        
        P(NW);
        P(SW);        
        message(REGION1, carnumber, cardirection, south);
        message(REGION2, carnumber, cardirection, south);      
        message(LEAVING, carnumber, cardirection, south);
        V(NW);
        V(SW);
        
        
    }
    
    //cardirection is east
    if (cardirection == east)
    {
        message(APPROACHING, carnumber, cardirection, west);
        
        //lock NE AND NW way
        P(NE);
        P(NW);
        message(REGION1, carnumber, cardirection, west);
        message(REGION2, carnumber, cardirection, west);       
        message(LEAVING, carnumber, cardirection, west);
        //release nw and nw
        V(NE);
        V(NW);
        
       
    }
    
    //cardirection is south
    if (cardirection == south)
    {
        message(APPROACHING, carnumber, cardirection, north);
        
        
        // lock se and ne
        P(SE);
        P(NE);        
        message(REGION1, carnumber, cardirection, north);   
        message(REGION2, carnumber, cardirection, north);       
        message(LEAVING, carnumber, cardirection, north);
        //release se and ne
        V(SE);
        V(NE);
        
        
    }
    
    //cardirection is west
    
    if (cardirection == west)
    {
        message(APPROACHING, carnumber, cardirection, east);
       
        //lock SW AND SE 
        P(SW);
        P(SE);
        message(REGION1, carnumber, cardirection, east);    
        message(REGION2, carnumber, cardirection, east);      
        message(LEAVING, carnumber, cardirection, east);
        //release sw and se
        V(SW);
        V(SE);
        
       
    }
    
    
}


/*
 * turnleft()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement making a left turn through the 
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
turnleft(unsigned long cardirection,
         unsigned long carnumber)
{
        /*
         * Avoid unused variable warnings.
         */

        (void) cardirection;
        (void) carnumber;
        
        
        //cardirection is north
    if (cardirection == north)
    {
        message(APPROACHING, carnumber, cardirection, east);
        
        //lock nw sw se
        P(NW);
        P(SW);
        P(SE);
        
        message(REGION1, carnumber, cardirection, east);      
        message(REGION2, carnumber, cardirection, east);                        
        message(REGION3, carnumber, cardirection, east);       
        message(LEAVING, carnumber, cardirection, east);
        //release nw sw se
        V(NW);
        V(SW);
        V(SE);
       
    }
    
    //cardirection is east
    if (cardirection == east)
    {
        message(APPROACHING, carnumber, cardirection, south);
       
        //lock ne nw sw
        P(NE);
        P(NW);
        P(SW);
        message(REGION1, carnumber, cardirection, south);      
        message(REGION2, carnumber, cardirection, south);                              
        message(REGION3, carnumber, cardirection, south);      
        message(LEAVING, carnumber, cardirection, south);
        
        //release ne nw sw
        V(NE);
        V(NW);
        V(SW);
        
        
    }
    
    //cardirection is south
    if (cardirection == south)
    {
        message(APPROACHING, carnumber, cardirection, west);
       
        //lock se ne nw
        P(SE);
        P(NE);
        P(NW);
        message(REGION1, carnumber, cardirection, west);      
        message(REGION2, carnumber, cardirection, west);      
        message(REGION3, carnumber, cardirection, west);       
        message(LEAVING, carnumber, cardirection, west);
        
        //release se ne nw
        V(SE);
        V(NE);
        V(NW);
        
        
        
    }
    
    //cardirection is west
    
    if (cardirection == west)
    {
        message(APPROACHING, carnumber, cardirection, north);
      
        //lock sw se ne
        P(SW); 
        P(SE);
        P(NE);
        message(REGION1, carnumber, cardirection, north);      
        message(REGION2, carnumber, cardirection, north);
        message(REGION3, carnumber, cardirection, north);       
        message(LEAVING, carnumber, cardirection, north);
        //release sw se ne
        V(SW);
        V(SE);
        V(NE);
        
        
        
    }
}


/*
 * turnright()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement making a right turn through the 
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
turnright(unsigned long cardirection,
          unsigned long carnumber)
{
        /*
         * Avoid unused variable warnings.
         */

        (void) cardirection;
        (void) carnumber;
        
    //cardirection is north
    if (cardirection == north)
    {
        message(APPROACHING, carnumber, cardirection, west);
        
        
        //lock nw
        P(NW);       
        message(REGION1, carnumber, cardirection, west);                     
        message(LEAVING, carnumber, cardirection, west);
        //release nw
        V(NW);
        
       
    }
    
    //cardirection is east
    if (cardirection == east)
    {
        message(APPROACHING, carnumber, cardirection, north);
        
        //lock ne
        P(NE);       
        message(REGION1, carnumber, cardirection, north);     
        message(LEAVING, carnumber, cardirection, north);
        //release ne
        V(NE);
        
       
    }
    
    //cardirection is south
    if (cardirection == south)
    {
        message(APPROACHING, carnumber, cardirection, east);
        
        //lock se
        P(SE);       
        message(REGION1, carnumber, cardirection, east);     
        message(LEAVING, carnumber, cardirection, east);
        //release se
        V(SE);
        
        
    }
    
    //cardirection is west
    
    if (cardirection == west)
    {
        message(APPROACHING, carnumber, cardirection, south);
       
        //lock sw
        P(SW);      
        message(REGION1, carnumber, cardirection, south);     
        message(LEAVING, carnumber, cardirection, south);
        //release sw
        V(SW);
        
        
    }
}


/*
 * approachintersection()
 *
 * Arguments: 
 *      void * unusedpointer: currently unused.
 *      unsigned long carnumber: holds car id number.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Change this function as necessary to implement your solution. These
 *      threads are created by createcars().  Each one must choose a direction
 *      randomly, approach the intersection, choose a turn randomly, and then
 *      complete that turn.  The code to choose a direction randomly is
 *      provided, the rest is left to you to implement.  Making a turn
 *      or going straight should be done by calling one of the functions
 *      above.
 */
 
static
void
approachintersection(void * unusedpointer,
                     unsigned long carnumber)
{
        int cardirection;
        int direction_change;

        /*
         * Avoid unused variable and function warnings.
         */

        (void) unusedpointer;
        (void) carnumber;
	(void) gostraight;
	(void) turnleft;
	(void) turnright;

        /*
         * cardirection is set randomly.
         * direction_change is set random,as it only have three choice: 1. go left, 2. go right, 3. go straight
         */

        
        cardirection = random() % 4;
        
        //random choose turn direction
        direction_change = random() % 3;
        
       
        // control only max two car can get into the  intersection
        P(totalcarnumber);
        //call the three function to finish the direction change
        //0 is go straight,1 is turn left, 2 is turn right
        if (direction_change == 0)
            gostraight(cardirection,carnumber);
        if (direction_change == 1)
            turnleft(cardirection,carnumber);
        if (direction_change == 2)
            turnright(cardirection, carnumber);
        
       
        V(totalcarnumber);
        
}



/*
 * createcars()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up the approachintersection() threads.  You are
 *      free to modiy this code as necessary for your solution.
 */

int
createcars(int nargs,
           char ** args)
{
        int index, error;

        /*
         * Avoid unused variable warnings.
         */

        (void) nargs;
        (void) args;
        
        //create the direction for each lock     
        
        //create sem for nw ne sw and se way
        NW = sem_create("NW",1);
        NE = sem_create("NE",1);
        SW = sem_create("SW",1);
        SE = sem_create("SE",1);
        totalcarnumber = sem_create("totalcarnumber",2);
        
        /*
         * Start NCARS approachintersection() threads.
         */

        for (index = 0; index < NCARS; index++) {

                error = thread_fork("approachintersection thread",
                                    NULL,
                                    index,
                                    approachintersection,
                                    NULL
                                    );

                /*
                 * panic() on error.
                 */

                if (error) {
                        
                        panic("approachintersection: thread_fork failed: %s\n",
                              strerror(error)
                              );
                }
        }
        
              
          
            
    
       
        
        
        
        return 0;
}
