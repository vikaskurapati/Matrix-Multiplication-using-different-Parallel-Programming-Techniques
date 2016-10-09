#include <iostream>
#include <omp.h>

using namespace std;
int main()
{
	int N;
	cout<<"Give N:"<<endl;
	cin>>N;
	float A[N][N];
	float B[N][N];
	float C[N][N];
	int i,j,k;
	int n = N;
	long double rtime;
	cout<< "\n";
	rtime = omp_get_wtime();

	# pragma omp parallel shared(A,B,C,n) private(i,j,k)
	{
		# pragma omp for
		for (i = 0; i < n; i++)
		{
			/* code */
			for (j = 0; j < n; j++)
			{
				/* code */
				A[i][j] = i+j+2;
			}
		}
		# pragma omp for
		for (i = 0; i < n; i++)
		{
			/* code */
			for (j = 0; j < n; j++)
			{
				/* code */
				B[i][j] = (i+1)*(j+1);
			}
		}
		#pragma omp for
		for (i =0; i<n; i++)
		{
			for (j = 0; j < n; j++)
			{
				/* code */
				C[i][j] = 0.0;
				for (k = 0; k < n; k++)
				{
					/* code */
					C[i][j] = C[i][j] + A[i][k]*B[k][j];
				}
			}
		}
	}
	rtime = omp_get_wtime() - rtime;
	cout<<" Time taken for  "<<n<<" rows and columns is : " <<rtime<<"\n";
	return 0;
}
