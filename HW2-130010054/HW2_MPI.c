#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#define MASTER 0               
#define N 10000
#define FROM_MASTER 1          
#define FROM_WORKER 2          

int main (int argc, char *argv[])
{
int	numtasks,taskid,numworkers,source,dest,mtype,rows,averow,extra,offset,i, j, k, rc;           
double	a[N][N],b[N][N],c[N][N];           
double start,end;
MPI_Status status;
MPI_Init(&argc,&argv);
MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
start = MPI_Wtime();
numworkers = numtasks-1;
   if (taskid == MASTER)
   {
      for (i=0; i<N; i++)
         for (j=0; j<N; j++)
            a[i][j]= i+j+2;
      for (i=0; i<N; i++)
         for (j=0; j<N; j++)
            b[i][j]= (i+1)*(j+1);
      averow = N/numworkers;
      extra = N%numworkers;
      offset = 0;
      mtype = FROM_MASTER;
      for (dest=1; dest<numworkers+1; dest++)
      {
         if (dest<=extra)
           	{
            	/* code */
            	rows = averow+1;
           	}   	
         else 
         	{
         		rows = averow;
         	}
         MPI_Send(&offset, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
         MPI_Send(&rows, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
         MPI_Send(&a[offset][0], rows*N, MPI_DOUBLE, dest, mtype,MPI_COMM_WORLD);
         MPI_Send(&b, N*N, MPI_DOUBLE, dest, mtype, MPI_COMM_WORLD);
         offset = offset + rows;
      }
      mtype = FROM_WORKER;
      for (i=1; i<numworkers+1; i++)
      {
         source = i;
         MPI_Recv(&offset, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
         MPI_Recv(&rows, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
         MPI_Recv(&c[offset][0], rows*N, MPI_DOUBLE, source, mtype, MPI_COMM_WORLD, &status);
      }
   }
   if (taskid > MASTER)
   {
      mtype = FROM_MASTER;
      MPI_Recv(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
      MPI_Recv(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
      MPI_Recv(&a, rows*N, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);
      MPI_Recv(&b, N*N, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);

      for (k=0; k<N; k++)
         for (i=0; i<rows; i++)
         {
            c[i][k] = 0.0;
            for (j=0; j<N; j++)
               c[i][k] = c[i][k] + a[i][j] * b[j][k];
         }
      mtype = FROM_WORKER;
      MPI_Send(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
      MPI_Send(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
      MPI_Send(&c, rows*N, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);
   }
   end = MPI_Wtime();
   printf("\n Time taken by process %d for %d rows and columns is %f\n",taskid,N,(end-start));
   MPI_Finalize();
}