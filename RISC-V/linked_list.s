.data

head: .word 0x0
tail: .word 0x0
list_input: .string "ADD(1)"

command_add:    .string "ADD("
command_del:    .string "DEL("
command_rev:    .string "REV\0"
command_sort:   .string "SORT\0"
command_print:  .string "PRINT\0"

//arrary of command strings
command_strings:
  .word command_add
  .word command_del
  .word command_rev
  .word command_sort
  .word command_print

//array of command string lengths
command_string_lengths:
  .byte 4
  .byte 4
  .byte 5
  .byte 5
  .byte 6

//array of function pointers
list_functions:
  .word add
  .word del
  .word rev
  .word sort
  .word print

n_commands: .byte 5

.text

/*general rules of thumb:
  - a0-a7 are used for function arguments and return values (a0 is the return value, a7 is the syscall argument)
  - s0-s11 are used for callee registers
  - t0-t6 are used for temporary registers
  - sp is the stack pointer
  - ra is the return address register
  - zero is the zero register (always 0)

if a function needs to use a s0-s11 register, it must save the value to the stack and restore it as it was before returning
if a function calls a function, it must save the ra register to the stack and restore it as it was before returning
*/

//TODO add tests
main:
  la a0, head_ptr
  la a1, tail_ptr
  la a2, list_input
  jal run
  j exit

//void run(t_node **head, t_node **tail, char *input)
run:
  addi sp, sp, -4
  sw ra, 0(sp)

  mv s0, a0 // head
  mv s1, a1 // tail
  mv s2, a2 // input

  //tokenize(input, '~')
  mv a0, s2
  mv a1, '~'
  jal tokenize
  mv s3, a0 // n_commands

  //while (n_commands > 0)
  run_while:
    beqz s3, run_end_while

    //input += (input[0] == ' ')
    lb t0, 0(s2)
    xori t0, t0, ' '
    seqz t0, t0
    add s2, s2, t0

    //input += handle_operation(head, tail, input)
    mv a0, s0
    mv a1, s1 
    mv a2, s2
    jal handle_operation
    add s2, s2, a0

    //input += (n_commands > 0)
    sltu t0, zero, s3
    add s2, s2, t0

    addi s3, s3, -1
    j run_while
  run_end_while:

  lw ra, 0(sp)
  addi sp, sp, 4
  jr ra

//uint8_t handle_operation(t_node **head, t_node **tail, char *command)
handle_operation:
  addi sp, sp, -4
  sw ra, 0(sp)

  //save S registers on the stack in order to make space
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

  mv s0, a0 // head
  mv s1, a1 // tail
  mv s2, a2 // command

  //uint8_t segment_len = strlen(command);
  mv a0, s2
  jal strlen
  mv s3, a0 // segment_len

  //tokenize(command, ' ')
  mv a0, s2
  mv a1, ' '
  jal tokenize

  //bool is_valid = is_valid_normal_command(command)
  mv a0, s2
  jal is_valid_normal_command
  mv s4, a0 // is_valid

  //is_valid |= is_valid_parameterized_command(command);
  mv a0, s2
  jal is_valid_parameterized_command
  or s4, s4, a0 // is_valid

  //if (!is_valid) goto end
  beqz s4, handle_operation_end

  //load statically compiled arrays
  la s5, command_strings
  la s6, command_string_lengths
  la s7, list_functions
  lb s8, n_commands

  //for (uint8_t i = 0; i < n_commands; i++)
  li t0, 0 // i
  handle_operation_for:
    beq t0, s8, handle_operation_end
    //uint8_t command_len = command_string_lengths[i]
    add s9, s6, t0
    lb s9, 0(s9) // command_len

    slli t1, t0, 2
    add t1, s5, t1
    lw t1, 0(t1) //command_strings[i]

    //strnmatch(command, command_str, command_len)
    mv a0, s2
    mv a1, t1
    mv a2, s9
    jal strnmatch

    //if (strnmatch(command, command_str, command_len) == false)
    li t1, 1
    bne a0, t1, handle_operation_for //continue

    //else -> call the function
    slli t1, t0, 2
    add t1, s7, t1
    lw t1, 0(t1) // functions[i]

    add t2, s2, s9
    lb t2, 0(t2) // c = command[command_len]

    // function(head, tail, c)
    mv a0, s0
    mv a1, s1
    mv a2, t2
    jalr ra, t1

    addi t0, t0, 1
    j handle_operation_end //break

handle_operation_end:
  //restore S registers from the stack
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

//bool strnmatch(const char *s1, const char *s2, size_t n)
//TODO

//uint8_t tokenize(char *input, const char sep)
tokenize:
  //since this function does not call any other function, we can use the a registers directly
  li t0, 1 //n_commands
  lb t1, 0(a0) //c
  li t2, 0 //is_delim

  //while (c != '\0')
  tokenize_while:
    beqz t1, tokenize_end_while

    //is_delim = (c == sep)
    xori t2, t1, a1
    seqz t2, t2

    //*input *= !is_delim;
    xori t3, t2, 1
    mul t1, t1, t3
    sb t1, 0(a0)

    //n_commands += is_delim
    add t0, t0, t2
    //input++
    addi a0, a0, 1
    //c = *input
    lb t1, 0(a0)
    j tokenize_while
  tokenize_end_while:

  //return n_commands
  mv a0, t0
  j ra

//exit stays as far as possible, unlikely scenarios far from the hot path (instruction cache locality)
exit:
  li a7, 10
  ecall