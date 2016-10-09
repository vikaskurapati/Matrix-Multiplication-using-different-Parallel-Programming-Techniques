/*
*******************************************************************
* Laplace MPI Version
*
* T is initially 0.0
* Boundaries are as follows
*
*             T = 0                                        Y
*     |-------------------| 0      |-------------------|   |
*     |                   |        |                   |   |
*  T  |                   |        |-------------------|   |
*  =  |                   |  T     |                   |   |
*  0  |                   |        |-------------------|   |_______X
*     |                   |        |                   |
*     |                   |        |-------------------|
*     |                   |        |                   |
*     |-------------------| 100    |-------------------|
*     0                 100
*
* Use Central Differencing Method
* Each process only has a local subgrid.
* Each Processor works on a sub grid and then sends its
* boundaries to neighbours
*
* Modified by Shiva Gopalakrishnan, NPS ..2010
* Modified by John Urbanic,  PSC.... 2008;
* From the original Fortran version by Sushell Chitre, R Reddy, PSC 1994
*
******************************************************************   */

#define NPES     4
#define NC       1000                   /* Number of Cols        */
#define NR       1000                   /* Number of Rows        */
#define NRL      (NR/NPES)              /* Number of Rows per PE */
#define NITER    1000                   /* Max num of Iterations */
#define DOWN     100                    /* Tag for messages down */
#define UP       101                    /* Tag for messages up   */
#define ROOT     0                      /* The root PE           */
#define MAX(x,y) ( ((x) > (y)) ? x : y )

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

void initialize( float t[NRL+2][NC+2] );
void set_bcs   ( float t[NRL+2][NC+2], int mype, int npes );
void print_trace( float t[NRL+2][NC+2], int mype, int npes, int iter );

int main( int argc, char **argv ){
  
  int        npes;                      /* Number of PEs */
  int        mype;                      /* My PE number  */
  int        stat;                      /* Error Status  */
  int        niter;                     /* iter counter  */
  MPI_Status status;

  float      t[NRL+2][NC+2];            /*temperature */
  float      told[NRL+2][NC+2];         /* previous temperature*/
  float      dt;                        /* Delta t       */
  float      dtg;                       /* Delta t global*/
  double     start,end;                /* timing */
  int        i, j, iter;


  MPI_Init(&argc, &argv);

  MPI_Comm_size(MPI_COMM_WORLD, &npes );
  MPI_Comm_rank(MPI_COMM_WORLD, &mype );
  start=MPI_Wtime();  


  if ( npes != NPES ){                  /* Currently hardcoded */
    if( mype == 0 )
      fprintf(stdout, "The example is only for %d PEs\n", NPES);
    MPI_Finalize();
    exit(1);
  }

  initialize(t);                  /* Set initial guess to 0 */

  set_bcs(t, mype, npes);         /* Set the Boundary Conditions */

  for( i=0; i<=NRL+1; i++ )       /* Copy the values into told */
    for( j=0; j<=NC+1; j++ )
      told[i][j] = t[i][j];

  if( mype == ROOT ){
    printf("How many iterations [100-1000]?\n");
    scanf("%d", &niter);
    if( niter>NITER ) niter = NITER;
  }

  MPI_Bcast(&niter, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

/*----------------------------------------------------------*
 |       Do Computation on Sub-grid for Niter iterations    |
 *----------------------------------------------------------*/

  for( iter=1; iter<=niter; iter++ ) {

    for( i=1; i<=NRL; i++ )
      for( j=1; j<=NC; j++ )
        t[i][j] = 0.25 * ( told[i+1][j] + told[i-1][j] +
                           told[i][j+1] + told[i][j-1] );

    if( mype < npes-1 )    /*   Sending Down; Only npes-1 do this  */

      MPI_Send( &t[NRL][1],  NC, MPI_FLOAT, mype+1, DOWN, MPI_COMM_WORLD);

    if( mype != 0 )        /*   Sending Up  ; Only npes-1 do this  */
      MPI_Send( &t[1][1],    NC, MPI_FLOAT, mype-1, UP,   MPI_COMM_WORLD);

    if( mype != 0 )        /*   Receive from UP */
      MPI_Recv( &t[0][1], NC, MPI_FLOAT, MPI_ANY_SOURCE, DOWN,
                             MPI_COMM_WORLD, &status);

    if( mype != npes-1 )        /*   Receive from DOWN */
      MPI_Recv( &t[NRL+1][1],NC, MPI_FLOAT, MPI_ANY_SOURCE, UP  ,
                                MPI_COMM_WORLD, &status);

    // dt = 0.;

    for( i=0; i<=NRL+1; i++ )       /* Copy for next iteration  */
      for( j=1; j<=NC; j++ ){
	//	dt         = MAX( fabs(t[i][j]-told[i][j]), dt);
        told[i][j] = t[i][j];
      }

    //    MPI_Reduce(&dt, &dtg, 1, MPI_FLOAT, MPI_MAX, ROOT, MPI_COMM_WORLD);

/*   Periodically print some test Values   */

    if( (iter%100) == 0 ) {
      print_trace( t, mype, npes, iter );
    }

    MPI_Barrier( MPI_COMM_WORLD );

  }  /* End of kernal */


  end=MPI_Wtime();

  printf("\n Time taken by process %d is %f \n",mype,(end - start));

  MPI_Finalize();

}    /* End of Program   */



/********************************************************************
 *								    *
 * Initialize all the values to 0. as a starting value              *
 *								    *
 ********************************************************************/

void initialize( float t[NRL+2][NC+2] ){

  int        i, j, iter;
	  
  for( i=0; i<=NRL+1; i++ )
    for ( j=0; j<=NC+1; j++ )
      t[i][j] = 0.0;
}



/********************************************************************
 *								    *
 * Set the values at the boundary.  Values at the boundary do not   *
 * change throughout the execution of the program		    *
 *								    *
 ********************************************************************/

void set_bcs( float t[NRL+2][NC+2], int mype, int npes ){

  int i, j;
  float tmin, tmax;

  tmin = (mype  ) * 100.0/npes;
  tmax = (mype+1) * 100.0/npes;

  for( i=0; i<=NRL+1; i++ ) {      /* Set Left and Right bndry */
    t[i][0]       = 0.0;
    t[i][NC+1]    = tmin + ((tmax-tmin)/NRL) * i;
  }

  if( mype == 0 )                     /* Top boundary */
    for( j=0; j<=NC+1; j++ )
      t[0][j]        = 0.0;

  if( mype == npes-1 )               /* Bottom boundary */
    for( j=0; j<=NC+1; j++ )
      t[NRL+1][j] = (100.0/NC) * j;

}



/********************************************************************
 *								    *
 * Print the trace only in the last PE where most action is         *
 *								    *
 ********************************************************************/

void print_trace( float t[NRL+2][NC+2], int mype, int npes, int iter ){

  int joff, i;

  if( mype==npes-1 ){
    printf("\n---------- Iteration number: %d ------------\n", iter);

    joff = mype*NRL;
    for(i=NRL-10; i<=NRL; i++){
      printf("%15.8f", t[i][joff+i]);
    }
  }
    MPI_Barrier( MPI_COMM_WORLD );
  return;
}
