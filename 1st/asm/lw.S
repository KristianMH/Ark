addiu	$sp, $sp, -4
sw 	$t5,0($sp)
lw	$v0,0($sp)
addiu 	$sp, $sp,4
syscall
	