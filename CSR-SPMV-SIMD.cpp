#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <string.h>
#include<conio.h>
#include<stdlib.h>
#include<tmmintrin.h>

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


void spmv(int nz, int nr, int nc)
{
	int i, j, k, t;
	float *b;
	float *res;
	int x;
	float *c;
	__m128 data1, vect1, res1, temp, tempmr;
	float *tempd;
	
	MyTimer timer;

	timer.Start();                // take initial reading
	Sleep(1000);                // wait a second 
	unsigned cpuspeed10 = (unsigned)(timer.Stop() / 100000);

	b = (float *)malloc(nc*sizeof(float));
	res = (float *)malloc(nr*sizeof(float));
	tempd = (float *)malloc(4 * sizeof(float));
	c = (float *)malloc(sizeof(float));

	FILE *f1;
	fopen_s(&f1, "SpmvCSR.txt", "w");

	for (i = 0; i<nc; i++)
	{
		*(b + i) = 1;
	}

	for (i = 0; i<nr; i++)
		*(res + i) = 0;

	timer.Start();

	for (i = 0; i < nr; i++)
	{
		for (k = *(ptr + i); k < *(ptr + i + 1); k = k + 4)
		{
			x = *(ptr + i + 1) - *(ptr + i);
			if (x - k < 4)
				t = x;
			else
				t = 4;
			for (j = 0; j<t; j++)
				*(tempd + j) = *(data + k + j);
			while (j<4)
			{
				*(tempd + j) = 0;
				j++;
			}

			/*printf("tempd array:");
			for(j=0;j<4;j++)
			printf("%f", *(tempd+j));*/

			j = *(col + k);

			data1 = _mm_loadu_ps(tempd);
			vect1 = _mm_loadu_ps((b + j));

			temp = _mm_mul_ps(data1, vect1);

			res1 = _mm_hadd_ps(temp, temp);
			res1 = _mm_hadd_ps(res1, res1);
			_mm_storeu_ps(c, res1);
			*(res + i) += *c;

		}
		//printf("%f\n", *(res+i));

	}
	
	unsigned cycle = (unsigned)timer.Stop();
	unsigned speed = (unsigned)(cycle / 100000);

	printf("\n\nTime = %d\n", cycle * 10000 / cpuspeed10);


	for (i = 0; i<nr; i++)
		fprintf(f1, "%f\n ", *(res + i));
	fclose(f1);
}



int main()
{
	FILE *fp;
	char line[1024];
	int i, j;
	int n = 0; // number of nonzero elements in data
	int nr = 0; // number of rows in matrix
	int nc = 0; // number of columns in matrix

	fopen_s(&fp, "foldoc.txt", "r");

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

	fclose(fp);
	sort(n);
	csr(nc, n);
	spmv(n, nr, nc);

	system("pause");
	return 0;
}