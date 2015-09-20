addu 	$sp, $sp, -4
slt 	$v0, $t4, $t5
addu 	$sp, $sp, 4
syscall

#Forventet resultat --> v0 = 0x1