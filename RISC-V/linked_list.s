.data

head: .word 0x0
tail: .word 0x0
list_input: .string "ADD(1)"

.rodata

command_add:    .string "ADD("
command_del:    .string "DEL("
command_rev:    .string "REV\0"
command_sort:   .string "SORT\0"
command_print:  .string "PRINT\0"

.text

# general rules of thumb:
#   - a0-a7 are used for function arguments and return values (a0 is the return value, a7 is the syscall argument)
#   - s0-s11 are used for callee registers
#   - t0-t6 are used for temporary registers
#   - sp is the stack pointer
#   - ra is the return address register
#   - zero is the zero register (always 0)

# if a function needs to use a s0-s11 register, it must save the value to the stack and restore it as it was before returning
# if a function calls a function, it must save the ra register to the stack and restore it as it was before returning

#TODO add tests
main:
  la a0, head_ptr
  la a1, tail_ptr
  la a2, list_input
  jal run
  j exit

#uint8_t tokenize(char *input, const char sep)
tokenize:
  li t0, 1 #n_commands
  lb t1, 0(a0) #c
  li t2, 0 #is_delim

  #while (c != '\0')
  tokenize_while:
    beqz t1, tokenize_end_while

    #is_delim = (c == sep)
    xori t2, t1, a1
    seqz t2, t2

    #*input *= !is_delim;
    xori t3, t2, 1
    mul t1, t1, t3
    sb t1, 0(a0)

    #n_commands += is_delim
    add t0, t0, t2
    #input++
    addi a0, a0, 1
    #c = *input
    lb t1, 0(a0)
    j tokenize_while
  tokenize_end_while:

  #return n_commands
  mv a0, t0
  j ra

#void run(t_node **head, t_node **tail, char *input)
run:
  addi sp, sp, -4
  sw ra, 0(sp)

  mv s0, a0 # head
  mv s1, a1 # tail
  mv s2, a2 # input

  #tokenize(input, '~')
  mv a0, s2
  mv a1, '~'
  jal tokenize
  mv s3, a0 # n_commands

  #while (n_commands > 0)
  run_while:
    beqz s3, run_end_while

    #input += (input[0] == ' ')
    lb t0, 0(s2)
    xori t0, t0, ' '
    seqz t0, t0
    add s2, s2, t0

    #input += handle_operation(head, tail, input)
    mv a0, s0
    mv a1, s1 
    mv a2, s2
    jal handle_operation
    add s2, s2, a0

    #input += (n_commands > 0)
    sltu t0, zero, s3
    add s2, s2, t0

    addi s3, s3, -1
    j run_while
  run_end_while:

  lw ra, 0(sp)
  addi sp, sp, 4
  jr ra

.rodata

#arrary of command strings
command_strings:
  .word command_add
  .word command_del
  .word command_rev
  .word command_sort
  .word command_print

#array of command string lengths
command_lengths:
  .byte 4
  .byte 4
  .byte 5
  .byte 5
  .byte 6

#array of function pointers
list_functions:
  .word add
  .word del
  .word rev
  .word sort
  .word print

n_commands: .byte 5

.text

#uint8_t handle_operation(t_node **head, t_node **tail, char *command)
handle_operation:
  addi sp, sp, -4
  sw ra, 0(sp)

  #save S registers on the stack in order to make space
  addi sp, sp, -40
  sw s0, 0(sp)
  sw s1, 4(sp)
  sw s2, 8(sp)
  sw s3, 12(sp)
  sw s4, 16(sp)
  sw s5, 20(sp)
  sw s6, 24(sp)
  sw s7, 28(sp)
  sw s8, 32(sp)
  sw s9, 36(sp)

  mv s0, a0 # head
  mv s1, a1 # tail
  mv s2, a2 # command

  #uint8_t segment_len = strlen(command);
  mv a0, s2
  jal strlen
  mv s3, a0 # segment_len

  #tokenize(command, ' ')
  mv a0, s2
  mv a1, ' '
  jal tokenize

  #bool is_valid = is_valid_normal_cmd(command)
  mv a0, s2
  jal is_valid_normal_cmd
  mv s4, a0 # is_valid

  #if (is_valid) -> early exit
  bnez s4, handle_operation_valid

  #is_valid = is_valid_parameterized_cmd(command);
  mv a0, s2
  jal is_valid_parameterized_cmd
  or s4, s4, a0 # is_valid

  #if (!is_valid) goto end
  beqz s4, handle_operation_end
