#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include "list.h"

/* Локальные переменные для рекурсивного спуска */
static char **tokens;
static int token_count;
static int current;
static char *current_tok;

/* Вспомогательные функции */
static char* get_token(void);
static void next_token(void);
static int is_operator(const char *tok);
static int is_redirect(const char *tok);
static int is_logical(const char *tok);

/* Функции рекурсивного спуска */
static tree parse_expression(void);
static tree parse_sequence(void);
static tree parse_pipeline(void);
static tree parse_command(void);

void error(const char *msg) {
    fprintf(stderr, "Ошибка синтаксиса: %s\n", msg);
    if (current_tok)
        fprintf(stderr, "Токен: '%s'\n", current_tok);
    exit(1);
}

static char* get_token(void) {
    if (current >= token_count) return "END";
    return tokens[current];
}

static void next_token(void) {
    if (current < token_count) {
        current++;
        current_tok = get_token();
    }
}

static int is_operator(const char *tok) {
    return strcmp(tok, "|") == 0 || strcmp(tok, ">") == 0 ||
           strcmp(tok, "<") == 0 || strcmp(tok, ">>") == 0 ||
           strcmp(tok, "&") == 0 || strcmp(tok, ";") == 0 ||
           strcmp(tok, "&&") == 0 || strcmp(tok, "||") == 0 ||
           strcmp(tok, "(") == 0 || strcmp(tok, ")") == 0;
}

static int is_redirect(const char *tok) {
    return strcmp(tok, ">") == 0 || strcmp(tok, "<") == 0 ||
           strcmp(tok, ">>") == 0;
}

static int is_logical(const char *tok) {
    return strcmp(tok, "&&") == 0 || strcmp(tok, "||") == 0;
}

/* Парсинг простой команды или подоболочки */
static tree parse_command(void) {
    tree node;
    
    /* Проверка на подоболочку */
    if (strcmp(current_tok, "(") == 0) {
        next_token();
        
        node = malloc(sizeof(struct node));
        node->type = NODE_SUBSHELL;
        node->command = strdup("subshell");
        node->args = NULL;
        node->left = parse_expression();
        
        if (strcmp(current_tok, ")") != 0) {
            error("ожидалась закрывающая скобка ')'");
        }
        next_token();
        
        node->right = NULL;
        return node;
    }
    
    /* Проверка, что это не оператор */
    if (is_operator(current_tok)) {
        error("ожидалась команда или выражение");
    }
    
    /* Создаём узел для команды */
    node = malloc(sizeof(struct node));
    node->type = NODE_COMMAND;
    
    /* Собираем все аргументы команды */
    int arg_count = 0;
    int start = current;
    
    while (current < token_count && !is_operator(tokens[current])) {
        arg_count++;
        current++;
    }
    
    current = start;
    node->args = malloc((arg_count + 1) * sizeof(char*));
    
    for (int i = 0; i < arg_count; i++) {
        node->args[i] = strdup(tokens[current]);
        next_token();
    }
    node->args[arg_count] = NULL;
    
    node->command = strdup(node->args[0]);
    node->left = NULL;
    
    /* Обработка перенаправлений */
    node->right = NULL;
    while (current < token_count && is_redirect(current_tok)) {
        tree redirect_node = malloc(sizeof(struct node));
        redirect_node->type = NODE_REDIRECT;
        redirect_node->command = strdup(current_tok);
        next_token();
        
        if (current >= token_count || is_operator(current_tok)) {
            error("ожидалось имя файла после оператора перенаправления");
        }
        
        redirect_node->args = malloc(2 * sizeof(char*));
        redirect_node->args[0] = strdup(current_tok);
        redirect_node->args[1] = NULL;
        next_token();
        
        redirect_node->left = NULL;
        redirect_node->right = node->right;
        node->right = redirect_node;
    }
    
    return node;
}

