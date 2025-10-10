#include <stdio.h>

double count_root(double x,double e)
{
	double tmp,r;
	double xi = 1,xi_1;
	xi_1 = (xi + x/xi)/2;

	while ((xi - xi_1 > 0) ? (xi - xi_1 >= e) : (xi_1 - xi >= e))
	{
		xi = xi_1;
		xi_1 =(xi + x/xi)/2;
		
	}
	return xi_1;
}


int main()
{
	double x,e;
	printf("введите e: \n");
	scanf("%lf",&e);
	printf("вводите числа: \n");
	while (scanf("%lf", &x) == 1)
		if (x != 0)
			printf("%.10g \n", count_root(x,e));
		else 
			printf("%.10g \n", x);

}
