#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "list.h"
#include "tree.h"
#include "exec.h"

static int should_exit = 0;

/* Обработчик SIGINT для main */
static void sigint_handler_main(int sig) {
    printf("\n");
    fflush(stdout);
}

/* Основная функция обработки команды */
int process_command(const char *input) {
    if (!input || strlen(input) == 0) {
        return 0;
    }
    
    if (strcmp(input, "exit") == 0) {
        should_exit = 1;
        return 0;
    }
    
    /* Создаём поток для разбора строки */
    FILE *stream = fmemopen((void*)input, strlen(input), "r");
    if (!stream) {
        perror("fmemopen");
        return -1;
    }
    
    /* Построение списка лексем */
    List *list = build_list_from_input(stream);
    fclose(stream);
    
    if (!list || list->curlist == 0) {
        if (list) free_list(list);
        return 0;  
	}
    
    /* Обработка кавычек и переменных */
    process_quotes_in_list(list);
    expand_variables_in_list(list);
    
    /* Построение синтаксического дерева */
    tree root = build_tree(list);
    if (!root) {
        fprintf(stderr, "Ошибка построения дерева\n");
        free_list(list);
        return -1;
    }
    
    #ifdef DEBUG
    printf("\nСинтаксическое дерево:\n");
    print_tree(root, 0);
    #endif
    
    /* Выполнение команды */
    int result = execute_tree(root);
    
    free_tree(root);
    free_list(list);
    
    return result;
}

/* Основной цикл shell */
int main(int argc, char *argv[]) {
    char input[1024];
    
    /* Настройка обработчика SIGINT для main */
    signal(SIGINT, sigint_handler_main);
    
    /* Настройка обработчиков сигналов для exec */
    setup_signal_handlers();
    
    /* Режим скрипта */
    if (argc > 1) {
        FILE *script = fopen(argv[1], "r");
        if (!script) {
            perror(argv[1]);
            return 1;
        }
        
        while (fgets(input, sizeof(input), script)) {
            input[strcspn(input, "\n")] = 0;
            printf("shell> %s\n", input);
            process_command(input);
            cleanup_zombies();
        }
        
        fclose(script);
        wait_for_background();
        return 0;
    }
    
   printf("Добро пожаловать в shell! Введите 'exit' для выхода.\n");
    
    while (!should_exit) {
        printf("shell> ");
        fflush(stdout);
        
        if (!fgets(input, sizeof(input), stdin)) {
            printf("\n");
            break;  /* ЖEOF */
        }
        
        input[strcspn(input, "\n")] = 0;  /* Убираем перевод строки */
        
        process_command(input);
        cleanup_zombies();
    }
    
    /* Восстановление обработчиков сигналов */
    reset_signal_handlers();
    wait_for_background();
    
    printf("До свидания!\n");
    return 0;
}