/* Парсинг конвейера */
static tree parse_pipeline(void) {
    tree left = parse_command();
    
    while (strcmp(current_tok, "|") == 0) {
        next_token();
        
        tree pipe_node = malloc(sizeof(struct node));
        pipe_node->type = NODE_PIPE;
        pipe_node->command = strdup("|");
        pipe_node->args = NULL;
        pipe_node->left = left;
        
        if (current >= token_count) {
            error("ожидалась команда после '|'");
        }
        
        pipe_node->right = parse_command();
        left = pipe_node;
    }
    
    return left;
}

/* Парсинг последовательности */
static tree parse_sequence(void) {
    tree left = parse_pipeline();
    
    while (strcmp(current_tok, ";") == 0) {
        next_token();
        
        tree seq_node = malloc(sizeof(struct node));
        seq_node->type = NODE_SEQ;
        seq_node->command = strdup(";");
        seq_node->args = NULL;
        seq_node->left = left;
        
        if (current >= token_count) {
            seq_node->right = NULL;
            return seq_node;
        }
        
        seq_node->right = parse_pipeline();
        left = seq_node;
    }
    
    return left;
}

/* Парсинг логических операторов */
static tree parse_expression(void) {
    tree left = parse_sequence();
    
    while (is_logical(current_tok)) {
        tree logic_node = malloc(sizeof(struct node));
        
        if (strcmp(current_tok, "&&") == 0) {
            logic_node->type = NODE_AND;
            logic_node->command = strdup("&&");
        } else {
            logic_node->type = NODE_OR;
            logic_node->command = strdup("||");
        }
        
        logic_node->args = NULL;
        logic_node->left = left;
        next_token();
        
        if (current >= token_count) {
            error("ожидалась команда после логического оператора");
        }
        
        logic_node->right = parse_sequence();
        left = logic_node;
    }
    
    /* Фоновое выполнение */
    if (strcmp(current_tok, "&") == 0) {
        tree bg_node = malloc(sizeof(struct node));
        bg_node->type = NODE_BG;
        bg_node->command = strdup("&");
        bg_node->args = NULL;
        bg_node->left = left;
        bg_node->right = NULL;
        next_token();
        left = bg_node;
    }
    
    return left;
}

/* Основная функция построения дерева */
tree build_tree(List *list) {
    if (!list || !list->lst || list->curlist == 0) {
        return NULL;
    }
    
    tokens = list->lst;
    token_count = list->curlist;
    current = 0;
    current_tok = get_token();
    
    tree root = parse_expression();
    
    if (current < token_count) {
        error("неожиданный токен в конце выражения");
    }
    
    return root;
}

/* Освобождение памяти дерева */
void free_tree(tree t) {
    if (!t) return;
    
    free_tree(t->left);
    free_tree(t->right);
    
    if (t->command) free(t->command);
    
    if (t->args) {
        for (int i = 0; t->args[i] != NULL; i++) {
            free(t->args[i]);
        }
        free(t->args);
    }
    
    free(t);
}

/* Печать дерева */
void print_tree(tree t, int depth) {
    if (!t) return;
    
    for (int i = 0; i < depth; i++) printf("  ");
    
    switch (t->type) {
        case NODE_COMMAND:
            printf("COMMAND: %s", t->command);
            if (t->args && t->args[1]) {
                printf(" (");
                for (int i = 1; t->args[i] != NULL; i++) {
                    printf("%s%s", t->args[i], t->args[i+1] ? " " : "");
                }
                printf(")");
            }
            break;
        case NODE_PIPE: printf("PIPE (|)"); break;
        case NODE_REDIRECT: printf("REDIRECT: %s %s", t->command, t->args[0]); break;
        case NODE_BG: printf("BACKGROUND (&)"); break;
        case NODE_SEQ: printf("SEQUENCE (;)"); break;
        case NODE_AND: printf("AND (&&)"); break;
        case NODE_OR: printf("OR (||)"); break;
        case NODE_SUBSHELL: printf("SUBSHELL ( )"); break;
    }
    printf("\n");
    
    print_tree(t->left, depth + 1);
    print_tree(t->right, depth + 1);
}