handle_operation_valid:

  #load statically compiled arrays (no stalls since each load is independent)
  la s5, command_strings
  la s6, command_lengths
  la s7, list_functions
  lb s8, n_commands

  #for (uint8_t i = 0; i < n_commands; i++)
  li t0, 0 # i
  handle_operation_for:
    beq t0, s8, handle_operation_end
    #uint8_t command_len = command_lengths[i]
    add s9, s6, t0
    lb s9, 0(s9) # command_len

    slli t1, t0, 2
    add t1, s5, t1
    lw t1, 0(t1) #command_strings[i]

    #strnmatch(command, command_str, command_len)
    mv a0, s2
    mv a1, t1
    mv a2, s9
    jal strnmatch

    #if (strnmatch(command, command_str, command_len) == false)
    li t1, 1
    bne a0, t1, handle_operation_for #continue

    #else -> call the function
    slli t1, t0, 2
    add t1, s7, t1
    lw t1, 0(t1) # functions[i]

    add t2, s2, s9
    lb t2, 0(t2) # c = command[command_len]

    # function(head, tail, c)
    mv a0, s0
    mv a1, s1
    mv a2, t2
    jalr ra, t1

    addi t0, t0, 1
    j handle_operation_end #break

handle_operation_end:
  #restore S registers from the stack
  lw s0, 0(sp)
  lw s1, 4(sp)
  lw s2, 8(sp)
  lw s3, 12(sp)
  lw s4, 16(sp)
  lw s5, 20(sp)
  lw s6, 24(sp)
  lw s7, 28(sp)
  lw s8, 32(sp)
  lw s9, 36(sp)
  addi sp, sp, 40

  addi sp, sp, 4
  lw ra, 0(sp)
  j ra

#bool strnmatch(const char *s1, const char *s2, size_t n)
strnmatch:
  #bool safe_args = ((uintptr_t)s1 | (uintptr_t)s2) != 0;
  mv t0, a0
  or t0, t0, a1
  snez t0, t0

  li t1, 1 # bool result = true

  #while (n > 0)
  strnmatch_while:
    beqz a2, strnmatch_end_while
    lb t2, 0(a0) # c1
    lb t3, 0(a1) # c2

    #increment pointers while LOAD completes (no stall this way)
    addi a0, a0, 1
    addi a1, a1, 1

    #bool both_null = (c1 == '\0') & (c2 == '\0')
    seqz t4, t2
    seqz t5, t3
    and t4, t4, t5

    #bool chars_diff = (c1 != c2)
    xor t5, t2, t3
    snez t5, t5

    #bool either_null = (c1 == '\0') | (c2 == '\0')
    seqz t6, t2
    seqz t7, t3
    or t6, t6, t7

    #result &= ~(chars_diff | either_null) | both_null
    or t5, t5, t6
    not t5, t5
    or t5, t5, t4
    and t1, t1, t5

    #bool continue_mask = result != false
    snez t1, t1

    #n *= continue_mask;
    and a2, a2, t1

    addi a2, a2, -1
    j strnmatch_while
  strnmatch_end_while:

  #return result & safe_args
  and a1, t0, t1
  j ra

.rodata

command_strings_normal:
  .string "REV\0"
  .string "SORT\0"
  .string "PRINT\0"

command_lengths_normal:
  .byte 4
  .byte 5
  .byte 6

n_commands_normal: .byte 3

.text

