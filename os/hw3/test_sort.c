/******************************************************************************
 ** CS416 - Operating Systems Theory                                         **
 ** Homework 3                                                               **
 ** Guilherme Cox <cox@computer.org>                                         **
 ******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>        // printf(..)
#include <signal.h>       // signal(..)
#include <stdlib.h>       // atoi(..)
#include "third.h"
#include <unistd.h>       // usleep(..)
#include <sys/ucontext.h>
////////////////////////////////////////////////////////////////////////////////
// Defines
// Default list size (in terms of number of elements)
#define LISTSIZE     (32)

struct pthrarg
{
    int *num;
    int size;
    third_mutex_t ** mtx;
};

static int quitting = 0;

////////////////////////////////////////////////////////////////////////////////
// Function prototypes
void * fnsort( third_t * me, void *arg );
void * fncheck( third_t * me, void *arg );
void printList( int *p, int size );
int begin_app( void );
int end_app( void );
////////////////////////////////////////////////////////////////////////////////
int begin_app( void )
{
    return 0;
}

void *fnsort( third_t * me, void *arg )
{
    struct pthrarg *pargs;
    int *num, swap;
    third_mutex_t *mtx0, *mtx1;

    pargs = (struct pthrarg * )arg;
    num   = pargs->num;
    mtx0  = pargs->mtx[0];
    mtx1  = pargs->mtx[1];

    while( !quitting )
    {
        third_mutex_lock( me, mtx0 );
        if( third_mutex_trylock( me, mtx1 ) != 0 )
        {
            third_mutex_unlock( me, mtx0 );
            third_yield(me);
            continue;
        }
        else {
        }

        if( num[1] < num[0] )
        {
            swap   = num[0];
            num[0] = num[1];
            num[1] = swap;
        }

        third_mutex_unlock( me, mtx0 );
        third_mutex_unlock( me, mtx1 );

        third_yield(me);
    }

    third_exit(me);

    // I will never get here
    return 0;
}

void * fncheck( third_t * me, void *arg )
{
    struct pthrarg *pargs;
    int i, j = 0, size, check;
    third_mutex_t ** mtx;

    pargs = (struct pthrarg * )arg;
    mtx   = pargs->mtx;
    size  = pargs->size;

    while( !quitting )
    {
        printf( "." );
        if( (j+1) % 80 == 0 )
            printf( "\n" );

        //lock all threads
        for( i = 0; i < size; i++ )
            third_mutex_lock( me, mtx[i] );

        check = 1;
        for( i = 0; i < size-1 && check; i++ )
        {
            if( pargs->num[i] > pargs->num[i+1] )
                check = 0;
        }

        if( check )
            printf("\nQuitting...\n");
        quitting = check;

        //unlock all threads
        for( i = 0; i < size; i++ )
            third_mutex_unlock( me, mtx[i] );

        // j seconds
        j = j+1;
#ifndef MYTHREAD
//        sleep( j );
#endif
        third_yield(me);
    }

    third_exit(me);

    return 0;
}

void printList( int *p, int size )
{
    int i;
    for( i = 0 ; i < size; i++ )
    {
        printf( "%4d ", p[i] );

        if( ((i+1) % 10) == 0 )
            printf("\n");
    }
    printf("\n");
}

int main( int argc, char **argv )
{
    int i, *pList = 0, nListSize = LISTSIZE;
    third_t ** threads, * thrcheck;
    third_mutex_t ** mutexes;
    struct pthrarg *pthrargs, pthrargcheck;

    if( argc == 2 )
        nListSize = atoi( argv[1] );
    nListSize = nListSize > 0 ? nListSize : LISTSIZE;

    // Creating the List of numbers
    printf( "Number of elements: %d\n", nListSize );

    pList = (int *) malloc( sizeof( int ) * nListSize );
    for( i = 0; i < nListSize; i++ )
        pList[i] = random( ) % (nListSize<<1);   // random list
//        pList[i] = nListSize-i;   // decreasing list  (easier to debug)

    printf( "[BEFORE] The list is NOT sorted:\n" );
    printList( pList, nListSize );

    third_scheduler_t * sched = third_setup();

    threads  = (third_t **) malloc( sizeof(third_t *) * (nListSize-1) );
    mutexes  = (third_mutex_t **)malloc( sizeof(third_mutex_t *) * nListSize );
    pthrargs = (struct pthrarg *)malloc( sizeof(struct pthrarg) * (nListSize-1) );

    mutexes[0] = third_mutex_create();
    for( i = 0; i < nListSize-1; i++ )
    {
        mutexes[i+1] = third_mutex_create();

        pthrargs[i].num  = &pList[i];
        pthrargs[i].mtx  = mutexes + i;
        pthrargs[i].size = nListSize;

        threads[i] = third_create(sched, fnsort, &pthrargs[i]);
    }

    pthrargcheck.num  = pList;
    pthrargcheck.mtx  = mutexes;
    pthrargcheck.size = nListSize;

    thrcheck = third_create(sched, fncheck, &pthrargcheck);

    ///////////
    // Waiting the threads to complete the sorting
    //////////

    printf( "waiting...\n" );

    // begin scheduling with preemption
    third_begin(sched, true);

    printf( "[AFTER] The list is sorted:\n" );
    printList( pList, nListSize );

    // Cleaning
    free( pthrargs );
    free( mutexes );
    free( threads );
    free( pList );

    return 0;
}

int end_app( void )
{
    return 0;
}
// eof
