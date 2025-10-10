#include <stdio.h>
#include <time.h>


long long  iteration(int i)
{
	long long tmp, F0 = 0, F1 = 1;
 	if (i==0) return 0;
	if (i==1) return 1;
	for (int j = 2; j <= i; j++)
		{	
			tmp = F1;
			F1 += F0;
			F0 = tmp;
			
		}
	return F1;
}

long long recursion(int i )
{
	if (i==0) return 0;
	if (i==1) return 1;
	return (recursion(i-2) + recursion(i-1));
}

int main()
{   int i;
	long long iter_res,rec_res;
	clock_t t0,t1;
	double time_iter,time_rec;
	printf("Введите i: \n");
	while (	scanf( "%d",&i) == 1)
	{	
		t0 = clock();
		iter_res = iteration(i);
		t1 = clock();
		time_iter =( double)(t1-t0)/ CLOCKS_PER_SEC;
		printf("Итеративно: %lld , время: %.6f \n",iter_res,time_iter);

		t0 = clock();
		rec_res = recursion(i);
		t1 = clock();
		time_rec = (double)(t1- t0)/CLOCKS_PER_SEC;	
		printf("Рекурсивно: %lld , время: %.6f  \n", rec_res, time_rec);
	}
}
