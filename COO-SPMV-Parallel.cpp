#include<stdio.h>
#include<conio.h>
#include<stdlib.h>
#include<omp.h>

double *data;
int *col;
int *row;
int b[10000];
double *y;
int r=0, c=0;
int n=0;

void multiply()
{
     int i,j;
     y = (double *) malloc(c*sizeof(double));

     for(i=0;i<c;i++)
         *(y+i)=0;

	 #pragma omp_set_num_threads(2);
	 #pragma omp parallel for private (i)
     
     for(i=0;i<c;i++)
     {
		 printf("numf threadsin nn parsa %d\n",omp_get_num_threads());
         *(y+ *(row+i)) = *(y+ *(row+i)) + (*(data+i) * b[*(col+i)]);
     }

     printf("The answer is:\n");
     for(i=0;i<c;i++)
         printf("%lf\t",*(y+i));
}

int main()
{
  FILE *fp;
  char line[1024]; 
  int i,j;

  fp = fopen("COO sample.txt", "r");
  
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

  #pragma omp_set_num_threads(2);
	 #pragma omp parallel for private (i)
  for(i=0;i<c;i++)
  {
	  printf("numf threadsin nn parsa %d\n",omp_get_num_threads());
      b[i]=1;
  }

  multiply();
  
  getch();
  return 0;

}





         
             