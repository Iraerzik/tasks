#include <stdio.h>

int main(void) {
    int a, b;
    int result;

    printf("№8.Исследование операции %% с отрицательными операндами\n");
   
    printf("Отрицательное делимое, положительный делитель:\n");
    a = -17;
    b = 5;
    result = a % b;
    printf("   %d %% %d = %d\n", a, b, result);
    printf("   Проверка: (%d) = (%d) * (%d) + (%d)\n", a, a/b, b, result);
   
    printf("Положительное делимое, отрицательный делитель:\n");
    a = 17;
    b = -5;
    result = a % b;
    printf("   %d %% %d = %d\n", a, b, result);
    printf("   Проверка: (%d) = (%d) * (%d) + (%d)\n", a, a/b, b, result);

    printf("Оба отрицательные:\n");
    a = -17;
    b = -5;
    result = a % b;
    printf("   %d %% %d = %d\n", a, b, result);
    printf("   Проверка: (%d) = (%d) * (%d) + (%d)\n", a, a/b, b, result);

    printf("Итого:\n");
    printf("Результат %% всегда удовлетворяет уравнению:\n");
    printf("a = (a / b) * b + (a %% b)\n");
    printf("Знак результата совпадает с знаком делимого (a).\n");

    return 0;
}
