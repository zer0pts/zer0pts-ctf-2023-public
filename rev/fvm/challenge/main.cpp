#include <sstream>
#include <iostream>
#include <stdexcept>

#define IMPORT_BIN(sect, file, sym) asm (       \
    ".section " #sect "\n"                      \
    ".balign 4\n"                               \
    ".global " #sym "\n"                        \
    #sym ":\n"                                  \
    ".incbin \"" file "\"\n"                    \
    ".global _sizeof_" #sym "\n"                \
    ".set _sizeof_" #sym ", . - " #sym "\n"     \
    ".balign 4\n"                               \
    ".section \".text\"\n")

IMPORT_BIN(".rodata", "code.bin", code);
extern const char code[];

enum X87_OP {
  __X87_LOAD_CONST = 0x20,
  X87_LD0, X87_LD1, X87_LDP, X87_LDL2T,
  X87_LDL2E, X87_LDLG2, X87_LDLN2,

  __X87_STACK = 0x30,
  X87_XCH1, X87_XCH2, X87_XCH3, X87_XCH4,
  X87_XCH5, X87_XCH6, X87_XCH7, X87_DUP,
  X87_PUSH, X87_POP,

  __X87_ARITHMETIC = 0x40,
  X87_ADD, X87_SUB, X87_MUL, X87_DIV,
  X87_NEG,

  __X87_FUNCTION = 0x50,
  X87_SQRT, X87_SIN, X87_COS, X87_RND,

  __X87_BRANCH = 0x60,
  X87_RET, X87_CALL, X87_JMP, X87_JEQ, X87_JNE,
  X87_JAE, X87_JA, X87_JBE, X87_JB,

  __X87_MISC = 0x70,
  X87_HLT, X87_GET, X87_PUT
};

class fvm {
public:
  fvm(const char* data, size_t size) {
    ifs.write(data, size);
    init_cpu();
  }

  void init_cpu() {
    __asm__("finit");
  }

  /*
  void dump_cpu() {
    double f;
    unsigned char st0[10];
    for (int i = 0; i < 8; i++) {
      __asm__("fst qword ptr [%0]": : "r"(&f));
      __asm__("fstp tbyte ptr [%0]": : "r"(&st0));
      printf("%lf : %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
             f, st0[0], st0[1], st0[2], st0[3], st0[4],
             st0[5], st0[6], st0[7], st0[8], st0[9]);
    }
  }
  //*/

