#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <omp.h>

# define N 1000

int main(int argc, char *argv[])
{
	int i,j,k;
  int n = N;
	srand(time(NULL));
	struct timeval  tv1, tv2;
	float **a, **b,**c;
  
  	a= (float**) malloc(N*sizeof(float*));

  	for(i=0;i<N;i++)
      a[i]=(float*)malloc(N*sizeof(float));

  	b= (float**) malloc(N*sizeof(float*));

  	for(i=0;i<N;i++)
      b[i]=(float*)malloc(N*sizeof(float));

  	c=(float**)calloc(N,sizeof(float*));

  	for(i=0;i<N;i++)
      c[i]=(float*)calloc(N,sizeof(float));
	
	gettimeofday(&tv1, NULL);

	
	for (i=0; i<N; i++)
    	for (j=0; j<N; j++)
    {
      a[i][j] = i+j;
      b[i][j] = i*j;
      //c[i][j] = 0;
    }

/*	#pragma omp parallel for private(j)
  	for (i=0;i<N;i++)
  	{
    	for(j=0;j<N;j++)
      		for (k =0; k<N; k++)
      			{
        			c[i][j] = c[i][j] + a[i][k] * b[k][j];
      			}
  	}*/
    

#pragma omp parallel shared(a,b,c,n) private(i,j,k)

{  	

#pragma omp for
for (i=0;i<n;i++)
  	{
    	for(j=0;j<n;j++)
      		c[i][j] = 0.0;
          for (k =0; k<n; k++)
      			{
        			c[i][j] = c[i][j] + a[i][k] * b[k][j];
      			}
  	}
}

    gettimeofday(&tv2, NULL);
    printf ("Total time for %d size  = %f seconds\n", N,
             (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
             (double) (tv2.tv_sec - tv1.tv_sec));
}