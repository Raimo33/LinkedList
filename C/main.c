/*================================================================================

File: main.c                                                                    
Creator: Claudio Raimondi                                                       
Email: claudio.raimondi@pm.me                                                   

created at: 2025-04-04 16:42:53                                                 
last edited: 2025-04-04 17:29:22                                                

================================================================================*/

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
  //TODO avoid [i] indexing (it is an indirect sum)
*/
static uint8_t handle_operation(t_node **head, t_node **tail, const char *command)
{
  static const void (*simple_ops[])(t_node **, t_node **) = { print, sort, rev, sdx, ssx };
  static const char *simple_op_names[] = { "PRINT", "SORT", "REV", "SDX", "SSX" };
  static const uint8_t simple_op_names_len[] = { 5, 4, 3, 3, 3 };
  static const uint8_t n_simple_ops = sizeof(simple_ops) / sizeof(simple_ops[0]);

  static const void (*data_ops[])(t_node **, t_node **, const char) = { add, del };
  static const char *data_op_names[] = { "ADD(", "DEL(" };
  static const uint8_t data_op_names_len[] = { 4, 4 };
  static const uint8_t n_data_ops = sizeof(data_ops) / sizeof(data_ops[0]);

  static const shortest_command_len = 3;

  const uint8_t command_len = strlen(command);
  if (command_len < shortest_command_len) //early exit
    goto end;

  for (uint8_t i = 0; i < n_simple_ops; i++)
  {
    const uint8_t op_name_len = simple_op_names_len[i];

    if ((command_len >= op_name_len) && strncmp(command, simple_op_names[i], op_name_len))
    {
      simple_ops[i](head, tail);
      goto end;
    }
  }

  for (uint8_t i = 0; i < n_data_ops; i++)
  {
    const uint8_t op_name_len = data_op_names_len[i];

    if ((command_len >= op_name_len) && strncmp(command, data_op_names[i], op_name_len))
    {
      const char data = command[op_name_len];
      if (!data) //missing data, empty parenthesis
        goto end;

      const char closing_parenthesis = command[op_name_len + 1];
      if (closing_parenthesis != ')') //missing closing parenthesis, malformed parameters
        goto end;

      data_ops[i](head, tail, data);
      goto end;
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
  use of SWAR (SIMD within a register) to compare 8, 4, 2 bytes at a time. (not optimal for instruction cache, but 8x faster for long strings).
  SWAR works because the endianess of the memory is the same endianess of the ALU
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