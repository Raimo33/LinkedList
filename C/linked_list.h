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

void add(t_node **head, t_node **tail, const char data);
void del(t_node **head, t_node **tail, const char data);
void print(t_node *head, t_node **tail, const char data);
void sort(t_node **head, t_node **tail, const char data);
void rev(t_node **head, t_node **tail, const char data);
void sdx(t_node **head, t_node **tail, const char data);
void ssx(t_node **head, t_node **tail, const char data);

#endif