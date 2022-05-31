// #include <math.h>
#include <stdio.h>

#include "main.h"
int main() {
  asm("int $0x80");

  return 0;
}