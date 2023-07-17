#define _GNU_SOURCE
#include "obfuscate.h"
#include <gmp.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <search.h>

uint64_t encoded[] = {
  17475150661108, 30207309348490, 1665127590720, 13125369062874,
  2851045040721, 16165950837940, 1074023774728, 188395652722,
  104158599375430, 6320504950662
};

#define R ResolveModuleFunction

#define isprint  0x4e8a031a
#define putchar  0x00d588a9
#define srand    0xfc7e7318
#define rand     0x7b6cea5d
#define hcreate  0xe75e0ffe
#define hsearch  0x50ab4097
#define hdestroy 0xaf4c09bd
#define setbuf   0x9419a860
#define memfrob  0x1c46d38a

#define __gmpz_init   0x71b5428d
#define __gmpz_clear  0x31cc4f9f
#define __gmpz_set_ui 0xf122f362
#define __gmpz_mul    0x347d865b
#define __gmpz_powm   0x9023667e
#define __gmpz_add_ui 0xed3b7a10
#define __gmpz_sub_ui 0x1c3ef940
#define __gmp_sprintf 0x7489af98
#define __gmpz_cmp_ui 0xb1f820dc

/**
 * Find the next prime given an integer p
 */
void cap(HMODULE hLibc, HMODULE hGMP, uint64_t nel, mpz_t *p) {
  char fmt[4] = {0x0f, 0x70, 0x4e, 0x2a};
  char key[0x10];
  ENTRY item = { .key = key, .data = NULL };

  // Capacity of hcreate is always prime
  // https://elixir.bootlin.com/glibc/glibc-2.37.9000/source/misc/hsearch_r.c#L80
  R(hGMP, __gmpz_set_ui, p, 0);
  R(hLibc, hcreate, nel);
  R(hLibc, memfrob, fmt, 4);
  do {
    R(hGMP, __gmp_sprintf, key, fmt, p);
    R(hGMP, __gmpz_add_ui, p, p, 1);
  } while (R(hLibc, hsearch, item, ENTER));
  R(hLibc, hdestroy);
  R(hGMP, __gmpz_sub_ui, p, p, 1);
}

int main(int argc, char **argv) {
  char *flag;
  size_t len;
  mpz_t p, n, e;

  if (argc < 2) {
    printf("Usage: %s FLAG\n", argv[0]);
    return 1;
  } else {
    flag = argv[1];
    len = strlen(flag);
  }
  if (len != sizeof(encoded) / sizeof(encoded[0]) * 4)
    goto NG1;

  HMODULE hLibc = LoadLibraryA("libc.so.6");
  assert (hLibc != NULL);
  HMODULE hGMP = LoadLibraryA("libgmp.so");
  assert (hGMP != NULL);

  R(hGMP, __gmpz_init, &p);
  R(hGMP, __gmpz_init, &n);
  R(hGMP, __gmpz_init, &e);

  R(hLibc, srand, *(unsigned int*)main);
  R(hLibc, setbuf, stdout, NULL);

  printf("Checking...");

  for (size_t i = 0; i < len; i++) {
    if ((int)R(hLibc, isprint, flag[i]) == 0)
      goto NG2;
  }

  for(size_t i = 0; i < len; i+=4) {
    R(hGMP, __gmpz_set_ui, &n, 1);
    for (size_t i = 0; i < 3; i++) {
      R(hLibc, putchar, '.');
      cap(hLibc, hGMP, (int)R(hLibc, rand) % 0x10000, &p);
      R(hGMP, __gmpz_mul, n, n, p);
    }
    R(hLibc, putchar, '.');
    cap(hLibc, hGMP, (int)R(hLibc, rand) % 0x10000, &e);

    R(hGMP, __gmpz_set_ui, &p, *(unsigned int*)(flag + i));
    R(hGMP, __gmpz_powm, &p, &p, &e, &n);
    if ((int)R(hGMP, __gmpz_cmp_ui, &p, encoded[i/4]))
      goto NG2;
  }

  puts("\nCorrect!");
  goto cleanup1;

 NG1:
  puts("Nowhere near close.");
  return 0;

 NG2:
  puts("\nWrong.");

 cleanup1:
  R(hGMP, __gmpz_clear, &p);
  R(hGMP, __gmpz_clear, &n);
  R(hGMP, __gmpz_clear, &e);
  CloseHandle(hLibc);
  CloseHandle(hGMP);
  return 0;
}
