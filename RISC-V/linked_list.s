#general rules of thumb:
#  - a0-a7 are used for function arguments and return values (a0 is the return value, a7 is the syscall argument)
#  - s0-s11 are used for callee registers
#  - t0-t6 are used for temporary registers
#  - sp is the stack pointer
#  - ra is the return address register
#  - zero is the zero register (always 0)

#if a function needs to use a s0-s11 register, it must save the value to the stack and restore it as it was before returning
#if a function calls a function, it must save the ra register to the stack and restore it as it was before returning

.data

head_ptr: .word 0x0
tail_ptr: .word 0x0
list_input: .string "ADD(3) ~ ADD(1) ~ ADD(2) ~ SORT ~ PRINT"

heap_ptr: .word 0x00FFFFFF

command_add:    .string "ADD("
command_del:    .string "DEL("
command_print:  .string "PRINT"
                .byte 0
command_sort:   .string "SORT"
                .byte 0
command_rev:    .string "REV"
                .byte 0

#arrary of command strings
command_strings:
  .word command_add
  .word command_del
  .word command_print
  .word command_sort
  .word command_rev

#array of command string lengths
command_lengths:
  .byte 4
  .byte 4
  .byte 6
  .byte 5
  .byte 5
  
.text

main:
  la a0, head_ptr
  la a1, tail_ptr
  la a2, list_input
  jal run
  j exit

.data

node_size: .byte 5
max_nodes: .byte 30

#memory pool for a max of 30 nodes (5 * 30 bytes). (Ripes does not support .space)
mempool: .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
         .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
         .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
         .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
         .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

#boolean array (0 = free, 1 = alloc'd) of size 30
free_list: .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
           .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
           .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

.text

#void *alloc_node(void)
alloc_node:
  la t0, node_size
  la t1, max_nodes
  la t2, mempool
  la t3, free_list

  lbu t0, 0(t0) #node_size
  lbu t1, 0(t1) #max_nodes

  #for (i = 0; i < max_nodes; i++)
  li t4, 0 #i = 0
  alloc_node_loop:
    bgeu t4, t1, alloc_node_full

    add t5, t3, t4
    lbu t6, 0(t5)
    beqz t6, alloc_node_found #if (free_list[i] == 0)

    addi t4, t4, 1
    j alloc_node_loop
alloc_node_full:
  li a0, 0
  ret
alloc_node_found:
  li t6, 1
  sb t6, 0(t5) #free_list[i] = 1

  mul t6, t4, t0 #offset = i * sizeof(t_node)
  add a0, t2, t6
  ret

#void add(t_node **head, t_node **tail, const char data)
list_add:
  addi sp, sp, -4
  sw ra, 0(sp)

  #save S registers on the stack in order to make space
  addi sp, sp, -12
  sw s0, 0(sp)
  sw s1, 4(sp)
  sw s2, 8(sp) 

  mv s0, a0 #head
  mv s1, a1 #tail
  mv s2, a2 #data

  #t_node *new_node = alloc_node(sizeof(t_node));
  li a0, 5
  jal alloc_node
  mv t0, a0 #new_node
  beqz t0, exit #if (new_node == NULL)

  sb s2, 0(t0) #new_node->data = data;
  sw zero, 1(t0) #new_node->next = NULL;

  lw t1, 0(s0)
  beqz t1, list_add_update_head #if (*head == NULL)

  lw t1, 0(s1)
  sw t0, 1(t1) #(*tail)->next = new_node
  sw t0, 0(s1) #*tail = new_node
  j list_add_end

list_add_update_head:
  sw t0, 0(s0) #*head = new_node
  sw t0, 0(s1) #*tail = new_node

list_add_end:
  #restore S registers from the stack
  lw s0, 0(sp)
  lw s1, 4(sp)
  lw s2, 8(sp)
  addi sp, sp, 12

  lw ra, 0(sp)
  addi sp, sp, 4
  ret

#void free_node(t_node *node)
free_node:
  la t0, mempool
  la t1, free_list
  la t2, node_size

  lbu t2, 0(t2) #node_size

  #index = (node - mempool) / sizeof(t_node)
  sub t3, a0, t0
  div t3, t3, t2

  #free_list[index] = 0
  add t4, t1, t3
  sb zero, 0(t4)

  ret

