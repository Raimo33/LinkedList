.data

head: .word 0x0
tail: .word 0x0
listInput: .string "ADD(1) ~ ADD(a) ~ ADD(a) ~ ADD(B) ~ ADD(;) ~ ADD(9) ~SORT~PRINT~DEL(b) ~DEL(B)~PRI~REV~PRINT"

.text

main:
  la a0, head_ptr
  la a1, tail_ptr
  la a2, listInput
  jal run
  j exit

//void run(t_node **head, t_node **tail, char *input)
run:
  addi sp, sp, -4
  sw ra, 0(sp)

  mv t0, a0 // head
  mv t1, a1 // tail
  mv t2, a2 // input

  mv a0, '~'
  jal tokenize
  mv t3, a1 // n_commands

  while:
    beqz t1, end_while

    //input += (input[0] == ' ')
    lb t4, 0(t2)
    xori t4, t4, ' '
    seqz t4, t4
    add t2, t2, t4

    //input += handle_operation(head, tail, input)
    mv a0, t0
    mv a1, t1
    mv a2, t2
    jal handle_operation
    add t2, t2, a1

    //input += (n_commands > 0)
    sltu t4, zero, t3
    add t2, t2, t4

    addi t1, t1, -1
    j while
  end_while:

  lw ra, 0(sp)
  addi sp, sp, 4
  jr ra

//uint8_t tokenize(char *input, const char sep)
tokenize:








//exit stays as far as possible, unlikely scenarios far from the hot path (instruction cache locality)
exit:
  li a7, 10
  ecall