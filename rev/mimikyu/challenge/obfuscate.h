#include <assert.h>
#include <link.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <stddef.h>

typedef void* HMODULE;
typedef const char* LPCSTR;
typedef int BOOL;
typedef struct link_map* MODULEINFO;
typedef MODULEINFO* LPMODULEINFO;

HMODULE LoadLibraryA(LPCSTR lpModuleName) {
  return dlopen(lpModuleName, RTLD_LAZY | RTLD_GLOBAL);
}

BOOL CloseHandle(HMODULE hModule) {
  dlclose(hModule);
  return 1;
}

BOOL GetModuleInformation(HMODULE hModule, LPMODULEINFO lpmodinfo) {
  return dlinfo(hModule, RTLD_DI_LINKMAP, lpmodinfo) == 0;
}

uint32_t CryptGetHashParam(LPCSTR s) {
  uint32_t len = 0, rem, hash = 0;
  for (const char *p = s; *p; p++) {
    hash++;
    len++;
  }
  rem = len & 3;

  for (len >>= 2; len > 0; len--) {
    hash += *(const uint16_t*)s;
    hash = (hash << 16) ^ (((*(const uint16_t*)(s + 2)) << 11) ^ hash);
    s += 2 * sizeof(uint16_t);
    hash += hash >> 11;
  }

  switch (rem) {
    case 3:
      hash += *(const uint16_t*)s;
      hash ^= hash << 16;
      hash ^= s[sizeof(uint16_t)] << 18;
      hash += hash >> 11;
      break;
    case 2:
      hash += *(const uint16_t*)s;
      hash ^= hash << 11;
      hash += hash >> 17;
      break;
    case 1:
      hash += *s;
      hash ^= hash << 10;
      hash += hash >> 1;
      break;
  }

  hash ^= hash << 3;
  hash += hash >> 5;
  hash ^= hash << 4;
  hash += hash >> 17;
  hash ^= hash << 25;
  hash += hash >> 6;
  return hash;
}

/*
#include <stdio.h>
void check(HMODULE hModule) {
  MODULEINFO lpmodinfo = NULL;
  int moduleSymbolEntries;
  Elf64_Sym *moduleSymbolTable;
  char *moduleStringTable;
  assert (GetModuleInformation(hModule, &lpmodinfo));
  for (ElfW(Dyn) *s = lpmodinfo->l_ld; s->d_tag != DT_NULL; s++) {
    switch (s->d_tag) {
      case DT_SYMTAB:
        moduleSymbolTable = (Elf64_Sym*)s->d_un.d_ptr;
        break;
      case DT_STRTAB:
        moduleStringTable = (char*)s->d_un.d_ptr;
        break;
      case DT_SYMENT:
        moduleSymbolEntries = s->d_un.d_val;
        break;
    }
  }

  int size = moduleStringTable - (char*)moduleSymbolTable;
  for (int k = 0; k < size / moduleSymbolEntries; k++) {
    Elf64_Sym *sym = &moduleSymbolTable[k];
    if (ELF64_ST_TYPE(moduleSymbolTable[k].st_info) == STT_FUNC) {
      char *str = &moduleStringTable[sym->st_name];
      printf("0x%08x %s\n", (unsigned int)CryptGetHashParam(str), str);
    }
  }
}
*/

typedef void* T;
uint64_t ResolveModuleFunction(HMODULE hModule, uint32_t sig, ...) {
  MODULEINFO lpmodinfo = NULL;
  int moduleSymbolEntries;
  Elf64_Sym *moduleSymbolTable;
  char *moduleStringTable;
  uint64_t ret = 0;
  void *args[6];
  va_list arg;
  va_start(arg, sig);

  assert (GetModuleInformation(hModule, &lpmodinfo));
  for (ElfW(Dyn) *s = lpmodinfo->l_ld; s->d_tag != DT_NULL; s++) {
    switch (s->d_tag) {
      case DT_SYMTAB:
        moduleSymbolTable = (Elf64_Sym*)s->d_un.d_ptr;
        break;
      case DT_STRTAB:
        moduleStringTable = (char*)s->d_un.d_ptr;
        break;
      case DT_SYMENT:
        moduleSymbolEntries = s->d_un.d_val;
        break;
    }
  }

  dlerror();
  int size = moduleStringTable - (char*)moduleSymbolTable;
  for (int k = 0; k < size / moduleSymbolEntries; k++) {
    Elf64_Sym *sym = &moduleSymbolTable[k];
    if (ELF64_ST_TYPE(moduleSymbolTable[k].st_info) == STT_FUNC) {
      char *str = &moduleStringTable[sym->st_name];
      if (CryptGetHashParam(str) == sig) {
        uint64_t (*f)(T, T, T, T, T, T) = dlsym(hModule, str);
        if (dlerror())
          __builtin_trap();
        for (int i = 0; i < 6; i++) {
          args[i] = va_arg(arg, void*);
        }
        ret = f(args[0], args[1], args[2], args[3], args[4], args[5]);
        break;
      }
    }
  }

  va_end(arg);
  return ret;
}