#void del(t_node **head, t_node **tail, const char data)
list_del:
  addi sp, sp, -4
  sw ra, 0(sp)

  #save S registers on the stack in order to make space
  addi sp, sp, -24
  sw s0, 0(sp)
  sw s1, 4(sp)
  sw s2, 8(sp)
  sw s3, 12(sp)
  sw s4, 16(sp)
  sw s5, 20(sp)

  mv s0, a0 #head
  mv s1, a1 #tail
  mv s2, a2 #data

  lw s3, 0(a0) #t_node *current = *head;
  li s4, 0 #t_node *prev = NULL;
  li s5, 0 #t_node *last = NULL;

  #while (current != NULL)
  list_del_loop:
    beqz s3, list_del_loop_end

    lb t3, 0(s3) #current->data

    xor t3, t3, a2
    beqz t3, list_del_remove_node  #if (current->data == data)

    mv s4, s3 #prev = current
    mv s5, s3 #last = current
    lw s3, 1(s3) #current = current->next

    j list_del_loop
  list_del_remove_node:
    mv t3, s3 #t_node *to_delete = current
    lw s3, 1(s3) #current = current->next

    beqz s4, list_del_update_head  #if (prev == NULL)
    sw s3, 1(s4) #prev->next = current
    j list_del_free_node

  list_del_update_head:
    sw s3, 0(s0) #*head = current

  list_del_free_node:
    #free_node(to_delete)
    mv a0, t3
    jal free_node

    j list_del_loop
  list_del_loop_end:

  sw s5, 0(s1) #*tail = last

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

  ret

#void print(t_node **head, t_node **tail, const char data)
list_print:
  addi sp, sp, -4
  sw ra, 0(sp)

  lw t0, 0(a0) #t_node *current = *head;
  #if (current == NULL) -> exit
  beqz t0, list_print_end

  lbu a0, 0(t0)
  li a7, 11 #syscall for putchar
  ecall

  #print(&current->next, NULL, 0);
  addi a0, t0, 1
  li a1, 0
  li a2, 0
  jal list_print

list_print_end:
  lw ra, 0(sp)
  addi sp, sp, 4

  ret

#void sort(t_node **head, t_node **tail, const char data)
list_sort:
  addi sp, sp, -4
  sw ra, 0(sp)

  #save S registers on the stack in order to make space
  addi sp, sp, -32
  sw s0, 0(sp)
  sw s1, 4(sp)
  sw s2, 8(sp)
  sw s3, 12(sp)
  sw s4, 16(sp)
  sw s5, 20(sp)
  sw s6, 24(sp)
  sw s7, 28(sp)

  mv s6, a0 #head
  mv s7, a1 #tail

  lw s0, 0(a0) #*head
  lw s1, 0(a1) #*tail

  beq s0, s1, list_sort_end # if (*head == *tail) -> return

  mv t2, s0 #t_node *slow = *head;
  mv t3, s0 #t_node *fast = *head;
  list_sort_loop:
    lw t4, 1(t3)
    beqz t4, list_sort_loop_end
    lw t5, 1(t4)
    beqz t5, list_sort_loop_end

    lw t2, 1(t2) #slow = slow->next
    mv t3, t5 #fast = fast->next->next

    j list_sort_loop
  list_sort_loop_end:

  mv s2, s0 #t_node *head_a = *head;
  mv s3, t2 #t_node *tail_a = slow;
  lw s4, 1(t2) #t_node *head_b = slow->next;
  mv s5, s1 #t_node *tail_b = *tail;
  sw zero, 1(t2) #slow->next = NULL;

  #save pointers on the stack to reference them by address
  addi sp, sp, -16
  sw s2, 0(sp)
  sw s3, 4(sp)
  sw s4, 8(sp)
  sw s5, 12(sp)

  addi a0, sp, 0
  addi a1, sp, 4
  jal list_sort #sort(&head_a, &tail_a)

  addi a0, sp, 8
  addi a1, sp, 12
  jal list_sort #sort(&head_b, &tail_b)

  lw s2, 0(sp) #head_a
  lw s4, 8(sp) #head_b

  addi sp, sp, 16 #free the pointers from the stack

  mv a0, s6
  mv a1, s7
  mv a2, s2
  mv a3, s4
  jal list_merge #merge(head, tail, head_a, head_b)

