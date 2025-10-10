#include <stdio.h>



double x,a ;



int main()
{
	double value = 0, derivative = 0,integral = 0;
	int n, count = 0 ;
	printf("Введите n: \n");
	scanf("%d", &n);
	n++;
	printf("Введите x \n");
	scanf("%lf", &x);
	printf("вводите коэфициенты \n");
	while (scanf("%lf",&a) == 1)
	{
		count++;
		integral = integral * x + (a/n);
		derivative = derivative * x + value;
		value = value * x + a;		
		n--;
	}
	integral *= x;
	printf("Значение многочлена: %.10g \n", value);
	printf("Производная: %.10g \n", derivative);
	printf("Интеграл: %.10g \n", integral);

}
