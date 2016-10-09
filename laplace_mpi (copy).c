#define NPES     4
#define N       100                   /* Number of Rows        */
#define NL      (NR/NPES)              /* Number of Rows per PE */
#define DOWN     100                    /* Tag for messages down */
#define UP       101                    /* Tag for messages up   */
#define ROOT     0                      /* The root PE           */
#define MAX(x,y) ( ((x) > (y)) ? x : y )

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <time.h>

int main( int argc, char **argv ){
  
  int        npes;                      /* Number of PEs */
  int        mype;                      /* My PE number  */
  int        stat;                      /* Error Status  */
  MPI_Status status;

  float      a[N][N],b[N][N],c[N][N];            
  double     start,end;                /* timing */
  int        i, j, k;
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
  if( mype == ROOT )
  {
    initialize(a,b,c);                  /* Set initial values*/
  }

  MPI_Bcast(&N, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
  for( i=0; i<N; i++ ) 
{

    for( j=0; j<N; j++ )
      {
      	for( k =0; k < N; k++ )
        {c[i][j]+= a[i][k] * b[k][j];
		}}
}
    if( mype < npes-1 )    /*   Sending Down; Only npes-1 do this  */

      MPI_Send( &t[N-2][1],  N , MPI_FLOAT, mype+1, DOWN, MPI_COMM_WORLD);

    if( mype != 0 )        /*   Sending Up  ; Only npes-1 do this  */
      MPI_Send( &c[1][1],    N , MPI_FLOAT, mype-1, UP,   MPI_COMM_WORLD);

    if( mype != 0 )        /*   Receive from UP */
      MPI_Recv( &c[0][1], N , MPI_FLOAT, MPI_ANY_SOURCE, DOWN, MPI_COMM_WORLD, &status);

    if( mype != npes-1 )        /*   Receive from DOWN */
      MPI_Recv( &c[N-1][1], N , MPI_FLOAT, MPI_ANY_SOURCE, UP  , MPI_COMM_WORLD, &status);

    MPI_Barrier( MPI_COMM_WORLD );

  }  

  end=MPI_Wtime();

  printf("\n Time taken by process %d is %f \n",mype,(end - start));

  MPI_Finalize();

} 
void initialize( float a[N][N], float b[N][N], float c[N][N] )
{

  int        i, j, iter;
	srand(time(NULL));	  
  for( i=0; i<N; i++ )
  {
  	for ( j=0; j<=NC+1; j++ )
      {a[i][j] = rand() % 100 + 1;
  		b[i][j] = rand() % 100 + 1;
  		c[i][j] = rand() % 100 +1 ;

  }
    }
}