list_sort_end:

  #restore S registers from the stack
  lw s0, 0(sp)
  lw s1, 4(sp)
  lw s2, 8(sp)
  lw s3, 12(sp)
  lw s4, 16(sp)
  lw s5, 20(sp)
  lw s6, 24(sp)
  lw s7, 28(sp)
  addi sp, sp, 32

  lw ra, 0(sp)
  addi sp, sp, 4

  ret

#void merge(t_node **head, t_node **tail, t_node *a, t_node *b)
list_merge:
  li t0, 0 #t_node *current = NULL;
  li t1, 0 #t_node *chosen = NULL;

  addi sp, sp, -8
  mv t2, sp #t_node *nodes[2]
  sw a2, 0(t2) #nodes[0] = a
  sw a3, 4(t2) #nodes[1] = b

  lbu t3, 0(a2)
  lbu t4, 0(a3)
  sltu t3, t4, t3 #int8_t idx = (b->data < a->data)

  slli t3, t3, 2
  add t4, t2, t3 #nodes + idx
  lw t1, 0(t4) #chosen = nodes[idx]
  mv t0, t1 #current = chosen
  sw t0, 0(a0) #*head = current
  lw t1, 1(t1) #chosen = chosen->next
  sw t1, 0(t4) #nodes[idx] = chosen

  lw a2, 0(t2) #a = nodes[0]
  lw a3, 4(t2) #b = nodes[1]

  list_merge_loop:
    snez t3, a2
    snez t4, a3
    and t3, t3, t4
    beqz t3, list_merge_loop_end #while ((a != NULL) & (b != NULL))

    lbu t3, 0(a2)
    lbu t4, 0(a3)
    sltu t3, t4, t3 #int8_t idx = (b->data < a->data)

    slli t3, t3, 2
    add t4, t2, t3 #nodes + idx
    lw t1, 0(t4) #chosen = nodes[idx]
    sw t1, 1(t0) #current->next = chosen
    mv t0, t1 #current = chosen
    lw t1, 1(t1) #chosen = chosen->next
    sw t1, 0(t4) #nodes[idx] = chosen

    lw a2, 0(t2) #a = nodes[0]
    lw a3, 4(t2) #b = nodes[1]

    j list_merge_loop
  list_merge_loop_end:

  snez t3, a3

  slli t3, t3, 2
  add t4, t2, t3 #nodes + idx
  lw t1, 0(t4) #chosen = nodes[idx]

  sw t1, 1(t0) #current->next = chosen
  mv t0, t1 #current = chosen

  list_merge_tail_loop:
    #while (current->next)
    lw t1, 1(t0)
    beqz t1, list_merge_tail_loop_end

    mv t0, t1 #current = current->next

    j list_merge_tail_loop
  list_merge_tail_loop_end:

  sw t0, 0(a1) #*tail = current

  addi sp, sp, 8 #free the stack array
  ret

#void rev(t_node **head, t_node **tail, const char data)
list_rev:
  lw t0, 0(a0) #t_node *current = *head;
  lw t1, 0(a1)

  #if ((*head == NULL) | (*tail == NULL))
  seqz t2, t0
  seqz t3, t1
  or t2, t2, t3
  bnez t2, list_rev_end

  mv t1, t0 #t_node *original_head = *current
  li t2, 0 #t_node *prev = NULL
  li t3, 0 #t_node *next = NULL

  #while (current != NULL)
  list_rev_loop:
    beqz t0, list_rev_loop_end
    lw t3, 1(t0) #next = current->next
    sw t2, 1(t0) #current->next = prev
    mv t2, t0 #prev = current
    mv t0, t3 #current = next
    j list_rev_loop
  list_rev_loop_end:

  sw t2, 0(a0) #*head = prev
  sw t1, 0(a1) #*tail = original_head
list_rev_end:
  ret

.data

#array of function pointers
list_functions:
  .word list_add
  .word list_del
  .word list_print
  .word list_sort
  .word list_rev

n_commands: .byte 5

command_strings_normal:
  .word command_print
  .word command_sort
  .word command_rev

command_lengths_normal:
  .byte 6
  .byte 5
  .byte 4

n_commands_normal: .byte 3

command_strings_parameterized:
  .word command_add
  .word command_del

command_lengths_parameterized:
  .byte 4
  .byte 4

n_commands_parameterized: .byte 2

.text

