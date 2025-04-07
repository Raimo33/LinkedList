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
//TODO exit

run:
  addi sp, sp, -4
  sw ra, 0(sp)

  jal tokenize_input

  lw ra, 0(sp)
  addi sp, sp, 4
  jr ra