  void emulate() {
    uint8_t op;
    int16_t pos;
    bool jmp, alive = true;
    ifs.seekg(0);

    while (alive) {
      ifs.read(reinterpret_cast<char*>(&op), 1);
      if (ifs.eof()) __asm__("int3");

      if (X87_RET < op && op < __X87_BRANCH + 0x10) {
        ifs.read(reinterpret_cast<char*>(&pos), 2);
        if (ifs.eof()) __asm__("int3");
      }

      switch (op) {
        case X87_LD0 : __asm__("fldz");  break; // Const
        case X87_LD1 : __asm__("fld1");  break;
        case X87_LDP : __asm__("fldpi"); break;
        case X87_LDL2T: __asm__("fldl2t"); break;
        case X87_LDL2E: __asm__("fldl2e"); break;
        case X87_LDLG2: __asm__("fldlg2"); break;
        case X87_LDLN2: __asm__("fldln2"); break;
        case X87_XCH1: __asm__("fxch");  break; // Stack
        case X87_XCH2: __asm__("fxch st(2)");  break;
        case X87_XCH3: __asm__("fxch st(3)");  break;
        case X87_XCH4: __asm__("fxch st(4)");  break;
        case X87_XCH5: __asm__("fxch st(5)");  break;
        case X87_XCH6: __asm__("fxch st(6)");  break;
        case X87_XCH7: __asm__("fxch st(7)");  break;
        case X87_DUP : __asm__("fst st(7); fdecstp"); break;
        case X87_PUSH: {
          uint8_t t[10];
          ifs.read(reinterpret_cast<char*>(t), 10);
          if (ifs.eof()) __asm__("int3");
          __asm__("fld tbyte ptr [%0]": : "r"(t) : "memory");
          break;
        }
        case X87_POP : __asm__("ffree st(0); fincstp"); break;
        case X87_ADD : __asm__("faddp"); break; // Arithmetic
        case X87_SUB : __asm__("fsubp"); break;
        case X87_MUL : __asm__("fmulp"); break;
        case X87_DIV : __asm__("fdivp"); break;
        case X87_NEG : __asm__("fchs"); break;
        case X87_RET : {
          uint64_t pc;
          __asm__("fistp qword ptr [%0]": : "r"(&pc) : "memory");
          ifs.seekg(pc, std::ios_base::beg);
          break;
        }
        case X87_CALL: { // Branch
          uint64_t pc = static_cast<uint64_t>(ifs.tellg());
          __asm__("fild qword ptr [%0]": : "r"(&pc) : "memory");
          // Emulate JMP
        }
        case X87_JMP :
          ifs.seekg(pos, std::ios_base::cur);
          break;
        case X87_JEQ :
          __asm__("xor edx, edx; fcomp; fstsw ax; sahf;"
                  "sete dl; mov %0, dl;"
                  :"=r"(jmp)::"eax","edx","memory");
          if (jmp) ifs.seekg(pos, std::ios_base::cur);
          break;
        case X87_JNE :
          __asm__("xor edx, edx; fcomp; fstsw ax; sahf;"
                  "setne dl; mov %0, dl;"
                  :"=r"(jmp)::"eax","edx","memory");
          if (jmp) ifs.seekg(pos, std::ios_base::cur);
          break;
        case X87_JAE :
          __asm__("xor edx, edx; fcomp; fstsw ax; sahf;"
                  "setae dl; mov %0, dl;"
                  :"=r"(jmp)::"eax","edx","memory");
          if (jmp) ifs.seekg(pos, std::ios_base::cur);
          break;
        case X87_JA  :
          __asm__("xor edx, edx; fcomp; fstsw ax; sahf;"
                  "seta dl; mov %0, dl;"
                  :"=r"(jmp)::"eax","edx","memory");
          if (jmp) ifs.seekg(pos, std::ios_base::cur);
          break;
        case X87_JBE :
          __asm__("xor edx, edx; fcomp; fstsw ax; sahf;"
                  "setbe dl; mov %0, dl;"
                  :"=r"(jmp)::"eax","edx","memory");
          if (jmp) ifs.seekg(pos, std::ios_base::cur);
          break;
        case X87_JB  :
          __asm__("xor edx, edx; fcomp; fstsw ax; sahf;"
                  "setb dl; mov %0, dl;"
                  :"=r"(jmp)::"eax","edx","memory");
          if (jmp) ifs.seekg(pos, std::ios_base::cur);
          break;
        case X87_SQRT: __asm__("fsqrt"); break; // Functions
        case X87_SIN: __asm__("fsin"); break;
        case X87_COS: __asm__("fcos"); break;
        case X87_RND: __asm__("frndint"); break;
        case X87_HLT : alive = false; break; // Misc
        case X87_GET : {
          uint64_t cc;
          char c;
          std::cin.get(c);
          if (!std::cin.good()) c = -1;
          cc = static_cast<uint64_t>(c);
          __asm__("fild qword ptr [%0]": : "r"(&cc) : "memory");
          break;
        }
        case X87_PUT : {
          uint64_t c;
          __asm__("fistp qword ptr [%0]": : "r"(&c) : "memory");
          std::cout << static_cast<char>(c);
          break;
        }
        default: __asm__("ud2"); break;
      }
    }
    //dump_cpu();
  }

private:
  std::stringstream ifs;
};

int main() {
  fvm vm(code, 635);
  vm.emulate();
  return 0;
}

