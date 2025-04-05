/*================================================================================

File: linked_list.h                                                             
Creator: Claudio Raimondi                                                       
Email: claudio.raimondi@pm.me                                                   

created at: 2025-04-04 16:42:53                                                 
last edited: 2025-04-05 16:59:14                                                

================================================================================*/

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

void print(t_node **head);
void sort(t_node **head, t_node **tail);
void rev(t_node **head, t_node **tail);
void sdx(t_node **head, t_node **tail);
void ssx(t_node **head, t_node **tail);

#endif