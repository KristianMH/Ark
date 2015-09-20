bne $t5,$t4,L1


L1:	addu 	$sp, $sp, -4
	addiu	$v0, $t5, 0
	addu 	$sp, $sp, 4
syscall

	