#bool is_valid_normal_cmd(const char *command)
is_valid_normal_cmd:
  addi sp, sp, -4
  sw ra, 0(sp)

  #save S registers on the stack in order to make space
  addi sp, sp, -20
  sw s0, 0(sp)
  sw s1, 4(sp)
  sw s2, 8(sp)
  sw s3, 12(sp)
  sw s4, 16(sp)

  mv s0, a0 # command

  #load statically compiled arrays (no stalls since each load is independent)
  la s1, command_strings_normal
  la s2, command_lengths_normal
  lb s3, n_commands_normal

  li s3, 0 # i
  #for (uint8_t i = 0; i < n_commands; i++)
  is_valid_normal_cmd_for:
    beq s3, s3, is_valid_normal_cmd_end

    slli t0, s3, 2
    add t0, s1, t0
    lw t0, 0(t0) # command_strings[i]

    add t1, s2, s3
    lb t1, 0(t1) # command_lengths[i]

    #strnmatch(command, command_str, command_len)
    mv a0, s0
    mv a1, t0
    mv a2, t1
    jal strnmatch

    #if (strnmatch(command, command_str, command_len) == false) -> continue
    beqz a0, is_valid_normal_cmd_for 
  is_valid_normal_cmd_end_for:

  #restore S registers from the stack
  lw s0, 0(sp)
  lw s1, 4(sp)
  lw s2, 8(sp)
  lw s3, 12(sp)
  lw s4, 16(sp)
  addi sp, sp, 20

  lw ra, 0(sp)
  addi sp, sp, 4

  j ra

.rodata

command_strings_parameterized:
  .string "ADD("
  .string "DEL("

command_lengths_parameterized:
  .byte 4
  .byte 4

n_commands_parameterized: .byte 2

.text

#bool is_valid_parameterized_cmd(const char *command)
is_valid_parameterized_cmd:
  addi sp, sp, -4
  sw ra, 0(sp)

  #save S registers on the stack in order to make space
  addi sp, sp, -20
  sw s0, 0(sp)
  sw s1, 4(sp)
  sw s2, 8(sp)
  sw s3, 12(sp)
  sw s4, 16(sp)
  sw s5, 20(sp)

  mv s0, a0 # command

  #load statically compiled arrays (no stalls since each load is independent)
  la s1, command_strings_parameterized
  la s2, command_lengths_parameterized
  lb s3, n_commands_parameterized

  li s4, 0 # i
  #for (uint8_t i = 0; i < n_commands; i++)
  is_valid_parameterized_cmd_for:
    beq s4, s3, is_valid_parameterized_cmd_end_for

    add s5, s2, s4
    lb s5, 0(s5) #  uint8_t command_len = command_lengths[i]

    slli t0, s4, 2
    add t0, s1, t0
    lw t0, 0(t0) # command_strings[i]

    #strnmatch(command, command_str, command_len)
    mv a0, s0
    mv a1, t0
    mv a2, s5
    jal strnmatch

    #if (strnmatch(command, command_str, command_len) == false);
    bezq a0, is_valid_parameterized_cmd_for #continue 

    #return is_valid_args(&command[command_len]);
    add a0, s0, s5
    jal is_valid_args
  is_valid_parameterized_cmd_end_for: #at this point a0 will already contain the boolean result

  #restore S registers from the stack
  lw s0, 0(sp)
  lw s1, 4(sp)
  lw s2, 8(sp)
  lw s3, 12(sp)
  lw s4, 16(sp)
  lw s5, 20(sp)
  addi sp, sp, 24

  lw ra, 0(sp)
  addi sp, sp, 4

  j ra

#bool is_valid_args(const char *args)
is_valid_args:
  lb t0, 0(a0) # args[0]
  lb t1, 1(a0) # args[1]
  li t2, ')'

  #return (c != '\0') & (c != ')') & (args[1] == ')');
  snez t3, t0

  sub t5, t0, t2
  snez t5, t5

  sub t6, t1, t2
  seqz t6, t6

  and t3, t3, t5
  and t3, t3, t6

  mv a0, t3
  j ra

#size_t m_strlen(const char *s)
strlen:
  li t0, 0 #size_t len = 0

  #while (*s != '\0')
  strlen_while:
    lb t1, 0(a0) # c
    beqz t1, strlen_end_while

    addi t0, t0, 1
    addi a0, a0, 1
    j strlen_while
  strlen_end_while:

  #return len
  mv a0, t0
  j ra

.data

mempool: .space 150 #max of 30 nodes of 5 bytes each
free_list_head: .word 0x0

#void add(t_node **head, t_node **tail, const char data)


#exit stays as far as possible, unlikely scenarios far from the hot path (instruction cache locality)
exit:
  li a7, 10
  ecall