#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node *link;
typedef struct Node 
{
	char *elem;
	link next;
} node;

typedef link list;

void add_word(list *lp, char * lword)
{
	list t = malloc(sizeof(node));
	t->elem = malloc(sizeof(lword)+1);
	strcpy(t->elem,lword);
	t->next = NULL;
	if (*lp == NULL) *lp = t;
	else
	{
		list cur = *lp;
		while (cur->next != NULL) cur = cur->next;
		cur->next = t;
	}
}

int compair_word(char *s1,char *s2)
{
		while (*s1++ == *s2++) ; 
		if (*s1 == '\0') return 1;
		else return 0;
}


void delete_word(list *lp, char *lword)
{	
	if (*lp==NULL)	return;
	 list last_node = *lp;
 	
	while (last_node->next != NULL) 
	   last_node = last_node->next;
	
	list cur = *lp;
	list prev = NULL;

	while (cur != NULL)
	{	if (compair_word(cur->elem, lword) && cur != last_node)
		{
			list q = cur;
			cur = cur->next;

			if (prev == NULL) *lp = cur;
			else prev->next = cur;

			free(q->elem);
			free(q);
		}	
		else
		{
			prev = cur;
			cur = cur->next;
		}
	}
}
void print_word(char *p)
{
	if (p != NULL)
	{
		printf("%s ", p);
	}
}


void print_list(list lp)
{	
	while (lp != NULL)
	{
	print_word(lp->elem);
	lp = lp->next;
	}
	printf("\n");	
}	

void free_list(list lp)
{
	while(lp != NULL)
	{
		list tmp = lp->next;
		free(lp->elem);
		free(lp);
		lp = tmp;
	}
}



int main()
{	
	char word[100];
	list ls = NULL;
	char * last_word = NULL;
	printf("Вводите слова: \n");
	while (	scanf("%99s", word) == 1)
	{
		add_word(&ls, word);
	}
	last_word = word;
	delete_word(&ls,last_word);
	print_list(ls);
	free_list(ls);
}	
