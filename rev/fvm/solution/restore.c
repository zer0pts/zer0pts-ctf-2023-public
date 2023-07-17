// $ gcc restore.c -O4 -o restore -lm
#include <stdio.h>
#include <math.h>
#define EPSILON 0.0001

double cycloid_x(double a, double b) {
  double theta = 2 * M_PI * b / 0x100;
  return a * (theta - sin(theta));
}

double cardioid_y(double a, double b) {
  double theta = 2 * M_PI * b / 0x100;
  return a * (1 + cos(theta)) * sin(theta);
}

int simeq(double a, double b) {
  return fabs(a - b) < EPSILON;
}

int main() {
  double mul, add;
  if (scanf("%lf", &mul) != 1) return 1;
  if (scanf("%lf", &add) != 1) return 1;
  for (int c1 = 0x20; c1 < 0x7f; c1++) {
    for (int c2 = 0x20; c2 < 0x7f; c2++) {
      for (int c3 = 0x20; c3 < 0x7f; c3++) {
        for (int c4 = 0x20; c4 < 0x7f; c4++) {
          double a = cycloid_x(c1, c2);
          double b = cardioid_y(c3, c4);
          if (simeq(a*b, mul) && simeq(a+b, add)) {
            printf("FOUND: %c%c%c%c\n", c1, c2, c3, c4);
          }
        }
      }
    }
  }
  return 0;
}
