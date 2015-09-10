
fib:    addi $sp, $sp, -8
        sw   $ra, 4($sp)
        sw   $a0, 0($sp)    
        beq  $a0, $zero, L1
        addi $t0, $zero, 1
        beq  $t0, $a0, L2
	nop 
        addi $a0, $a0, -1
        jal  fib
        add  $t1, $zero, $v0
        addi $a0, $a0, -1
        jal  fib 
        add  $t2, $zero, $v0
        addi $sp, $sp, 8
        add  $v0, $t1, $t2
        jr $ra
        
        
L1:     addi $v0, $v0, 0
        addi $sp, $sp, 8
        jr   $ra

L2:     addi $v0, $v0, 1
        addi $sp, $sp, 8
        jr   $ra     
