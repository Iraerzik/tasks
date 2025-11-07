#include <stdio.h>
#include <stdlib.h>
#include "functions.h"

extern int c;
extern char **lst;
extern char *buf;
extern int sizebuf;
extern int sizelist;
extern int curbuf;
extern int curlist;
extern char *popular;

int main(){
	int count_word = 0; 
	char saved_char;
	typedef enum {Start, Word,First, Second,Spec,Newline, Stop} vertex;
	vertex V = Start;
	c = getchar();
	null_list();
	while(1==1) switch(V) {
			case Start:
						if (c ==' ' || c == '\t') c = getchar();
						else if (c==EOF){
							printf("\n Кол-во слов: %d \n",count_word);
						   	printf("Исходная строка: \n");	
							termlist();
							printlist();
							printf("Строка в лексикографическом порядке: \n");
							insertsortlist();
							printlist();							
							count_popular();
							printf("Популярное слово: %s \n ", popular);
							clearlist();
							free(popular);
							V = Stop;
						}
						else if (c=='\n'){
							termlist();
							printf("Кол-во слов: %d \n",count_word);
						   	printf("Исходная строка: \n");
							printlist();
							insertsortlist();
							printf("Строка в лексикографическом порядке: \n");					
							printlist();
							count_popular();
							printf("Популярное слово: %s \n ", popular);
							count_word = 0;
							V = Newline;
							c = getchar();
						}
						else{
							nullbuf();
							addsym();
							if (c == '>' || c == '|' || c == '&'){
								   	V = First;
									saved_char = c;
							}
							else if (c == '<' || c == '(' || c == ')' || c == ':'){
									V = Spec;
							}
							else 
									V = Word;
							c = getchar();
				break;
				case Word:
						if (symset(c)){
								addsym();
								c = getchar();
						}
						else {
							V = Start;
							addword();
							count_word++;
						}
				break;

				case First:
					if(c == saved_char) {
						addsym();
						c = getchar();
						V = Second;
					}
					else {
						V = Start;
						addword();
						count_word++;
					}

				break;
				case Second:
					V = Start;
					addword();
					count_word++;
				break;
				case Spec:
					V = Start;
					addword();
					count_word++;
				break;
				
				case Newline:
					clearlist();
					V = Start;
				break;

				case Stop:
					exit(0);
				break;
			}

}
}
