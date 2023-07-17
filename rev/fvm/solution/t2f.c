// $ gcc t2f.c -masm=intel -o t2f
#include <stdio.h>
#include <unistd.h>

// Convert tbytes to float64
int main() {
  double val;
  char buf[10];
  if (read(0, buf, 10) != 10) return 1;
  __asm__("fld tbyte ptr [%0];"
          "fstp qword ptr [%1];"
          : : "r"(buf), "r"(&val) : "memory");
  printf("%lf\n", val);
  return 0;
}

