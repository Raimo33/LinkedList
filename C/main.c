/*================================================================================

File: main.c                                                                    
Creator: Claudio Raimondi                                                       
Email: claudio.raimondi@pm.me                                                   

created at: 2025-04-04 16:42:53                                                 
last edited: 2025-04-09 15:34:25                                                

================================================================================*/

#include <stdio.h>

#include "linked_list.h"

static void run(t_node **head, t_node **tail, char *input);
static uint8_t tokenize(char *input, const char sep);
static uint8_t handle_operation(t_node **head, t_node **tail, char *command);
static bool is_valid_normal_command(const char *command);
static bool is_valid_parameterized_command(const char *command);
static bool is_valid_args(const char *args);
static bool m_strncmp(const char *s1, const char *s2, size_t n);
static size_t m_strlen(const char *s);

#define RUN_TEST(input, expected) \
  printf("running test: %s\n", input); \
  run(&head, &tail, input); \
  printf(", expected: %s\n", expected); \
  head = tail = NULL;

int main(void)
{
  t_node *head = NULL;
  t_node *tail = NULL;

  // //Parsing - two adjacent separators
  // {
  //   char input[] = "~~";
  //   char expected[] = "";
  //   RUN_TEST(input, expected);
  // }

  // //Parsing - three adjacent separators
  // {
  //   char input[] = "~~~";
  //   char expected[] = "";
  //   RUN_TEST(input, expected);
  // }

  // //Parsing - separator at end
  // {
  //   char input[] = "ADD(x)~";
  //   char expected[] = "";
  //   RUN_TEST(input, expected);
  // }

  // //Add - malformed ADD
  // {
  //   char input[] = "ADDD(x) ~ AADD(x) ~ AD D(x) ~ ADD (x) ~ ADD() ~ ADD ~ PRINT";
  //   char expected[] = "";
  //   RUN_TEST(input, expected);
  // }

  // //Add - add two chars in same command
  // {
  //   char input[] = "ADD(ab) ~ PRINT";
  //   char expected[] = "";
  //   RUN_TEST(input, expected);
  // }

  // //Add - add one
  // {
  //   char input[] = "ADD(a) ~ PRINT";
  //   char expected[] = "a";
  //   RUN_TEST(input, expected);
  // }

  // //Add - add same element twice
  // {
  //   char input[] = "ADD(a) ~ ADD(a) ~ PRINT";
  //   char expected[] = "aa";
  //   RUN_TEST(input, expected);
  // }

  // //Sort - malformed SORT
  // {
  //   char input[] = "ADD(2) ~ ADD(1) ~ SORTT ~ S ORT ~ SORT() ~ SSORT ~ PRINT";
  //   char expected[] = "21";
  //   RUN_TEST(input, expected);
  // }

  // //Sort - normal
  // {
  //   char input[] = "ADD(2) ~ ADD(1) ~ SORT ~ PRINT";
  //   char expected[] = "12";
  //   RUN_TEST(input, expected);
  // }

  // //Sort - empty
  // {
  //   char input[] = "SORT ~ PRINT";
  //   char expected[] = "";
  //   RUN_TEST(input, expected);
  // }

  // //Sort - one element
  // {
  //   char input[] = "ADD(1) ~ SORT ~ PRINT";
  //   char expected[] = "1";
  //   RUN_TEST(input, expected);
  // }

  // //Sort - odd # of elements
  // {
  //   char input[] = "ADD(3) ~ ADD(1) ~ ADD(2) ~ SORT ~ PRINT";
  //   char expected[] = "123";
  //   RUN_TEST(input, expected);
  // }

  // //Sort - even # of elements
  // {
  //   char input[] = "ADD(4) ~ ADD(2) ~ ADD(3) ~ ADD(1) ~ SORT ~ PRINT";
  //   char expected[] = "1234";
  //   RUN_TEST(input, expected);
  // }

  // //Sort - already sorted
  // {
  //   char input[] = "ADD(1) ~ ADD(2) ~ ADD(3) ~ ADD(4) ~ SORT ~ PRINT";
  //   char expected[] = "1234";
  //   RUN_TEST(input, expected);
  // }

  // //Delete - malformed DELETE
  // {
  //   char input[] = "ADD(x) DE L(x) ~ DELL(x) ~ DDEL(x) ~ DEL() ~ DEL ~ PRINT";
  //   char expected[] = "x";
  //   RUN_TEST(input, expected);
  // }

  // //Delete - delete two chars in same command
  // {
  //   char input[] = "ADD(ab) ~ DEL(ab) ~ PRINT";
  //   char expected[] = "";
  //   RUN_TEST(input, expected);
  // }

  // //Delete - delete multiple elements
  // {
  //   char input[] = "ADD(x) ~ ADD(y) ~ ADD(x) ~ DEL(x) ~ PRINT";
  //   char expected[] = "y";
  //   RUN_TEST(input, expected);
  // }

  // //Delete - delete non existing element
  // {
  //   char input[] = "ADD(x) ~ DEL(y) ~ PRINT";
  //   char expected[] = "x";
  //   RUN_TEST(input, expected);
  // }

  // //Delete - delete empty list
  // {
  //   char input[] = "DEL(x) ~ PRINT";
  //   char expected[] = "";
  //   RUN_TEST(input, expected);
  // }

  // //Rev - malformed REV
  // {
  //   char input[] = "ADD(x) ~ R EV ~ RREV ~ REVV ~ REV(x) ~ PRINT";
  //   char expected[] = "x";
  //   RUN_TEST(input, expected);
  // }

  // //Rev - rev empty list
  // {
  //   char input[] = "REV ~ PRINT";
  //   char expected[] = "";
  //   RUN_TEST(input, expected);
  // }

  // //Rev - rev one element
  // {
  //   char input[] = "ADD(x) ~ REV ~ PRINT";
  //   char expected[] = "x";
  //   RUN_TEST(input, expected);
  // }

  // //Rev - rev odd # of elements
  // {
  //   char input[] = "ADD(x) ~ ADD(y) ~ ADD(z) ~ REV ~ PRINT";
  //   char expected[] = "zyx";
  //   RUN_TEST(input, expected);
  // }

  // //Rev - rev even # of elements
  // {
  //   char input[] = "ADD(x) ~ ADD(y) ~ ADD(z) ~ ADD(w) ~ REV ~ PRINT";
  //   char expected[] = "wzyx";
  //   RUN_TEST(input, expected);
  // }

  // //Rev - rev twice in a row
  // {
  //   char input[] = "ADD(x) ~ ADD(y) ~ ADD(z) ~ ADD(w) ~ REV ~ REV ~ PRINT";
  //   char expected[] = "wzyx";
  //   RUN_TEST(input, expected);
  // }

  // //Print - malformed PRINT
  // {
  //   char input[] = "ADD(x) ~ P RINT ~ PPRINT ~ PRINTT ~ PRINT()";
  //   char expected[] = "";
  //   RUN_TEST(input, expected);
  // }

  // //Print - print empty list
  // {
  //   char input[] = "PRINT";
  //   char expected[] = "";
  //   RUN_TEST(input, expected);
  // }

  // //Print - print 1 element
  // {
  //   char input[] = "ADD(x) ~ PRINT";
  //   char expected[] = "x";
  //   RUN_TEST(input, expected);
  // }

  //Print - print twice in a row
  // {
  //   char input[] = "ADD(x) ~ PRINT ~ PRINT";
  //   char expected[] = "xx";
  //   RUN_TEST(input, expected);
  // }

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
  uint8_t n_commands = tokenize(input, '~');

  while (n_commands--)
  {
    input += (input[0] == ' '); // skip leading spaces
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
static uint8_t tokenize(char *input, const char sep)
{
  uint8_t n_commands = 1;
  char c = *input;
  bool is_delim = false;

  while (c)
  {
    is_delim = (c == sep);

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
static uint8_t handle_operation(t_node **head, t_node **tail, char *command)
{
  const uint8_t segment_len = m_strlen(command);

  tokenize(command, ' '); //tokenize by spaces
  const bool is_valid = is_valid_normal_command(command) || is_valid_parameterized_command(command);

  if (!is_valid)
    goto end;

  const char *commands[] = { "ADD(", "DEL(", "SORT\0", "REV\0", "PRINT\0" };
  const void (*functions[])(t_node **, t_node **, const char) = { add, del, sort, rev, print };
  const uint8_t command_lengths[] = { 4, 4, 5, 5, 6 };
  const uint8_t n_commands = sizeof(commands) / sizeof(commands[0]);

  for (uint8_t i = 0; i < n_commands; i++)
  {
    if (m_strncmp(command, commands[i], command_lengths[i]))
    {
      functions[i](head, tail, command[command_lengths[i]]);
      break;
    }
  }

end:
  return segment_len;
}

/*
description:
  checks if the command is a valid normal command (no parameters).
  returns true if the command is valid, false otherwise.

implementation:\
  use of array of commands and lengths for more readability / mantainability.
*/
static bool is_valid_normal_command(const char *command)
{
  const char *valid_commands[] = { "SORT\0", "REV\0", "PRINT\0"};
  const uint8_t command_lengths[] = { 4, 4, 5, 4, 6 };
  const uint8_t n_commands = sizeof(valid_commands) / sizeof(valid_commands[0]);

  for (uint8_t i = 0; i < n_commands; i++)
  {
    if (m_strncmp(command, valid_commands[i], command_lengths[i]))
      return true;
  }

  return false;
}

/*
description:
  checks if the command is a valid parameterized command (with parameters).
  returns true if the command is valid, false otherwise.

implementation:
  use of array of commands and lengths for more readability / mantainability.
*/
static bool is_valid_parameterized_command(const char *command)
{
  const char *valid_commands[] = { "ADD(", "DEL(" };
  const uint8_t command_lengths[] = { 4, 4 };
  const uint8_t n_commands = sizeof(valid_commands) / sizeof(valid_commands[0]);

  for (uint8_t i = 0; i < n_commands; i++)
  {
    if (m_strncmp(command, valid_commands[i], command_lengths[i]))
      return is_valid_args(&command[command_lengths[i]]);
  }

  return false;
}

/*
description:
  checks if the args are valid (only one char allowed).
  returns true if the args are valid, false otherwise.

implementation:
  use of bitwise | instead of logical || to avoid branches.
*/
static bool is_valid_args(const char *args)
{
  const char c = args[0];
  return (c != '\0') & (c != ')') & (args[1] == ')');
}

/*
definition:
  compares the first n characters of two strings.
  returns true if they are equal, false otherwise.
  returns false if either s1 or s2 are NULL.

implementation:
  lazy evaulation with an accumulator variable to avoid branches.
  use of bitwise & instead of logical && to avoid branches.
  generalized the function by handling edge cases even though it wont ever be called with such
*/
static bool m_strncmp(const char *s1, const char *s2, size_t n)
{
  bool null_check = ((uintptr_t)s1 | (uintptr_t)s2) == 0;

  bool result = true;
  while (n--)
  {
    char c1 = *s1;
    char c2 = *s2;
    s1++;
    s2++;

    bool both_null = (c1 == '\0') & (c2 == '\0');
    bool chars_diff = (c1 != c2);
    bool either_null = (c1 == '\0') | (c2 == '\0');

    result &= ~(chars_diff | either_null) | both_null;

    bool continue_mask = result != false;
    n *= continue_mask;
  }

  return result & !null_check;
}

static size_t m_strlen(const char *s)
{
  size_t len = 0;

  while (*s++)
    len++;

  return len;
}