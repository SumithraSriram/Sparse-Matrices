#include<stdio.h>
#include<conio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <string.h>
#include<conio.h>
#include<stdlib.h>
#include<tmmintrin.h>
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include<cuda.h>

#define N 30

#pragma once
#pragma warning(disable : 4035)

int *row, *col, *ptr;
double *data;

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


__global__ void csrSpmv(double *x,double * y,int * ptr,int * indices,int * data, int * limiter)
{
	int i=threadIdx.x;
	int startrow = limiter[blockIdx.x];
	int stoprow = limiter[blockIdx.x+1];
	int nnz = ptr[stoprow] - ptr[startrow];
	int numrows = stoprow - startrow;
	int t = ptr[startrow];

	__shared__ double LDS[N];
	
	if(i<nnz)
	{
		LDS[i] = data[t+i] * x[indices[t+i]];
	}
     
	if ((startrow+i) < stoprow)
	{
		double temp =0.0;
		for(int j=(ptr[startrow+i]-t);j<(ptr[startrow+i+1]-t);j++)
			temp = temp+LDS[j];
		y[startrow+i]=temp;
	}	
}

void csr(int nr, int nc, int nonz)
{
     FILE *f1=fopen("CSRsample.txt","w");
     int k;
     for(k=0;k<=nr;k++)
     {
        *(ptr+k)=0;
    }
          
     int i,j=0;
     k=0;
     for(i=1;i<=nr;i++)
     {
     	 *(ptr+i)=*(ptr+i-1);
         for(j=k;j<=nonz;j++)
         {
         	if(*(row+j)==i)
         	{
         		*(ptr+i)=*(ptr+i)+1;
         		k++;
         	}
         	else
         		break;
         }
     }
     fprintf(f1, "%d %d %d\n",nr,nc,nonz);
     for(i=0;i<nonz;i++)
     {
         fprintf(f1, "%d ",*(col+i));
         fprintf(f1, "%lf\n",*(data+i));
     }
     for(i=0;i<=nr;i++)
     {
        fprintf(f1, "%d ",*(ptr+i));
       // printf("%d",*(ptr+i));
	 }
         
     fclose(f1);
	 
	/*for(i=0;i<nonz;i++)
     {
         printf("col vlus %d\t ",*(col+i));
         printf("data values %lf\t",*(data+i));
		 printf("pointer values %d \t",*(ptr+i));
     }*/
         
 }
 
 void sort(int nz)
 {
 	int i,j,temp,posn;
 	float t;
 	for(i=0;i<nz-1;i++)
	 {
	 	posn=i;
	 	for(j=i+1;j<nz;j++)
		 {
		 	if(*(row+posn)>*(row+j))
		 	    posn=j;
		 }	
		 if(posn != i)
		 {
		 	temp=*(row+i);
		 	*(row+i)=*(row+posn);
		 	*(row+posn)=temp;
		 	
		 	temp=*(col+i);
		 	*(col+i)=*(col+posn);
		 	*(col+posn)=temp;
		 	
		 	t=*(data+i);
		 	*(data+i)=*(data+posn);
		 	*(data+posn)=t;
		 }
	 }
	
	 printf("Sorting completed");
 }
 

