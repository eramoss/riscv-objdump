    add x5, x6, x7
    addi x8, x5, 10
    sw x8, 0(x10)
    beq x5, x8, branch_target
    lui x9, 0x12345
    jal x1, jump_target
    ecall
    
   
branch_target:
   

jump_target:
    