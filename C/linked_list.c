/*================================================================================

File: linked_list.c                                                             
Creator: Claudio Raimondi                                                       
Email: claudio.raimondi@pm.me                                                   

created at: 2025-04-04 16:42:53                                                 
last edited: 2025-04-04 16:42:53                                                

================================================================================*/

#include "linked_list.h"

static void putchar(const char c);

/*
description:
  takes pointers to the head and tail of the singly-linked list, and the character to add.
  adds the character to the end of the list.
  if the list is empty, head and tail will point to the new node.

implementation:
  O(1) insert thanks to the tail pointer.
  
*/
void add(t_node **head, t_node **tail, const char data)
{
  t_node *new_node = malloc(sizeof(t_node));
  if (!new_node)
    return;

  new_node->data = data;
  new_node->next = NULL;

  if (*head == NULL)
  {
    *head = new_node;
    *tail = new_node;
  }
  else
  {
    (*tail)->next = new_node;
    *tail = new_node;
  }
}

/*
description:
  takes pointers to the head and tail of the singly-linked list, and the character to delete.
  deletes all occurrences of the character from the list.
  if the string becomes empty, head and tail will point to NULL.

implementation:
  recursive function for lower instruction count (better for the instruction cache).
  deletes forwards instead of backwards to avoid the need to keep the stack of functions growing (parameter registers can be reused). aka tail-call optimization.
*/
void del(t_node **head, t_node **tail, const char data)
{
  t_node *current = *head;

  if (current == NULL)
    return;

  t_node *next = current->next; //i save the next node to avoid use after free

  if (current->data == data)
  {
    *head = next;
    free(current);
    if (*head == NULL)
      *tail = NULL;
  }

  del(&next, tail, data);
}

/*
description:
  takes a pointer to the head of the singly-linked list. the char data will be ignored.
  prints the list in order, followed by a newline.

implementation:
  recursive function for lower instruction count (better for the instruction cache).
  prints forwards instead of backwards to avoid the need to keep the stack of functions growing (parameter registers can be reused). aka tail-call optimization.
*/
void print(t_node **head, t_node **tail, const char data)
{
  if (head == NULL)
    putchar('\n');

  putchar(head->data);
  print(head->next, tail, data);
}

void sort()

static void putchar(const char c)
{
  write(1, &c, 1);
}