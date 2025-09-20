#include <stdio.h>
int main(void)
{
    printf("№10 Проверка 'ленивых' вычислений\n");
    int a = 1; /* true */
    int b = 0; /* false */
    printf("начальные значения a = %d,b = %d\n",a,b); 

    printf("операция &,когда 1ый ложь,(b & (a=4))\n");
    if(b && (a=4))
    	printf ("true\n");
    else 
	printf ("false\n");
    
    printf("a=%d\n",a);
    a = 1;

    printf("операция &,когда 1ый истина,(a & (b=2))\n");
    if(a && (b=2))
     	printf ("true\n");
    else 
	printf ("false\n");
    
    printf("b=%d\n",b);
    b = 0;

    printf("операция ||,когда 1ый ложь,(b || (a=4))\n");
    if(b || (a=4))
    	printf ("true\n");
    else 
	printf ("false\n");
    
    printf("a=%d\n",a);
    a = 1;
   
    printf("операция ||,когда 1ый истина,(a || (b=2))\n");
    if(a || (b=2))
     	printf ("true\n");
    else 
	printf ("false\n");
    
    printf("b=%d\n",b);
}
