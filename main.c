// Kode skrevet til forelæsningen i ARK den 1. september 2015
//
// Første C program
//
// Forfatter: Oleksandr Shturmov (Oleks) <oleks@oleks.info>
//
// Kørselsvejledning:
// $ gcc main.c
// $ ./a.out
// ...
// $ echo $?
// 42
// $

// As in all human activities, progress in C is driven by many factors,
// corporate or individual interest, politics, beauty, logic, luck, ignorance,
// selfishness, ego, sectarianism, [...] Thus the development of C has not been
// and cannot be ideal. It has flaws and artifacts that can only be understood
// with their historic and societal context.
//
// Jens Gustedt, INRIA. Modern C. Preliminary version as of March 30, 2015.
// http://icube-icps.unistra.fr/index.php/File:ModernC.pdf

#include <stdio.h>

int main() {
  int x = 5;
  char name[32];

  printf("Hej!\n");

  printf("Skriv dit navn: ");
  // gets(name); // Brug ALDRIG gets. Brug fgets i stedet:
  fgets(name, 32, stdin);
  printf("Hej %s!\n", name);
  printf("Her er et tal: %x\n", x);
  return 42;
}
