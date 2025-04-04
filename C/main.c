#include "linked_list.h"

static void run(t_node **head, t_node **tail, char *input);
static uint8_t tokenize_input(char *input);
static uint8_t handle_operation(t_node **head, t_node **tail, const char *command);
inline static bool is_space(const char c);
static size_t strlen(const char *s);
static bool strncmp(const char *s1, const char *s2, size_t n);

int main(void)
{
  t_node *head = NULL;
  t_node *tail = NULL;
  char input[] = "ADD(1) ~ ADD(a) ~ ADD(a) ~ ADD(B) ~ ADD(;) ~ ADD(9) ~SORT~PRINT~DEL(b) ~DEL(B) ~PRI~REV~PRINT";

  run(&head, &tail, input);
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
    input += is_space(input[0]); // skip leading space
    input += handle_operation(head, tail, input);
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
  array of function pointers for mantainability and readability.
  static arrays for operation names and lengths to avoid repeated calls to strlen.
  static variables to keep along multiple calls.
*/
static uint8_t handle_operation(t_node **head, t_node **tail, const char *command)
{
  static const void (*operations[])(t_node **, t_node **, const char) = { add, del, print, sort, rev, sdx, ssx };
  static const char *op_names[] = { "ADD(", "DEL(", "PRINT", "SORT", "REV", "SDX", "SSX" };
  static const uint8_t op_names_len[] = { 3, 3, 5, 4, 3 };
  static const shortest_name_len = 3;
  static const uint8_t n_operations = sizeof(operations) / sizeof(operations[0]);

  const uint8_t command_len = strlen(command);
  if (command_len < shortest_name_len) //early exit
    goto end;

  //TODO avoid [i] indexing (it is an indirect sum) 
  for (uint8_t i = 0; i < n_operations; i++)
  {
    if ((command_len >= op_names_len[i]) & strncmp(command, op_names[i], op_names_len[i])) //equivalent to && for integer types
    {
      const char data = command[op_names_len[i]]; //get the character after the operation name (either a '\0' or the data)
      operations[i](head, tail, data);
      break;
    }
  }

end:
  return command_len;
}

/*
definition:
  compares the first n characters of two strings.
  returns true if they are equal, false otherwise.
  expects both strings to be at least n characters long (Undefined behavior otherwise).
  if n is 0, it returns true.

implementation:
  use of SWAR (SIMD within a register) to compare 8, 4, 2 bytes at a time. (not optimal for instruction cache, but 8x faster for long strings)
  unalignment is calculated to avoid misaligned memory access on architectures that don't support it.
  lazy evaulation with an accumulator variable (keep_going) to avoid branches.
  use of bitwise & instead of logical && to avoid branches.
*/
static bool strncmp(const char *s1, const char *s2, size_t n)
{
  uint8_t unalignment = (-(uintptr_t)s1) & 7; // number of bytes until the next 8-byte boundary

  //reuse the same variables, in the max possible size
  uint64_t a = 0;
  uint64_t b = 0;

  bool keep_going = (n > 0);
  while (keep_going & unalignment) //equivalent to (keep_going > 0 && unalignment > 0) for integer types
  {
    a = *(uint8_t *)s1;
    b = *(uint8_t *)s2;

    keep_going = (a != b);

    s1++;
    s2++;
    n--;
    unalignment--;

    keep_going &= (n > 0);
  }

  keep_going &= (n >= 8);
  while (keep_going)
  {
    a = *(uint64_t *)s1;
    b = *(uint64_t *)s2;

    keep_going = (a != b);

    s1 += 8;
    s2 += 8;
    n -= 8;

    keep_going &= (n >= 8);
  }

  keep_going &= (n >= 4);
  while (keep_going)
  {
    a = *(uint32_t *)s1;
    b = *(uint32_t *)s2;

    keep_going = (a != b);

    s1 += 4;
    s2 += 4;
    n -= 4;

    keep_going &= (n >= 4);
  }

  keep_going &= (n >= 2);
  while (n >= 2)
  {
    a = *(uint16_t *)s1;
    b = *(uint16_t *)s2;

    keep_going = (a != b);

    s1 += 2;
    s2 += 2;
    n -= 2;

    keep_going &= (n >= 2);
  }

  return (a == b);
}

/*
description:
  checks if a character is a whitespace (as defined by man isspace).
  returns true if it is, false otherwise.

implementation:
  use of ranges of ASCII values to reduce instructions.
  use of bitwise | insteado of logical || to avoid branches.
*/
inline static bool is_space(const char c)
{
  return ((c >= '\t') & (c <= 'r')) | (c == ' ');
}

static size_t strlen(const char *s)
{
  size_t len = 0;

  while (*s++)
    len++;

  return len;
}