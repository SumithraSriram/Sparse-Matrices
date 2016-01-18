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



struct packet
{
	int startrow;
	int startcolumn;
	int endrow;
	int endcolumn;
	struct csrf *csr;
};

struct csrf
{
	int nz;
	float data[100];
	int ptr[100];
	int col[100];
};

struct packet *p[10000000];
struct packet *x;
int density;
static int count = 0;
float a[10000][10000];
float b[1000000];
float y[1000000];

float *res;


int isEmpty(int sr, int er, int sc, int ec)
{
	int i, j;
	for (i = sr; i <= er; i++)
	{
		for (j = sc; j <= ec; j++)
		{
			if (a[i][j] != 0)
				return 0;
		}
	}
	return 1;
}

void convertcsr(struct packet *x)
{
	int c = 0;
	int p = 1;
	int i, j;
	x->csr = (struct csrf *) malloc(sizeof(struct csrf));
	x->csr->nz = 0;
	x->csr->ptr[0] = 0;
	for (i = x->startrow; i <= x->endrow; i++)
	{
		for (j = x->startcolumn; j <= x->endcolumn; j++)
		{
			if (a[i][j] != 0)
			{
				x->csr->data[c] = a[i][j];
				x->csr->col[c] = j;
				c++;
				x->csr->nz++;
			}
		}
		x->csr->ptr[p] = c;
		p++;
	}
}

void multiply(struct packet *x)
{
	int i, j, k, a;

	int q;
	float *c;
	__m128 data1, vect1, res1, temp, tempmr;
	float *tempd;

	tempd = (float *)malloc(4 * sizeof(float));
	c = (float *)malloc(2 * sizeof(float));

	//FILE *f1=fopen("SpmvCSR.txt","w");

	int t = 0;


	for (i = x->startrow; i <= x->endrow; i++)
	{
		for (k = x->csr->ptr[t]; k < x->csr->ptr[t + 1]; k = k + 4)
		{
			q = x->csr->ptr[t + 1] - x->csr->ptr[t];
			if (q - k < 4)
				a = q;
			else
				a = 4;
			//printf("data: ");
			for (j = 0; j<a; j++)
			{
				*(tempd + j) = x->csr->data[k + j];
				//	printf("%f ", x->csr->data[k+j]);
			}
			//printf("\n");
			while (j<4)
			{
				*(tempd + j) = 0;
				j++;
			}

			/*printf("tempd array:");
			for(j=0;j<4;j++)
			printf("%f", *(tempd+j));*/

			j = x->csr->col[k];

			data1 = _mm_loadu_ps(tempd);
			vect1 = _mm_loadu_ps((b + j));

			temp = _mm_mul_ps(data1, vect1);

			res1 = _mm_hadd_ps(temp, temp);
			res1 = _mm_hadd_ps(res1, res1);
			_mm_storeu_ps(c, res1);
			*(res + i) += *c;

		}
		//printf("%f\n", *(res+i));
		t++;
	}


	/*for(i=0;i<nr;i++)
	fprintf(f1, "%f\n ",*(res+i));
	fclose(f1);*/
}
void trans(int sr, int er, int sc, int ec)
{
	int mr = (sr + er) / 2;
	int mc = (sc + ec) / 2;
	int r = er - sr;
	int c = ec - sc;
	if (r >= density && c >= density)
	{
		if (!isEmpty(sr, mr, sc, mc))
			trans(sr, mr, sc, mc);
		if (!isEmpty(sr, mr, (mc + 1), ec))
			trans(sr, mr, (mc + 1), ec);
		if (!isEmpty((mr + 1), er, sc, mc))
			trans((mr + 1), er, sc, mc);
		if (!isEmpty((mr + 1), er, (mc + 1), ec))
			trans((mr + 1), er, (mc + 1), ec);
	}
	else if (c > density)
	{
		trans(sr, er, sc, mc);
		trans(sr, er, (mc + 1), ec);
	}
	else if (r > density)
	{
		trans(sr, mr, sc, ec);
		trans((mr + 1), er, sc, ec);
	}
	else
	{
		if (!isEmpty(sr, er, sc, ec))
		{
			p[count] = (struct packet *) malloc(sizeof(struct packet));
			x = *(p + count);
			x->startrow = sr;
			x->endrow = er;
			x->startcolumn = sc;
			x->endcolumn = ec;
			convertcsr(x);
			count++;
		}
	}
}

int main()
{
	int col;
	MyTimer timer;

	timer.Start();                // take initial reading
	Sleep(1000);                // wait a second 
	unsigned cpuspeed10 = (unsigned)(timer.Stop() / 100000);


	printf("Enter the number of columns\n");
	scanf_s("%d", &col);
	FILE * file;
	fopen_s(&file, "na5full.txt", "r");
	int i = 0, j, k;
	while (!feof(file))
	{
		j = 0;
		fscanf_s(file, "%f", &a[i][j]);
		for (j = 1; j<col; j++)
		{
			fscanf_s(file, "%f", &a[i][j]);
		}
		i++;
	}
	fclose(file);
	printf("Enter the density: ");
	scanf_s("%d", &density);

	trans(0, (col - 1), 0, (col - 1));
	//printf("Enter the vector elements\n");
	for (i = 0; i<col; i++)
	{
		b[i] = 1.0;
		y[i] = 0;
	}
	FILE *f;
	fopen_s(&f, "quad.txt", "w");
	for (i = 0; i<count; i++)
	{
		x = *(p + i);
		fprintf(f, "%d\t", x->startrow);
		fprintf(f, "%d\t", x->endrow);
		fprintf(f, "%d\t", x->startcolumn);
		fprintf(f, "%d\n", x->endcolumn);
		// fprintf("Data elements:");
		for (j = 0; j<x->csr->nz; j++)
			fprintf(f, "%f\t", x->csr->data[j]);
		fprintf(f, "\n");
		//fprintf("Column elements:");
		for (j = 0; j<x->csr->nz; j++)
			fprintf(f, "%d\t", x->csr->col[j]);
		fprintf(f, "\n");
		// fprintf("Pointer elements:");
		for (j = 0; j <= density; j++)
			fprintf(f, "%d\t", x->csr->ptr[j]);
		fprintf(f, "\n");
		//fprintf("\n");
	}
	fclose(f);
	res = (float *)malloc(col*sizeof(float));

	timer.Start();

	for (i = 0; i<count; i++)
	{
		x = *(p + i);
		multiply(x);
	}

	unsigned cycle = (unsigned)timer.Stop();
	unsigned speed = (unsigned)(cycle / 100000);

	printf("\n\nTime = %d", cycle * 10000 / cpuspeed10);

	system("pause");
	return 0;
}