int main()
{
    int h;
    int w;
    double * hostInputImageData;
    double * hostOutputImageData;
    int * hostptr;
	int * hostcol;
	double * hostdata;
	int * hostdelimiters;
    double * deviceInputImageData;
    double * deviceOutputImageData;
    int * deviceptr;
	int * devicecol;
	int * devicedata;
	int * devicedelimiters;
	char line[1024];
    
  FILE *fp, *f1, *f2; 
  int i,j,nnz;
  int c=1, sum=0;

  fp = fopen("Na5.txt", "r");
  
  fgets(line, 128, fp);
  while (line[0] == '%') 
  {
    fgets(line, 128, fp); 
  }
  sscanf(line,"%d %d %d\n", &h, &w, &nnz);
  printf("%d, %d, %d\n", h, w, nnz);
  row = (int *) malloc(nnz*sizeof(int));
  col = (int *) malloc(nnz*sizeof(int));
  ptr = (int *) malloc((h+1)*sizeof(int));
  data = (double *) malloc(nnz*sizeof(double));
  hostdelimiters = (int *) malloc(h*sizeof(int));

  for(i=0;i<nnz;i++)
  {
      fscanf(fp, "%d %d %lf\n", &(row[i]),&(col[i]),&(data[i]));
  }

  fclose(fp);

  sort(nnz);
  csr(h, w, nnz);
  
  hostcol = (int *) malloc(nnz*sizeof(int));
  hostdata = (double *) malloc(nnz*sizeof(double));
  hostptr = (int *) malloc((h+1)*sizeof(int));
  hostInputImageData = (double *) malloc(w*sizeof(double));
  hostOutputImageData = (double *) malloc(w*sizeof(double));

  for(i=0;i<nnz;i++)
  {
		*(hostcol+i) = col[i];
		*(hostdata+i) = data[i];
  }
  for(i=0;i<=h;i++)
  {
		*(hostptr+i) = ptr[i];
  }
  
  for(i=0;i<w;i++)
  {
		*(hostInputImageData+i) = 1;
  }  

  hostdelimiters[0]=0;
  for(i=1;i<h;i++)
  {
	  sum += hostptr[i]-hostptr[i-1];
	  if(sum < N)
		  continue;
	  else if(sum > N)
	  {
		  hostdelimiters[c] = i-1;
		  printf("%d ",hostdelimiters[c]);
		  i--;
		  c++;
		  sum = 0;
	  }
	  else
	  {
		  hostdelimiters[c]=i;
		  sum = 0;
		  c++;
	  }
  }
  hostdelimiters[++c]=h;

    cudaMalloc((void **) &deviceInputImageData, h * sizeof(int));
    cudaMalloc((void **) &deviceOutputImageData, h * sizeof(int));
    cudaMalloc((void **) &deviceptr, (h +1) * sizeof(int));
	cudaMalloc((void **) &devicecol, nnz * sizeof(int));
	cudaMalloc((void **) &devicedata, nnz * sizeof(double));
	cudaMalloc((void **) &devicedelimiters, h * sizeof(int));

	cudaMemcpy(deviceInputImageData, hostInputImageData, h * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(deviceptr, hostptr, (h +1) * sizeof(int) ,cudaMemcpyHostToDevice);
	cudaMemcpy(devicecol, hostcol, nnz * sizeof(int) ,cudaMemcpyHostToDevice);
	cudaMemcpy(devicedata, hostdata, nnz * sizeof(double) ,cudaMemcpyHostToDevice);
    cudaMemcpy(devicedelimiters, hostdelimiters, h * sizeof(int), cudaMemcpyHostToDevice);

	dim3 block(32,1,1);
	dim3 grid( (nnz)/N , 1 , 1);

	MyTimer timer;

	timer.Start();                // take initial reading
	Sleep(1000);                // wait a second 
	unsigned cpuspeed10 = (unsigned)(timer.Stop() / 100000);
	
	timer.Start();
	
	csrSpmv<<<grid, block>>>(deviceInputImageData, deviceOutputImageData, deviceptr, devicecol, devicedata, devicedelimiters);	

	unsigned cycle = (unsigned)timer.Stop();
	unsigned speed = (unsigned)(cycle / 100000);

	printf("\n\nTime = %d\n", cycle * 10000 / cpuspeed10);
    
    cudaMemcpy(hostOutputImageData,deviceOutputImageData, h * sizeof(double),cudaMemcpyDeviceToHost);
    

   	f2 = fopen("OutputC512.txt", "w");
  
  for(i=0;i<h;i++)
  {
	  fprintf(f2, "%lf\n", *(hostOutputImageData+i));
  }
  fclose(f2);

  cudaFree(deviceInputImageData);
    cudaFree(deviceOutputImageData);
    cudaFree(deviceptr);
	cudaFree(devicecol);
	cudaFree(devicedata);
	cudaFree(devicedelimiters);

    getch();
    return 0;
}
 
