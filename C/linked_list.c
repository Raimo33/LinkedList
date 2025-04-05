/*================================================================================

File: linked_list.c                                                             
Creator: Claudio Raimondi                                                       
Email: claudio.raimondi@pm.me                                                   

created at: 2025-04-04 16:42:53                                                 
last edited: 2025-04-05 18:21:09                                                

================================================================================*/

#include <stdlib.h>
#include <unistd.h>

#include "linked_list.h"

static void merge(t_node **head, t_node **tail, t_node *a, t_node *b);
static void m_putchar(const char c);

/*
description:
  takes pointers to the head and tail of the singly-linked list, and the character to add.
  adds the character to the end of the list.
  if the list is empty, head and tail will point to the new node.

implementation:
  O(1) insert thanks to the tail pointer.
  memory allocation of the new node.
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
  iterative function to avoid stack bloat.
*/
void del(t_node **head, t_node **tail, const char data)
{
  if (*head == NULL)
    return;

  t_node *current = *head;
  t_node *prev = NULL;

  while (current != NULL)
  {
    if (current->data == data)
    {
      if (prev == NULL) // deleting the head
      {
        *head = current->next;
        free(current);
        current = *head;
      }
      else
      {
        prev->next = current->next;
        free(current);
        current = prev->next;
      }
    }
    else
    {
      prev = current;
      current = current->next;
    }
  }

  if (*head == NULL)
    *tail = NULL;
}

/*
description:
  takes a pointer to the head of the singly-linked list.
  prints the list in order, followed by a newline.

implementation:
  recursive function for lower instruction count (better for the instruction cache).
  prints forwards instead of backwards to avoid the need to keep the stack of functions growing (parameter registers can be reused). aka tail-call optimization.
*/
void print(t_node **head)
{
  t_node *current = *head;

  if (!current)
  {
    m_putchar('\n');
    return;
  }

  m_putchar(current->data);
  print(&current->next);
}

static void m_putchar(const char c)
{
  write(1, &c, 1);
}

/*
description:
  takes pointers to the head and tail of the singly-linked list.
  sorts the list in ascending order by ASCII value.

implementation:
  use of mergesort because linked lists inherently don't provide random access. could be bypassed by constructing an array of pointers but that's a poor design choice O(n)...
  merge sort is the fastest known sorting algo for linked lists, even though finding the midpoint is O(n/2) = O(n).
  use of the slow-fast pointer technique to find the midpoint of the list. same performance as the doubly-linked list -> <- approach.
  implemented by changing pointers, even though a char is 1 byte and easier to copy, for better mantainability, reusability, and standard compliance.
  use trick multiplication to avoid branches.
*/
void sort(t_node **head, t_node **tail)
{
  if ((*head == NULL) | (*tail == NULL)) //same as (*head == NULL || *tail == NULL)
    return;

  // Find the middle using slow-fast pointer technique
  t_node *slow = *head;
  t_node *fast = *head;
  while (fast->next)
  {
    slow = slow->next;
    fast = fast->next->next;
  }

  printf("FAST: %p\n", fast);
  printf("SLOW: %p\n", slow);

  // Split the list
  t_node *a = *head;
  t_node *b = slow->next;
  slow->next = NULL;

  // Recursively sort both halves
  sort(&a, &slow);
  sort(&b, &fast);

  // Merge the sorted halves
  merge(head, tail, a, b);
}

/*
description:
  takes pointers to the head and tail of the singly-linked list.
  merges two sorted lists into one sorted list, updating head and tail respectively.
  it assumes that head and tail pointers are not NULL. (only meant to be used by sort function)

implementation:
  bitwise & instead of logical && to avoid branches.
  updating of the tail inside merge function to avoid having to iterate the whole list again.
  use of binary array to completely avoid branches by selectively chosing a or b.
*/
static void merge(t_node **head, t_node **tail, t_node *a, t_node *b)
{
  printf("MERGE CALLED\n");

  t_node *current = NULL;
  t_node *nodes[2] = {a, b};
  bool idx = (a->data > b->data); //i choose the smaller one

  current = nodes[idx];
  *head = current;

  while ((a != NULL) & (b != NULL)) //same as (a != NULL && b != NULL) for booleans
  {
    idx = (b->data < a->data); //i choose the smaller one

    t_node *chosen = nodes[idx];
    current->next = chosen;
    current = chosen;

    nodes[idx] = chosen->next;

    a = nodes[0];
    b = nodes[1];
  }

  //append the list with remaining elements
  current->next = nodes[(a == NULL)]; //i choose the non-null one

  //update the tail pointer
  while (current->next != NULL)
    current = current->next;
  *tail = current;
}

/*
description:
  takes pointers to the head and tail of the singly-linked list.
  reverses the list in place.

implementation:
  imlemented by changing pointers, even though a char is 1 byte and easier to copy, for better mantainability, reusability, and standard compliance.
  iterative function as the recursive one would either incur in stack bloat or require having a doubly-linked list.
*/
void rev(t_node **head, t_node **tail)
{
  if ((*head == NULL) | (*tail == NULL)) //same as (*head == NULL || *tail == NULL)
    return;

  t_node *prev = NULL;
  t_node *current = *head;
  t_node *next = NULL;
  t_node *original_head = *head;

  while (current != NULL)
  {
    next = current->next;
    current->next = prev;
    prev = current;
    current = next;
  }

  *head = prev;
  *tail = original_head;
}

/*
description:
  takes pointers to the head and tail of the singly-linked list.
  shifts all the elements to the right by one position, in a circular fashion.
  does nothing if the list is empty.

implementation:
  implemented by iterating the whole list to find the last node, in O(n).
  it is a compromise between performance and simplicity. doing it in O(1) would require:
   - a doubly-linked list, which is a pain in the a** to implement in assembly.
   - a third pointer to the second to last node, which is not allowed by the assignment.
*/
void sdx(t_node **head, t_node **tail)
{
  //TODO
}

/*
description:
  takes pointers to the head and tail of the singly-linked list.
  shifts all the elements to the left by one position, in a circular fashion.
  does nothing if the list is empty.

implementation:
  implemented by incrementing head and tail pointers, without iterating the whole list. O(1).
*/
void ssx(t_node **head, t_node **tail)
{
  if (*head == NULL)
    return;

  t_node *current = *head;
  *head = current->next;
  (*tail)->next = current;
  current->next = NULL;
  *tail = current;
}