#uint8_t tokenize(char *input, const char sep)
tokenize:
  li t0, 1 #n_commands
  lb t1, 0(a0) #c
  li t2, 0 #is_delim

  #while (c != '\0')
  tokenize_loop:
    beqz t1, tokenize_loop_end

    #is_delim = (c == sep)
    xor t2, t1, a1
    seqz t2, t2

    #*input *= !is_delim;
    xori t3, t2, 1
    mul t1, t1, t3
    sb t1, 0(a0)

    add t0, t0, t2 #n_commands += is_delim
    addi a0, a0, 1 #input++
    lb t1, 0(a0) #c = *input
    j tokenize_loop
  tokenize_loop_end:

  #return n_commands
  mv a0, t0
  ret

#void run(t_node **head, t_node **tail, char *input)
run:
  addi sp, sp, -4
  sw ra, 0(sp)

  mv s0, a0 #head
  mv s1, a1 #tail
  mv s2, a2 #input

  #tokenize(input, '~')
  mv a0, s2
  li a1, 126 #'~' in ASCII
  jal tokenize
  mv s3, a0 #n_commands

  #while (n_commands > 0)
  run_loop:
    beqz s3, run_loop_end
    addi s3, s3, -1

    #input += (input[0] == ' ')
    lb t0, 0(s2)
    xori t0, t0, 32 #SPACE in ASCII
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

    j run_loop
  run_loop_end:

  lw ra, 0(sp)
  addi sp, sp, 4
  jr ra

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

  mv s0, a0 #head
  mv s1, a1 #tail
  mv s2, a2 #command

  #uint8_t segment_len = strlen(command);
  mv a0, s2
  jal strlen
  mv s3, a0 #segment_len

  #tokenize(command, ' ')
  mv a0, s2
  li a1, 32 #SPACE in ASCII
  jal tokenize

  #bool is_valid = is_valid_normal_cmd(command)
  mv a0, s2
  jal is_valid_normal_cmd
  mv s4, a0 #is_valid

  #if (is_valid) -> early exit
  bnez s4, handle_operation_valid

  #is_valid = is_valid_parameterized_cmd(command);
  mv a0, s2
  jal is_valid_parameterized_cmd
  or s4, s4, a0 #is_valid

  #if (!is_valid) goto end
  beqz s4, handle_operation_end
handle_operation_valid:

  la s5, command_strings
  la s6, command_lengths
  la s7, n_commands

  lbu s7, 0(s7) #n_commands

  #for (uint8_t i = 0; i < n_commands; i++)
  li s4, 0 #i
  handle_operation_loop:
    beq s4, s7, handle_operation_end

    add s8, s6, s4
    lbu s8, 0(s8) #command_len

    slli s9, s4, 2
    add s9, s5, s9
    lw s9, 0(s9) #command_strings[i]

    #strnmatch(command, command_str, command_len)
    mv a0, s2
    mv a1, s9
    mv a2, s8
    jal strnmatch

    #if (strnmatch(command, command_str, command_len) == true)
      li s9, 1
      beq a0, s9, handle_operation_call_function

    addi s4, s4, 1
    j handle_operation_loop #continue

  handle_operation_call_function:
    la t0, list_functions
    slli t1, s4, 2
    add t0, t0, t1
    lw t0, 0(t0) #functions[i]

    add t1, s2, s8
    lb t1, 0(t1) #c = command[command_len]

    #function(head, tail, c)
    mv a0, s0
    mv a1, s1
    mv a2, t1
    jalr ra, t0, 0
handle_operation_end:
  mv t0, s3 #segment_len

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

  lw ra, 0(sp)
  addi sp, sp, 4

  mv a0, t0 #return segment_len
  ret

