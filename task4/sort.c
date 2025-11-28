#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Структура для хранения флагов сортировки
typedef struct {
    int reverse;          // -r
    int case_insensitive; // -f  
    int numeric;          // -n
    int skip_lines;       // +n
} sort_flags;

// Глобальная переменная для флагов (для qsort)
sort_flags g_flags;

// Функция для чтения всех строк из файла или stdin
char **read_lines(const char *filename, int *line_count) {
    FILE *file = filename ? fopen(filename, "r") : stdin;
    if (!file) {
        perror(filename ? filename : "stdin");
        return NULL;
    }
    
    char **lines = NULL;
    char buffer[4096];
    *line_count = 0;
    int capacity = 0;
    
    while (fgets(buffer, sizeof(buffer), file)) {
        // Удаляем символ новой строки в конце
        buffer[strcspn(buffer, "\n")] = '\0';
        
        // Увеличиваем массив при необходимости
        if (*line_count >= capacity) {
            capacity = capacity == 0 ? 16 : capacity * 2;
            lines = realloc(lines, capacity * sizeof(char *));
            if (!lines) {
                perror("realloc");
                fclose(file);
                return NULL;
            }
        }
        
        // Копируем строку
        lines[*line_count] = strdup(buffer);
        if (!lines[*line_count]) {
            perror("strdup");
            fclose(file);
            return NULL;
        }
        (*line_count)++;
    }
    
    if (file != stdin) {
        fclose(file);
    }
    
    return lines;
}

// Функция сравнения для строк
int compare_strings(const void *a, const void *b) {
    const char *str1 = *(const char **)a;
    const char *str2 = *(const char **)b;
    
    int result;
    
    if (g_flags.case_insensitive) {
        result = strcasecmp(str1, str2);
    } else {
        result = strcmp(str1, str2);
    }
    
    if (g_flags.reverse) {
        result = -result;
    }
    
    return result;
}

// Функция для числового сравнения
int compare_numbers(const void *a, const void *b) {
    const char *str1 = *(const char **)a;
    const char *str2 = *(const char **)b;
    
    double num1 = atof(str1);
    double num2 = atof(str2);
    
    int result;
    if (num1 < num2) result = -1;
    else if (num1 > num2) result = 1;
    else result = 0;
    
    if (g_flags.reverse) {
        result = -result;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    sort_flags flags = {0, 0, 0, 0};
    char *filename = NULL;
    
    // Парсинг аргументов
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-r") == 0) {
            flags.reverse = 1;
        } else if (strcmp(argv[i], "-f") == 0) {
            flags.case_insensitive = 1;
        } else if (strcmp(argv[i], "-n") == 0) {
            flags.numeric = 1;
        } else if (argv[i][0] == '+') {
            int n;
            if (sscanf(argv[i] + 1, "%d", &n) == 1) {
                flags.skip_lines = n;
            } else {
                filename = argv[i];
            }
        } else {
            filename = argv[i];
        }
    }
    
    // Чтение строк
    int line_count;
    char **lines = read_lines(filename, &line_count);
    if (!lines) {
        return 1;
    }
    
    // Сохраняем оригинальный указатель для освобождения памяти
    char **original_lines = lines;
    int original_line_count = line_count;
    
    // Обрабатываем пропуск строк (+n)
    if (flags.skip_lines > 0) {
        if (flags.skip_lines >= line_count) {
            flags.skip_lines = 0;
        } else {
            lines += flags.skip_lines;
            line_count -= flags.skip_lines;
        }
    }
    
    // Выбираем функцию сравнения и сортируем
    g_flags = flags;
    
    int (*compare_func)(const void *, const void *) = compare_strings;
    if (flags.numeric) {
        compare_func = compare_numbers;
    }
    
    qsort(lines, line_count, sizeof(char *), compare_func);
    
    // Вывод результата
    for (int i = 0; i < line_count; i++) {
        printf("%s\n", lines[i]);
    }
    
    // Освобождаем память (все оригинальные строки)
    for (int i = 0; i < original_line_count; i++) {
        free(original_lines[i]);
    }
    free(original_lines);
    
    return 0;
}
