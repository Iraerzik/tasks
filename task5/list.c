#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "list.h"

#define SIZE 16

/* Глобальные переменные */
char **lst = NULL;

/* Локальные статические переменные  */
static int c;                /* текущий символ */
static char *buf;            /* буфер для накопления текущего слова */
static int sizebuf;          /* размер буфера текущего слова */
static int sizelist;         /* размер списка слов */
static int curbuf;           /* индекс текущего символа в буфере */
static int curlist;          /* индекс текущего слова в списке */

/* ============= код из task 3  ============= */

void clearlist(void) {
    int i;
    if (lst == NULL) {
        sizelist = 0;
        curlist = 0;
        return;
    }
    for (i = 0; i < curlist; i++) {
        if (lst[i] != NULL) {
            free(lst[i]);
            lst[i] = NULL;
        }
    }
    free(lst);
    lst = NULL;
    sizelist = 0;
    curlist = 0;
}

void null_list(void) {
    sizelist = 0;
    curlist = 0;
    lst = NULL;
}

void termlist(void) {
    if (lst == NULL) return;
    if (curlist > sizelist - 1)
        lst = realloc(lst, (sizelist + 1) * sizeof(*lst));
    lst[curlist] = NULL;
    /* выравниваем используемую под список память точно по размеру списка */
    lst = realloc(lst, (sizelist = curlist + 1) * sizeof(*lst));
}

int compair(char *s1, char *s2) {
    while (*s1 && *s2 && *s1 == *s2) {
        s1++;
        s2++;
    }
    if (*s1 == *s2) return 0;
    else if (*s1 > *s2) return 1;
    else return -1;
}

void insertsortlist(void) {
    if (lst == NULL || curlist == 0) return;

    for (int i = 1; i < curlist; i++) {
        char *key = lst[i];
        int j = i - 1;
        while (j >= 0 && (compair(lst[j], key) == 1)) {
            lst[j + 1] = lst[j];
            j = j - 1;
        }
        lst[j + 1] = key;
    }
}

int specword(char *word) {
    if (word == NULL || *word == '\0') return 0;

    char *p = word;
    while (*p) {
        if ((*p == '>' || *p == '<' || *p == '|' || *p == '&' ||
             *p == '(' || *p == ')' || *p == ':' || *p == ';')) {
            p++;
        } else {
            return 0;
        }
    }
    return 1;
}

void nullbuf(void) {
    buf = NULL;
    sizebuf = 0;
    curbuf = 0;
}

void addsym(void) {
    if (curbuf > sizebuf - 1) {
        size_t new_size = sizebuf + SIZE;
        char *new_buf = realloc(buf, new_size);
        if (new_buf == NULL) {
            return;
        }
        buf = new_buf;
        sizebuf = new_size;
    }
    buf[curbuf++] = c;
}

void addword(void) {
    if (curbuf > sizebuf - 1) {
        size_t new_size = sizebuf + SIZE;
        char *new_buf = realloc(buf, new_size);
        if (new_buf == NULL) {
            return;
        }
        buf = new_buf;
        sizebuf = new_size;
    }
    buf[curbuf++] = '\0';

    /* выравниваем используемую память точно по размеру слова */
    buf = realloc(buf, sizebuf = curbuf);

    if (curlist > sizelist - 1)
        lst = realloc(lst, (sizelist += SIZE) * sizeof(*lst));

    lst[curlist++] = buf;
    buf = NULL;
    sizebuf = 0;
    curbuf = 0;
}

void printlist(void) {
    int i;
    if (lst == NULL) return;

    for (i = 0; i < sizelist - 1; i++)
        printf("%s\n", lst[i]);
}

int symset(int c) {
    return c != '\n' &&
           c != ' ' &&
           c != '\t' &&
           c != '>' &&
           c != '|' &&
           c != '&' &&
           c != ':' &&
           c != '<' &&
           c != '(' &&
           c != ')' &&
           c != EOF;
}

/* ============= Автомат для shell============= */

