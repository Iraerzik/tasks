#include <stdio.h>
#include <stdlib.h>


typedef struct tNode *tree;
typedef unsigned elemtype;
typedef struct tNode {
		elemtype elem;
		tree left;
		tree right;
	} tnode;

tree create_leaf(elemtype elem)
{
	tree tmp = malloc(sizeof(tnode));
	tmp->elem = elem;
	tmp->left = tmp->right = NULL;
	return tmp;
}

void add_elem(tree *t,elemtype new_elem)
{	
	if (t == NULL)
		return;
	tree tmp = *t;
	if (tmp==NULL)
	{
		*t = create_leaf(new_elem);
		return ;
	}
	if (new_elem < tmp->elem)
		add_elem(&tmp->left,new_elem);
	else
		if (new_elem > tmp->elem) 
			add_elem(&tmp->right,new_elem);
}

tree findMin(tree t)
{
	if (t==NULL) return NULL;
	if (t->left != NULL) return findMin(t->left);
	return t;
}

void delete_elem(tree *t,elemtype to_delete_elem)
{
	if (t==NULL)
		return;
	tree cur = *t;
	if (cur==NULL)
		return;
	if (to_delete_elem < cur->elem)
			delete_elem(&cur->left,to_delete_elem);
	else
		if (to_delete_elem > cur->elem)
			delete_elem(&cur->right,to_delete_elem);
	else {
		if (cur->left == NULL && cur->right == NULL)
		{			
			free(cur);
			*t = NULL;
		}
		else if (cur->left == NULL || cur->right == NULL)
			{
				tree child = (cur->left != NULL)? cur->left : cur->right;
				free(cur);
				*t = child;
			}
		else
		{	
			tree tmp = findMin(cur->right);
			cur->elem = tmp->elem;
			delete_elem(&cur->right,tmp->elem);
		}
	}
}
 
int found_elem(tree t,elemtype to_found_elem)
{
	if (t==NULL) return 0;
	if (t->elem == to_found_elem) return 1;
	if (to_found_elem < t->elem) return found_elem(t->left,to_found_elem);
	return found_elem(t->right,to_found_elem);
}

void free_tree(tree t)
{
	if (t != NULL)
	{
		free_tree(t->left);
		free_tree(t->right);
		free(t);
	}
}

int main()
{
	char sign;
	elemtype number;
	tree t=NULL;
	printf("вводите последовательность: ");
	while (scanf("%c%u", &sign, &number) == 2)
	{
		if (sign=='+')
			add_elem(&t,number);
		else if (sign == '-')
			delete_elem(&t,number);
		else if (sign == '?')
		{
			int f =found_elem(t,number);
			if (f) printf("yes \n");
			else printf("no \n");
		}
	}
	free_tree(t);
}
