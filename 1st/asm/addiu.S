addiu 	$sp, $sp, -4
addiu $v0, $t5, 20
addiu 	$sp, $sp, 4
syscall
	
#Forventet resultat --> v0 = 0x3e