addu 	$sp, $sp, -4
sll 	$v0, $t5, 2
addu 	$sp, $sp, 4
syscall

	
#Forventet resultat --> v0 = 0xa8