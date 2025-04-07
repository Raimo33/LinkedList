/*================================================================================

File: main.c                                                                    
Creator: Claudio Raimondi                                                       
Email: claudio.raimondi@pm.me                                                   

created at: 2025-04-04 16:42:53                                                 
last edited: 2025-04-05 18:21:09                                                

================================================================================*/

#include "linked_list.h"

static void run(t_node **head, t_node **tail, char *input);
static uint8_t tokenize_input(char *input);
static uint8_t handle_operation(t_node **head, t_node **tail, const char *command);
inline static bool m_isspace(const char c);
static size_t m_strlen(const char *s);
static bool m_strncmp(const char *s1, const char *s2, size_t n);

//TODO add tests for sdx and ssx?

int main(void)
{
  t_node *head = NULL;
  t_node *tail = NULL;

  char input12[] = "ADD(1)ADD(2) ~ ADD(3) ~ PRINT";
  char expected12[] = "3";
  run(&head, &tail, input12);
  printf("Expected: %s\n", expected12);
  reset_list(&head, &tail);
  
  //TODO add test: malformed ADD
  //TODO add test: add \0
  //TODO add test: add two chars in same command
  //TODO add test: add one
  //TODO add test: add same element twice
  
  //TODO sorting test: malformed SORT
  //TODO sorting test: normal
  //TODO sorting test: empty
  //TODO sorting test: one element
  //TODO sorting test: odd elements
  //TODO sorting test: even elements
  //TODO sorting test: already sorted
  
  //TODO delete test: malformed DEL
  //TODO delete test: del \0
  //TODO delete test: del two chars in same command
  //TODO delete test: delete multiple elements
  //TODO delete test: delete non existing element
  //TODO delete test: delete empty list

  
  //TODO rev test: malformed REV
  //TODO rev test: 
  
  return 0;
}

/*
definition:
  takes a pointer to the head of the linked list and a string input.
  it executes the corresponding operations on the linked list based on the input string.

implementation:
  commands are extracted from the input string one by one.
  each command is executed by calling the appropriate function.
*/
void run(t_node **head, t_node **tail, char *input)
{
  uint8_t n_commands = tokenize_input(input);

  while (n_commands--)
  {
    input += m_isspace(input[0]); // skip leading space if any
    input += handle_operation(head, tail, input);
    input += (n_commands > 0); // skip the delimiter, unless it's the last command
  }
}

/*
definition:
  tokenizes the input string by replacing delimiters with null terminators.
  it returns the number of commands found in the input string.
  it expects the input string to hold a maximum of 255 commands.

implementation:
  branchless conditionals to minimize branch mispredictions (pipeline stalls).
  temp variable used to reduce the number of dereferences.
  use of uint8_t to save space since the input is guaranteed to be < 30
*/
static uint8_t tokenize_input(char *input)
{
  uint8_t n_commands = 1;
  char c = *input;
  bool is_delim = false;

  while (c)
  {
    is_delim = (c == '~');

    *input *= !is_delim; // if is_delim, multiply by 0 (set to '\0'), otherwise multiply by 1 (keep the character)
    n_commands += is_delim;

    input++;
    c = *input;
  }

  return n_commands;
}

/*
definition:
  takes a pointer to the head of the linked list and a string operation name.
  it dispatches the operation to the appropriate function
  returns the length of the command string.

implementation:
  i considered:
    - array of function pointers + array of strings + array of lengths (too many registers needed)
    - switch case by comparing just the first char (not correct, no sanitization, not scalable)
    - perfect hash function (not correct, false positives, development time overhead)
  in the end an if-else forest seems the best
*/
static uint8_t handle_operation(t_node **head, t_node **tail, const char *command)
{
  const uint8_t command_len = m_strlen(command);

  if (m_strncmp(command, "ADD(", 4))
    add(head, tail, command[4]); 
  else if (m_strncmp(command, "DEL(", 4))
    del(head, tail, command[4]);
  else if (m_strncmp(command, "SORT", 4))
    sort(head, tail);
  else if (m_strncmp(command, "REV", 3))
    rev(head, tail);
  else if (m_strncmp(command, "PRINT", 5))
    print(head);
  else if (m_strncmp(command, "SDX", 3))
    sdx(head, tail);
  else if (m_strncmp(command, "SSX", 3))
    ssx(head, tail);

  return command_len;
}

/*
definition:
  compares the first n characters of two strings.
  returns true if they are equal, false otherwise.
  returns false if either s1 or s2 are NULL.
  if n is 0, it returns true.

implementation:
  lazy evaulation with an accumulator variable (keep_going) to avoid branches.
  use of bitwise & instead of logical && to avoid branches.
  generalized the function by handling edge cases even though it wont ever be called with such
*/
static bool m_strncmp(const char *s1, const char *s2, size_t n)
{
  if (!s1 | !s2) //equivalent of (s1 == NULL || s2 == NULL)
    return false;

  char a = *s1;
  char b = *s2;

  bool is_equal = (a == b);
  bool keep_going;
  keep_going = (n > 0);
  keep_going &= (a != '\0') & (b != '\0');
  
  while (keep_going)
  {
    keep_going = is_equal = (a == b);

    s1++;
    s2++;
    a = *s1;
    b = *s2;
    n--;

    keep_going &= (a != '\0') & (b != '\0');
    keep_going &= (n > 0);
  }

  return is_equal & (n == 0);
}

/*
description:
  checks if a character is a whitespace (as defined by man isspace).
  returns true if it is, false otherwise.

implementation:
  use of ranges of ASCII values to reduce instructions.
  use of bitwise | insteado of logical || to avoid branches.
*/
inline static bool m_isspace(const char c)
{
  return ((c >= '\t') & (c <= '\r')) | (c == ' ');
}

static size_t m_strlen(const char *s)
{
  size_t len = 0;

  while (*s++)
    len++;

  return len;
}