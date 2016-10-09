/*
*******************************************************************
* Laplace Serial Version
*
* T is initerially 0.0
* Boundaries are as follows

*             T = 0              
*     |-------------------| 0    
*     |                   |      
*  T  |                   |      
*  =  |                   |  T   
*  0  |                   |      
*     |                   |      
*     |                   |      
*     |                   |      
*     |-------------------| 100  
*     0                 100
*
* Use Central Differencing Method
*
*Modified by Shiva Gopalakkrishnan, NPS...2010
*Modified by  John Urbanic,  PSC.... 2008;
* From the original Fortran version by Sushell Chitre, R Reddy, PSC 1994
*
******************************************************************   */

#define NC       510                   /* Number of Cols        */
#define NR       510                   /* Number of Rows        */
#define NITER    1000                   /* Max num of Iterations */
#define MAX(x,y) ( ((x) > (y)) ? x : y )

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h> /* only for timing */
#include "cuda_runtime.h"

void initialize( float t[NR+2][NC+2] );
void set_bcs   ( float t[NR+2][NC+2]);
void print_trace( float t[NR+2][NC+2], int iter);

__global__ void compute( float *t, float *told);
__global__ void update( float *t, float *told);

int main( int argc, char **argv ){
  
  int        niter;                     /* iter counter  */

  float      t[NR+2][NC+2];            /*temperature */
  float      told[NR+2][NC+2];         /* previous temperature*/
  float      dt;                        /* Delta t       */
  clock_t    start, end;              /* timiing */
  int        i, j, iter;
  double  elapsed;

  float *dev_t, *dev_told; //device pointers
  int size = (NR+2)*(NC+2)*sizeof(float); 

    start = clock();
  cudaMalloc((void **)&dev_t,size);
  cudaMalloc((void **)&dev_told,size);


    initialize(t);                  /* Set initial guess to 0 */

  set_bcs(t);         /* Set the Boundary Conditions */

  for( i=0; i<=NR+1; i++ )       /* Copy the values into told */
    for( j=0; j<=NC+1; j++ )
      told[i][j] = t[i][j];

  //   printf("How many iterations [100-1000]?\n");
  // scanf("%d", &niter);
  niter=1000;
   if( niter>NITER ) niter = NITER;
 
 
/*----------------------------------------------------------*
 |       Do Computation on grid for Niter iterations    |
 *----------------------------------------------------------*/
 cudaMemcpy(dev_told,&told,size,cudaMemcpyHostToDevice);

  for( iter=1; iter<=niter; iter++ ) {
    
   

    compute<<<512,512 >>>(dev_t,dev_told);

    update<<<512,512 >>>(dev_t,dev_told);



/*   Periodically print some test Values   */
    if( (iter%100) == 0 ) {
    cudaMemcpy(&t,dev_t,size,cudaMemcpyDeviceToHost);
      print_trace( t, iter );
    }

  }  /* End of kernal */
  end = clock();
  elapsed = ((double) (end - start)) / CLOCKS_PER_SEC;   
   printf(" \n Time taken is %f \n",elapsed);

}    /* End of Program   */


__global__ void compute(float *t, float *told)

{
  //Write CUDA kernal here
  int thread = threadIdx.x;
  int block = blockIdx.x;
  int dimen = blockDim.x;

  int i = thread + block*dimen;
  int j1 = thread + (block -1)*dimen;
  int j2 = thread + (block+1)*dimen;

  if((block != 0) && (block != 511))
    {
      if((thread !=0) && (thread !=511))
	{
	  t[i]=0.25*(told[i-1]+told[i+1]+told[j1]+told[j2]);
	}
      else
	{
      t[i]=told[i];
	}
    }
  else
    {
      t[i]=told[i];
    }

}

__global__ void update(float *t, float *told)

{
  //Write CUDA kernal here
  int thread = threadIdx.x;
  int block = blockIdx.x;
  int dimen = blockDim.x;

  int i = thread + block*dimen;
  told[i]=t[i];
  

}

/*******************************************************************
 *								    *
 * Initialize all the values to 0. as a starting value              *
 *								    *
 ********************************************************************/

void initialize( float t[NR+2][NC+2] ){

  int        i, j, iter;
	  
  for( i=0; i<=NR+1; i++ )
    for ( j=0; j<=NC+1; j++ )
      t[i][j] = 0.0;
}



/********************************************************************
 *								    *
 * Set the values at the boundary.  Values at the boundary do not   *
 * change throughout the execution of the program		    *
 *								    *
 ********************************************************************/

void set_bcs( float t[NR+2][NC+2]){

  int i, j;

  for( i=0; i<=NR+1; i++){
    t[i][0] = 0.0;
    t[i][NC+1] = (100.0/NR)*i;
  }

  for( j=0; j<=NC+1; j++){
    t[0][j] = 0.0;
    t[NR+1][j] = (100.0/NR)*j;
  }

}



/********************************************************************
 *								    *
 * Print the trace only in the bottom corner where most action is   *
 *								    *
 ********************************************************************/

void print_trace( float t[NR+2][NC+2], int iter ){

  int joff, i;
  joff=0;
    printf("\n---------- Iteration number: %d ------------\n", iter);

    for(i=NR-10; i<=NR; i++){
      printf("%15.8f", t[i][joff+i]);
    }

  return;
}
