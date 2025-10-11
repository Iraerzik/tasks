#include <stdio.h>

double str2double( char str[] )
{
	int i=0;
	double res = 0;
	double degree = 10;
	int sign = 1;
	int exp_sign = 1;
	int exp = 0;
	double fraction = 0;

	if (str[i] == '-')
	{	sign = -1;
		i++;
	}
	else if (str[i] == '+')
		i++;
		

	while (str[i]>= '0' && str[i] <= '9')
	{
		res = res * 10 + (str[i] - '0');
		i++;
	}

	if (str[i] == '.')
	{
		i++;
		while (str[i] >= '0' && str[i] <= '9')
		{
			fraction += (str[i] - '0')/degree;
			degree *= 10;
			i++;
		}
	}
	
	res += fraction;
	res *= sign;
		
	if (str[i] == 'e' || str[i] == 'E')
	{ 
		i++;
		if (str[i] == '-')
		{
			exp_sign = -1;
			i++;
		}
		else if (str[i] == '+')
			i++;
		
		while (str[i] >= '0' && str[i] <= '9')
		{
			exp = exp*10 + (str[i] - '0');
			i++;
		}
	}

	if (exp != 0)
	{
		double exp_degree = 1;
		if (exp_sign > 0)
			for (int j = 0; j < exp; j++)
				exp_degree *= 10;
		else
			for (int j = 0; j < exp; j++)
				exp_degree /= 10;

		res *= exp_degree;
	}
	return res;
}
		
	
int main()
{
	char str[100];
	printf("Введите строку-число: \n");
	while (scanf("%s",str) == 1)
		printf ("%.10g \n", str2double(str));
					
}
