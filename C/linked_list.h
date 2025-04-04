#ifndef LINKED_LIST_H
# define LINKED_LIST_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct s_node
{
  char data;
  struct s_node *next;
} t_node;

void run(t_node **head, char *input);

void add(t_node **head, char data);
void del(t_node **head, char data);
void print(t_node *head);
void sort(t_node **head); //TODO maybe also the tail
void rev(t_node **head);

#endif