#bool strnmatch(const char *s1, const char *s2, size_t n)
strnmatch:
  #bool safe_args = ((uintptr_t)s1 | (uintptr_t)s2) != 0;
  mv t0, a0
  or t0, t0, a1
  snez t0, t0

  li t1, 1 #bool result = true

  #while (n > 0)
  strnmatch_loop:
    beqz a2, strnmatch_loop_end
    addi a2, a2, -1

    lb t2, 0(a0) #c1
    lb t3, 0(a1) #c2

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
    seqz t3, t3 #c2 is overwritten to reuse registers
    or t6, t6, t3

    #result &= ~(chars_diff | either_null) | both_null
    or t5, t5, t6
    not t5, t5
    or t5, t5, t4
    and t1, t1, t5

    #bool continue_mask = result != false
    snez t2, t1

    #n *= continue_mask;
    mul a2, a2, t2

    j strnmatch_loop
  strnmatch_loop_end:

  #return result & safe_args
  and a0, t0, t1
  ret

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

  mv s0, a0 #command

  #load statically compiled arrays (no stalls since each load is independent)
  la s1, command_strings_normal
  la s2, command_lengths_normal
  la s4, n_commands_normal

  lbu s4, 0(s4) #n_commands_normal

  li s3, 0 #i
  #for (uint8_t i = 0; i < n_commands; i++)
  is_valid_normal_cmd_loop:
    beq s4, s3, is_valid_normal_cmd_loop_end

    slli t0, s3, 2
    add t0, s1, t0
    lw t0, 0(t0) #command_strings[i]

    add t1, s2, s3
    lbu t1, 0(t1) #command_lengths[i]

    #strnmatch(command, command_str, command_len)
    mv a0, s0
    mv a1, t0
    mv a2, t1
    jal strnmatch
    mv t1, a0 #is_valid

    addi s3, s3, 1 #i++

    #if (!valid) -> continue
    beqz t1, is_valid_normal_cmd_loop
  is_valid_normal_cmd_loop_end:

  #restore S registers from the stack
  lw s0, 0(sp)
  lw s1, 4(sp)
  lw s2, 8(sp)
  lw s3, 12(sp)
  lw s4, 16(sp)
  addi sp, sp, 20

  lw ra, 0(sp)
  addi sp, sp, 4

  mv a0, t1 #return is_valid
  ret

#bool is_valid_parameterized_cmd(const char *command)
is_valid_parameterized_cmd:
  addi sp, sp, -4
  sw ra, 0(sp)

  #save S registers on the stack in order to make space
  addi sp, sp, -24
  sw s0, 0(sp)
  sw s1, 4(sp)
  sw s2, 8(sp)
  sw s3, 12(sp)
  sw s4, 16(sp)
  sw s5, 20(sp)

  mv s0, a0 #command

  #load statically compiled arrays (no stalls since each load is independent)
  la s1, command_strings_parameterized
  la s2, command_lengths_parameterized
  la s3, n_commands_parameterized

  lbu s3, 0(s3) #n_commands_parameterized

  li s4, 0 #i
  #for (uint8_t i = 0; i < n_commands; i++)
  is_valid_parameterized_cmd_loop:
    beq s4, s3, is_valid_parameterized_cmd_loop_end

    add s5, s2, s4
    lbu s5, 0(s5) # uint8_t command_len = command_lengths[i]

    slli t0, s4, 2
    add t0, s1, t0
    lw t0, 0(t0) #command_strings[i]

    #strnmatch(command, command_str, command_len)
    mv a0, s0
    mv a1, t0
    mv a2, s5
    jal strnmatch
    mv t1, a0 #is_valid

    addi s4, s4, 1 #i++

    #if (!is_valid) -> continue
    beqz t1, is_valid_parameterized_cmd_loop #continue 

    #return is_valid_args(&command[command_len]);
    add a0, s0, s5
    jal is_valid_args
    mv t1, a0 #is_valid
  is_valid_parameterized_cmd_loop_end:

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

  #return is_valid
  mv a0, t1
  ret

#bool is_valid_args(const char *args)
is_valid_args:
  lb t0, 0(a0) #args[0]
  lb t1, 1(a0) #args[1]
  li t2, 41 #closed parenthesis in ASCII

  #return (c != '\0') & (c != 41) & (args[1] == 41);
  snez t3, t0

  sub t5, t0, t2
  snez t5, t5

  sub t6, t1, t2
  seqz t6, t6

  and t3, t3, t5
  and t3, t3, t6

  mv a0, t3
  ret

#size_t m_strlen(const char *s)
strlen:
  li t0, 0 #size_t len = 0

  #while (*s != '\0')
  strlen_loop:
    lb t1, 0(a0) #c
    beqz t1, strlen_loop_end

    addi t0, t0, 1
    addi a0, a0, 1
    j strlen_loop
  strlen_loop_end:

  #return len
  mv a0, t0
  ret

#exit stays as far as possible, unlikely scenarios far from the hot path (instruction cache locality)
exit:
  li a7, 10
  ecall