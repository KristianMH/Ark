fib:    addi $sp, $sp, -8
        sw   $ra, 4($sp)
        sw   $a0, 0($sp)
        beq  $a0, $zero, L1
        addi $t0, $zero, 1
        beq  $t0, $a0, L2

        
        
L1:     addi $v0, $zero, 0
        addi $sp, $sp, 8
        jr   $ra

L2:     addi $v1, $zero, 1
        addi $sp, sp, 8
        jr   $ra

L3:     
        
