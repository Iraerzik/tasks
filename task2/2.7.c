/* Вычисление значения выражения, содержащего цифры '0'-'9', знаки
* операций '+', '*' и скобки '(', ')''. ( Предполагается, что коды цифр
* упорядочены по возрастанию цифр и справедливо равенство '9'-'0'==9 как,
* например, в ASCII кодировке)
*/

#include <stdio.h>
#include <string.h>
#include <setjmp.h>

jmp_buf begin; /* точка начала диалога с пользователем */

char curlex; /* текущая лексема */

void getlex(void); /* выделяет из входного потока очередную лексему */
int expr(void);    /* распознает выражение и вычисляет его значение */
int add(void);     /* распознает слагаемое или вычитаемое и вычисляет значение */
int mult(void); 	/* распознает множитель или делитель и вычисляет значение */
int power(void);   /* распознает степень и вычисляет ее*/
void error(void);  /* сообщает об ошибке в выражении и передает управление
                    в начало функции main (точка begin) */
int factor(void);   /*распознает цифру или выражение в скобках*/

int ipow(int base, int exp)
{
	int res = 1;
	while (exp > 0)
	{
		res *= base;
		exp--;
	}
	return res;
}
int count = 0;
int res_count = 0;

int main()
{
	int result;
    setjmp(begin);
    printf("==>");
    getlex();
    result = expr();
    if (curlex != '\n') error();
	printf("результат");
    printf("\n%d\n", result);
	printf("max вложенность скобок:");
	printf("%d \n", res_count);
    return 0;
}

void getlex()
{
    while ((curlex = getchar()) == ' ');
}

void error(void)
{
    printf("\nОШИБКА!\n");
    while (getchar() != '\n');
    longjmp(begin, 1);
}

int expr()
{
    int e = add();
    while (curlex == '+' || curlex == '-')
    {
        char op = curlex;
		getlex();
		if (op == '+')   e += add();
		else e -= add();
    }
    return e;
}

int add()
{
    int a = mult();
    while (curlex == '*' || curlex == '/')
    {
		char op = curlex;
        getlex();
		if (op == '*')	a *= mult();
		else
		{
			int divisor = mult();
			if (divisor == 0)
				error();
			a /= divisor;
		}
    }
    return a;
}


int mult()
{
	int m = power();
	return m;
}
int power()
{
	int p = factor();
	if (curlex == '^')
	{
		getlex();
		int exp = power();
		p = ipow(p,exp);
	}
	return p;
}


int factor()
{
    int f;
    switch (curlex)
    {
    case '0': case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9':
        f = curlex - '0';
        break;
    case '(':
		count +=1;
        getlex();
        f = expr();
        if (curlex == ')') 
		{
			   	res_count = (res_count > count)? res_count :count;
				count = 0;
				break;
		}
        /* иначе ошибка - нет закрывающей скобки */
    default:
        error();
    }
    getlex();
    return f;
}

/* Сравните прототипы (в начале программы) и заголовки функций
* (в соответствующих определениях) error() и getlex() : информацию
* о параметрах (в данном случае пустые списки параметров - void) можно
* опускать в заголовке, если она есть в объявлении прототипа (как в случае
* с getlex) или не указывать в прототипе, а указать только в заголовке
* (как в случае с error). Тип обех функций - "функция, не возвращающая
* значения с пустым списком аргументов"
*/

