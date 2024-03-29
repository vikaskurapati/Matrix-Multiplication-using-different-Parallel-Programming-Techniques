//HW3
//130010054
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "cuda_runtime.h"
#define TILE_WIDTH 4
__global__ void
MatrixMulSh( float *Md , float *Nd , float *Pd , const int WIDTH )
{
  __shared__ float Mds [TILE_WIDTH][TILE_WIDTH] ;
  __shared__ float Nds [TILE_WIDTH][TILE_WIDTH] ;
  unsigned int col = TILE_WIDTH*blockIdx.x + threadIdx.x ;
  unsigned int row = TILE_WIDTH*blockIdx.y + threadIdx.y ;
  for (int m = 0 ; m<WIDTH/TILE_WIDTH ; m++ )
  {
    Mds[threadIdx.y][threadIdx.x] =  Md[row*WIDTH + (m*TILE_WIDTH + threadIdx.x)]  ;
    Nds[threadIdx.y][threadIdx.x] =  Nd[ ( m*TILE_WIDTH + threadIdx.y) * WIDTH + col] ;
    __syncthreads();
    for ( int k = 0; k<TILE_WIDTH ; k++ )
      Pd[row*WIDTH + col]+= Mds[threadIdx.x][k] * Nds[k][threadIdx.y] ;
    __syncthreads();
  }
}
int main ()
{
  const int WIDTH = 10000;
  clock_t  start,end;
  double elapsed;
  float A[WIDTH][WIDTH] ,B[WIDTH][WIDTH],C1[WIDTH][WIDTH] ,C[WIDTH][WIDTH]  ;
  float *A_d , *B_d ,*C1_d,*C_d ; // device array
  int i , j ;
  for ( i = 0 ; i<WIDTH ; i++ )
  {
    for (j = 0 ; j<WIDTH ; j++ )
    {
      A[i][j] = i+j+2 ;
      B[i][j] = (i+1)*(j+1);
    }
  }
  start = clock();
  cudaMalloc((void **) &A_d , WIDTH*WIDTH*sizeof (int) ) ;
  cudaMalloc((void **) &B_d , WIDTH*WIDTH*sizeof (int) ) ;
  cudaMemcpy ( A_d , A , WIDTH*WIDTH*sizeof (int) , cudaMemcpyHostToDevice ) ;
  cudaMemcpy ( B_d , B , WIDTH*WIDTH*sizeof (int) , cudaMemcpyHostToDevice ) ;
  cudaMalloc((void **) &C1_d , WIDTH*WIDTH*sizeof (int) ) ;
  cudaMalloc((void **) &C_d , WIDTH*WIDTH*sizeof (int) ) ;
  dim3 dimGrid ( WIDTH/TILE_WIDTH , WIDTH/TILE_WIDTH ,1 ) ;
  dim3 dimBlock( TILE_WIDTH, TILE_WIDTH, 1 ) ;
  MatrixMulSh<<<dimGrid,dimBlock>>> ( A_d , B_d ,C_d , WIDTH) ;
  cudaMemcpy(C , C_d , WIDTH*WIDTH*sizeof(int) ,cudaMemcpyDeviceToHost) ;
  end = clock();
  elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
  printf("\n Time taken is %f\n",elapsed);
  system("pause") ;
}
