#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <string.h>
#include<conio.h>
#include<stdlib.h>
#include<omp.h>

#pragma once
#pragma warning(disable : 4035)

inline unsigned __int64 GetCycleCount(void)
{
	_asm    _emit 0x0F
	_asm    _emit 0x31
}

class MyTimer
{
	unsigned __int64  m_startcycle;

public:

	unsigned __int64  m_overhead;

	MyTimer(void)
	{
		m_overhead = 0;
		Start();
		m_overhead = Stop();
	}

	void Start(void)
	{
		m_startcycle = GetCycleCount();
	}

	unsigned __int64 Stop(void)
	{
		return GetCycleCount() - m_startcycle - m_overhead;
	}
};


int *row;
double *data;
int *col;
int *ptr;
double *res;
int *b;



void csr(int n, int nonz)
{
	FILE *f1;
	fopen_s(&f1, "CSRsample.txt", "w");
	int k;
	for (k = 0; k <= n; k++)
	{
		*(ptr + k) = 0;
	}

	int i, j = 0;
	k = 0;
	for (i = 1; i <= n; i++)
	{
		*(ptr + i) = *(ptr + i - 1);
		for (j = k; j <= nonz; j++)
		{
			if (*(row + j) == (i - 1))
			{
				*(ptr + i) = *(ptr + i) + 1;
				//printf("%d ", *(ptr + i));
				k++;
			}
			else
				break;
		}
	}
	fprintf(f1, "%d %d %d\n", n, n, nonz);
	for (i = 0; i<nonz; i++)
	{
		fprintf(f1, "%d ", *(col + i));
		fprintf(f1, "%lf\n", *(data + i));
	}
	for (i = 0; i <= n; i++)
	{
		fprintf(f1, "%d ", *(ptr + i));
		//printf("%d",*(ptr+i));
	}

	fclose(f1);

}

void sort(int nz)
{
	int i, j, temp, posn;
	double t;
	for (i = 0; i<nz - 1; i++)
	{
		posn = i;
		for (j = i + 1; j<nz; j++)
		{
			if (*(row + posn)>*(row + j))
				posn = j;
		}
		if (posn != i)
		{
			temp = *(row + i);
			*(row + i) = *(row + posn);
			*(row + posn) = temp;

			temp = *(col + i);
			*(col + i) = *(col + posn);
			*(col + posn) = temp;

			t = *(data + i);
			*(data + i) = *(data + posn);
			*(data + posn) = t;
		}
	}
	printf("Sorting completed");
}

void spmv(int nz, int nr)
{
	int i, j, k;

	MyTimer timer;

	timer.Start();                // take initial reading
	Sleep(1000);                // wait a second 
	unsigned cpuspeed10 = (unsigned)(timer.Stop() / 100000);

	b = (int *)malloc(nr*sizeof(int));
	res = (double *)malloc(nr*sizeof(double));
	FILE *f1;
	fopen_s(&f1, "SpmvCSR.txt", "w");
	for (i = 0; i<nr; i++)
	{
		*(b + i) = 1;
	}

	#pragma omp parallel num_threads(4)
	timer.Start();
	#pragma omp	parallel for private (i)

	for (i = 0; i < nr; i++)
	{
		*(res + i) = 0;
		for (k = *(ptr + i); k < *(ptr + i + 1); k++)
		{
			j = *(col + k);
			*(res + i) = *(res + i) + (*(data + k) * (*(b + j)));
		}
		//printf("%lf\n", *(res + i));
	}

	unsigned cycle = (unsigned)timer.Stop();
	unsigned speed = (unsigned)(cycle / 100000);

	printf("\n\nTime = %d\n", cycle * 10000 / cpuspeed10);

	for (i = 0; i<nr; i++)
		fprintf(f1, "%lf\n ", *(res + i));
}

int main()
{
	FILE *fp;
	char line[1024];
	int i, j;
	int n = 0; // number of nonzero elements in data
	int nr = 0; // number of rows in matrix
	int nc = 0; // number of columns in matrix

	fopen_s(&fp, "sample.txt", "r");

	fgets(line, 128, fp);
	while (line[0] == '%')
	{
		fgets(line, 128, fp);
	}
	sscanf_s(line, "%d %d %d\n", &nr, &nc, &n);
	printf("%d, %d, %d\n", nr, nc, n);
	ptr = (int *)malloc((nr + 1)*sizeof(int));
	row = (int *)malloc(n*sizeof(int));
	col = (int *)malloc(n*sizeof(int));
	data = (double *)malloc(n*sizeof(double));
	for (i = 0; i<n; i++)
	{
		fscanf_s(fp, "%d %d %lf\n", &(row[i]), &(col[i]), &(data[i]));
	}
	sort(n);
	csr(nc, n);
	spmv(n, nr);
	fclose(fp);

	system("pause");
	return 0;
}