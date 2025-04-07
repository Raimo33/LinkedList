/*================================================================================

File: main.c                                                                    
Creator: Claudio Raimondi                                                       
Email: claudio.raimondi@pm.me                                                   

created at: 2025-04-04 16:42:53                                                 
last edited: 2025-04-07 15:30:17                                                

================================================================================*/

#include "linked_list.h"

static void run(t_node **head, t_node **tail, char *input);
static uint8_t tokenize_input(char *input);
static uint8_t handle_operation(t_node **head, t_node **tail, const char *command);
inline static bool m_isspace(const char c);
static size_t m_strlen(const char *s);
static bool m_strncmp(const char *s1, const char *s2, size_t n);

#define RUN_TEST(input, expected) \
  run(&head, &tail, input); \
  head = tail = NULL;

int main(void)
{
  t_node *head = NULL;
  t_node *tail = NULL;

  char *input = NULL;
  char *expected = NULL;

  //Parsing - two adjacent separators
  input = "~~";
  expected = "";
  RUN_TEST(input, expected);

  //Parsing - three adjacent separators
  input = "~~~";
  expected = "";
  RUN_TEST(input, expected);

  //Parsing - separator at end
  input = "ADD(x)~";
  expected = "";
  RUN_TEST(input, expected);

  //Add - malformed ADD
  input = "ADDD(x) ~ AADD(x) ~ AD D(x) ~ ADD (x) ~ ADD() ~ ADD ~ PRINT";
  expected = "";
  RUN_TEST(input, expected);

  //Add - add '\0'
  input = "ADD('\0') ~ PRINT";
  expected = "";
  RUN_TEST(input, expected);

  //Add - add two chars in same command
  input = "ADD(ab) ~ PRINT";
  expected = "";
  RUN_TEST(input, expected);

  //Add - add one
  input = "ADD(a) ~ PRINT";
  expected = "a";
  RUN_TEST(input, expected);

  //Add - add same element twice
  input = "ADD(a) ~ ADD(a) ~ PRINT";
  expected = "aa";

  //Sort - malformed SORT
  input = "ADD(2) ~ ADD(1) ~ SORTT ~ S ORT ~ SORT() ~ SSORT ~ PRINT";
  expected = "21";
  RUN_TEST(input, expected);

  //Sort - normal
  input = "ADD(2) ~ ADD(1) ~ SORT ~ PRINT";
  expected = "12";
  RUN_TEST(input, expected);

  //Sort - empty
  input = "SORT ~ PRINT";
  expected = "";
  RUN_TEST(input, expected);

  //Sort - one element
  input = "ADD(1) ~ SORT ~ PRINT";
  expected = "1";
  RUN_TEST(input, expected);

  //Sort - odd # of elements
  input = "ADD(3) ~ ADD(1) ~ ADD(2) ~ SORT ~ PRINT";
  expected = "123";
  RUN_TEST(input, expected);

  //Sort - even # of elements
  input = "ADD(4) ~ ADD(2) ~ ADD(3) ~ ADD(1) ~ SORT ~ PRINT";
  expected = "1234";
  RUN_TEST(input, expected);

  //Sort - already sorted
  input = "ADD(1) ~ ADD(2) ~ ADD(3) ~ ADD(4) ~ SORT ~ PRINT";
  expected = "1234";
  RUN_TEST(input, expected);

  //Delete - malformed DELETE
  input = "ADD(x) DE L(x) ~ DELL(x) ~ DDEL(x) ~ DEL() ~ DEL ~ PRINT";
  expected = "x";
  RUN_TEST(input, expected);

  //Delete - delete '\0'
  input = "ADD(x) ~ DEL('\0') ~ PRINT";
  expected = "x";
  RUN_TEST(input, expected);

  //Delete - delete two chars in same command
  input = "ADD(ab) ~ DEL(ab) ~ PRINT";
  expected = "";
  RUN_TEST(input, expected);

  //Delete - delete multiple elements
  input = "ADD(x) ~ ADD(y) ~ ADD(x) ~ DEL(x) ~ PRINT";
  expected = "y";
  RUN_TEST(input, expected);

  //Delete - delete non existing element
  input = "ADD(x) ~ DEL(y) ~ PRINT";
  expected = "x";
  RUN_TEST(input, expected);

  //Delete - delete empty list
  input = "DEL(x) ~ PRINT";
  expected = "";
  RUN_TEST(input, expected);

  //Rev - malformed REV
  input = "ADD(x) ~ R EV ~ RREV ~ REVV ~ REV(x) ~ PRINT";
  expected = "x";
  RUN_TEST(input, expected);

  //Rev - rev empty list
  input = "REV ~ PRINT";
  expected = "";
  RUN_TEST(input, expected);

  //Rev - rev one element
  input = "ADD(x) ~ REV ~ PRINT";
  expected = "x";
  RUN_TEST(input, expected);

  //Rev - rev odd # of elements
  input = "ADD(x) ~ ADD(y) ~ ADD(z) ~ REV ~ PRINT";
  expected = "zyx";
  RUN_TEST(input, expected);

  //Rev - rev even # of elements
  input = "ADD(x) ~ ADD(y) ~ ADD(z) ~ ADD(w) ~ REV ~ PRINT";
  expected = "wzyx";
  RUN_TEST(input, expected);

  //Rev - rev twice in a row
  input = "ADD(x) ~ ADD(y) ~ ADD(z) ~ ADD(w) ~ REV ~ REV ~ PRINT";
  expected = "wzyx";
  RUN_TEST(input, expected);

  //Print - malformed PRINT
  input = "ADD(x) ~ P RINT ~ PPRINT ~ PRINTT ~ PRINT()";
  expected = "";
  RUN_TEST(input, expected);

  //Print - print empty list
  input = "PRINT";
  expected = "";
  RUN_TEST(input, expected);

  //Print - print 1 element
  input = "ADD(x) ~ PRINT";
  expected = "x";

  //Print - print twice in a row
  input = "ADD(x) ~ PRINT ~ PRINT";
  expected = "x";

  //Sort - malformed SORT
  input = "ADD(4) ~ ADD(3) ~ S ORT ~ SSORT ~ SORTT ~ SORT() ~ PRINT";
  expected = "43";
  RUN_TEST(input, expected);

  //Sort - sort empty list
  input = "SORT ~ PRINT";
  expected = "";
  RUN_TEST(input, expected);

  //Sort - sort 1 element
  input = "ADD(x) ~ SORT ~ PRINT";
  expected = "x";
  RUN_TEST(input, expected);

  //Sort - sort odd # of elements
  input = "ADD(z) ~ ADD(y) ~ ADD(x) ~ SORT ~ PRINT";
  expected = "xyz";
  RUN_TEST(input, expected);

  //Sort - sort even # of elements
  input = "ADD(1) ~ ADD(4) ~ ADD(3) ~ ADD(2) ~ SORT ~ PRINT";
  expected = "1234";
  RUN_TEST(input, expected);

  //Sort - sort already sorted
  input = "ADD(1) ~ ADD(2) ~ ADD(3) ~ ADD(4) ~ SORT ~ PRINT";
  expected = "1234";
  RUN_TEST(input, expected);

  //Sort - sort list of same elements
  input = "ADD(1) ~ ADD(1) ~ ADD(1) ~ ADD(1) ~ SORT ~ PRINT";
  expected = "1111";
  RUN_TEST(input, expected);

  //Sort - sort twice in a row
  input = "ADD(3) ~ ADD(2) ~ ADD(1) ~ ADD(4) ~ SORT ~ SORT ~ PRINT";
  expected = "1234";
  RUN_TEST(input, expected);

  //TODO sdx test: malformed SDX: S DX ~ SSDX ~ SDXX ~ SDX()
  //TODO sdx test: sdx empty list
  //TODO sdx test: sdx 1 element
  //TODO sdx test: sdx 2 elements
  //TODO sdx test: sdx twice in a row

  //TODO ssx test: malformed SSX: S SX ~ SSXX ~ SSX() ~ SSX(x)
  //TODO ssx test: ssx empty list
  //TODO ssx test: ssx 1 element
  //TODO ssx test: ssx 2 elements
  //TODO ssx test: ssx twice in a row

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