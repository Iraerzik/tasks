#include <stdio.h>
#include <stdlib.h>
#include "functions.h"
#define SIZE 16

/* Глобальные переменные */
int c; /* текущий символ */
char **lst; /* список слов (в виде массива) */
char *buf; /* буфер для накопления текущего слова */
int sizebuf; /* размер буфера текущего слова */
int sizelist; /* размер списка слов */
int curbuf; /* индекс текущего символа в буфере */
int curlist; /* индекс текущего слова в списке */
char *popular;

/* Прототипы функций 
void clearlist();
void null_list();
void termlist();
void sortlist();
void nullbuf();
void addsym();
void addword();
void printlist();
int symset(int c);
*/

void clearlist() {
    int i;
    sizelist = 0;
    curlist = 0;
    if (lst == NULL) return;
    for (i = 0; lst[i] != NULL; i++)
        free(lst[i]);
    free(lst);
    lst = NULL;
}

void null_list() {
    sizelist = 0;
    curlist = 0;
    lst = NULL;
}

void termlist() {
    if (lst == NULL) return;
    if (curlist > sizelist - 1)
        lst = realloc(lst, (sizelist + 1) * sizeof(*lst));
    lst[curlist] = NULL;
    /* выравниваем используемую под список память точно по размеру списка */
    lst = realloc(lst, (sizelist = curlist + 1) * sizeof(*lst));
}


int compair(char *s1,char *s2){ /*if s1>s2: return 1, if s1 = s2: 0 else -1*/
	while (*s1 && *s2 && *s1==*s2){
		s1++;
		s2++;
	}
	if (*s1 == *s2 ) return 0;
	else if  (*s1 > *s2) return 1;
	else return -1;
}

void insertsortlist(){
	if (lst == NULL|| curlist == 0) return;

	for (int i = 1; i < curlist; i++){
		char * key = lst[i];
		int j = i - 1;
		while (j >= 0 && (compair(lst[j],key)==1)){
			lst[j+1] = lst[j];
			j = j-1;
		}
		lst[j+1] = key;
	}
}

void count_popular(){
	if (lst == NULL){ popular = NULL; return ;}
	int counter = 1;
	int max_count  = 1;
	char *buf_word = lst[0];
	char *res_word = lst[0];	
	for (int i = 1; i < curlist ; i++){
		if (compair(buf_word,lst[i])== 0) counter++;
		else {
			if (counter > max_count){
				max_count = counter;
				res_word = buf_word;
			}
			counter = 1;
			buf_word = lst[i];
		}
	}
	if (counter > max_count){
		max_count = counter;
		res_word = buf_word;
	}
	popular = res_word;
}


void nullbuf() {
    buf = NULL;
    sizebuf = 0;
    curbuf = 0;
}

void addsym() {
    if (curbuf > sizebuf - 1)
        buf = realloc(buf, sizebuf += SIZE); /* увеличиваем буфер при необходимости */
    buf[curbuf++] = c;
}

void addword() {
    if (curbuf > sizebuf - 1)
        buf = realloc(buf, sizebuf += 1); /* для записи '\0' увеличиваем буфер при необходимости */
    buf[curbuf++] = '\0';

    /* выравниваем используемую память точно по размеру слова */
    buf = realloc(buf, sizebuf = curbuf);

    if (curlist > sizelist - 1)
        lst = realloc(lst, (sizelist += SIZE) * sizeof(*lst)); /* увеличиваем массив под список при необходимости */
    
    lst[curlist++] = buf;
}

void printlist() {
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

