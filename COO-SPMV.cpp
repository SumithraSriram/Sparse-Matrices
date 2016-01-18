#include<stdio.h>
#include<conio.h>
#include<stdlib.h>
#include<time.h>

double *data;
int *col;
int *row;
int b[10000];
double *y;
int r=0, c=0;
int n=0;

double diffclock(clock_t clock1, clock_t clock2)
{
	double diffticks = clock1 - clock2;
	double diffms = (diffticks * 10) / CLOCKS_PER_SEC;
	return diffms;
}

void multiply()
{
     int i,j;
     y = (double *) malloc(c*sizeof(double));
	 clock_t begin, end;

     for(i=0;i<c;i++)
         *(y+i)=0;

	 begin = clock();
     for(i=0;i<c;i++)
     {
         j=row[i];
         *(y+j) = *(y+j) + (*(data+i) * b[*(col+i)]);
     }
	 end = clock();

	printf("\nClock: %lf", diffclock(end, begin));

    /* printf("The answer is:\n");
     for(i=0;i<c;i++)
         printf("%lf\t",*(y+i));*/
}

int main()
{
  FILE *fp;
  char line[1024]; 
  int i,j;

  fp = fopen("Na5.txt", "r");
  
  fgets(line, 128, fp);
  while (line[0] == '%') 
  {
    fgets(line, 128, fp); 
  }

  sscanf(line,"%d %d %d\n", &r, &c, &n);
  printf("%d, %d, %d\n", r, c, n);
  row = (int *) malloc(n*sizeof(int));
  col = (int *) malloc(n*sizeof(int));
  data = (double *) malloc(n*sizeof(double));

  for(i=0;i<n;i++)
  {
      fscanf(fp, "%d %d %lf\n", &(row[i]),&(col[i]),&(data[i]));
  }

  fclose(fp);

  for(i=0;i<c;i++)
  {
      b[i]=1;
  }

  multiply();
  
  getch();
  return 0;

}





         
             