static void run_automaton(FILE *input) {
    char saved_char;
    char in_quote = 0;      /* 0 - нет кавычек, '\'' - одинарные, '\"' - двойные */
    int escaped = 0;
    
    typedef enum {Start, Word, First, Second, Spec} vertex;
    vertex V = Start;
    
    c = fgetc(input);
    null_list();
    
    while(1) {
        switch(V) {
            case Start:
                if (c == ' ' || c == '\t') {
                    c = fgetc(input);
                    break;
                } else if (c == EOF || c == '\n') {
                    termlist();
                    return;
                } else {
                    /* Обработка кавычек и экранирования */
                    if (escaped) {
                        escaped = 0;
                    } else if (c == '\\') {
                        escaped = 1;
                        c = fgetc(input);
                        break;
                    } else if (c == '\'' && in_quote != '\"') {
                        in_quote = (in_quote == '\'') ? 0 : '\'';
                        c = fgetc(input);
                        break;
                    } else if (c == '\"' && in_quote != '\'') {
                        in_quote = (in_quote == '\"') ? 0 : '\"';
                        c = fgetc(input);
                        break;
                    }
                    
                    nullbuf();
                    addsym();
                    
                    if (c == '>' || c == '|' || c == '&') {
                        V = First;
                        saved_char = c;
                    } else if (c == '<' || c == '(' || c == ')' || c == ':' || c == ';') {
                        V = Spec;
                    } else {
                        V = Word;
                    }
                    c = fgetc(input);
                }
                break;
                
            case Word:
                if (escaped) {
                    escaped = 0;
                    addsym();
                    c = fgetc(input);
                    break;
                } else if (c == '\\') {
                    escaped = 1;
                    c = fgetc(input);
                    break;
                } else if (c == '\'' && in_quote != '\"') {
                    in_quote = (in_quote == '\'') ? 0 : '\'';
                    c = fgetc(input);
                    break;
                } else if (c == '\"' && in_quote != '\'') {
                    in_quote = (in_quote == '\"') ? 0 : '\"';
                    c = fgetc(input);
                    break;
                }
                
                if (in_quote || symset(c)) {
                    addsym();
                    c = fgetc(input);
                } else {
                    V = Start;
                    addword();
                    in_quote = 0;
                    escaped = 0;
                }
                break;
                
            case First:
                if (escaped) {
                    escaped = 0;
                    addsym();
                    c = fgetc(input);
                    V = Word;
                    break;
                }
                
                if (c == saved_char) {
                    addsym();
                    c = fgetc(input);
                    V = Second;
                } else {
                    V = Start;
                    addword();
                }
                break;
                
            case Second:
                V = Start;
                addword();
                break;
                
            case Spec:
                V = Start;
                addword();
                break;
        }
    }
}

void list(void) {
    run_automaton(stdin);
}


List* create_list(void) {
    List *list = malloc(sizeof(List));
    if (!list) return NULL;
    
    list->lst = NULL;
    list->sizelist = 0;
    list->curlist = 0;
    
    return list;
}

void free_list(List *list) {
    if (!list) return;
    
    if (list->lst) {
        for (int i = 0; i < list->curlist; i++) {
            if (list->lst[i] != NULL) {
                free(list->lst[i]);
            }
        }
        free(list->lst);
    }
    
    free(list);
}

List* build_list_from_input(FILE *input) {
    /* Сбрасываем глобальные переменные */
    clearlist();
    nullbuf();
    
    /* Запускаем автомат */
    run_automaton(input);
    if (lst == NULL || curlist == 0) {
        clearlist();
        return NULL;
    }
    
    /* Создаем структуру List  */
    List *list = create_list();
    if (!list) {
        clearlist();
        return NULL;
    }
    
    list->lst = malloc((curlist + 1) * sizeof(char*));
    if (!list->lst) {
        free_list(list);
        clearlist();
        return NULL;
    }
    
    for (int i = 0; i < curlist; i++) {
        list->lst[i] = strdup(lst[i]);
    }
    list->lst[curlist] = NULL;
    list->sizelist = curlist + 1;
    list->curlist = curlist;
    
    clearlist();
    return list;
}

void expand_variables_in_list(List *list) {
    if (!list || !list->lst) return;
    
    for (int i = 0; i < list->curlist; i++) {
        char *word = list->lst[i];
        if (word && word[0] == '$') {
            int j = 1;
            while (word[j] && (isalnum(word[j]) || word[j] == '_')) {
                j++;
            }
            
            if (j > 1) {
                char var_name[256];
                strncpy(var_name, word + 1, j - 1);
                var_name[j - 1] = '\0';
                
                char *value = getenv(var_name);
                if (value) {
                    free(list->lst[i]);
                    list->lst[i] = strdup(value);
                } else {
                    free(list->lst[i]);
                    list->lst[i] = strdup("");
                }
            }
        }
    }
}

void process_quotes_in_list(List *list) {
    if (!list || !list->lst) return;
    
    for (int i = 0; i < list->curlist; i++) {
        char *word = list->lst[i];
        if (!word) continue;
        
        int len = strlen(word);
        char *result = malloc(len + 1);
        if (!result) continue;
        
        int pos = 0;
        char in_quote = 0;
        int escaped = 0;
        
        for (int j = 0; j < len; j++) {
            if (escaped) {
                result[pos++] = word[j];
                escaped = 0;
                continue;
            }
            
            if (word[j] == '\\') {
                escaped = 1;
                continue;
            }
            
            if ((word[j] == '\'' || word[j] == '\"') && 
                (in_quote == 0 || in_quote == word[j])) {
                in_quote = (in_quote == 0) ? word[j] : 0;
                continue;
            }
            
            result[pos++] = word[j];
        }
        result[pos] = '\0';
        
        free(word);
        list->lst[i] = result;
    }
}

void print_list(const List *list) {
    if (!list || !list->lst) return;
    
    for (int i = 0; i < list->curlist; i++) {
        if (list->lst[i] != NULL) {
            printf("%s\n", list->lst[i]);
        }
    }
}
