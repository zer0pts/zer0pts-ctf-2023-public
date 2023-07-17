from ptrlib import *
import string
import struct
import random

flag = b"zer0pts{kMo7UtDhqMfXhaUp0kP8MEPLPJFgKUx7YlWyyxB9POKUhegFqdNm5sXIfxk2FIfV}"
NUM = 100

def randstr(length=8):
    s = random.choice(string.ascii_letters)
    s += ''.join(random.choices(table_alphanumeric, k=length-1))
    return s

def encode(s):
    code = 'unsigned long v = *(unsigned long*)flag;\n'
    v = u64(s)
    n = random.randint(1, 200)
    ss = [random.randint(0, 63) for i in range(n)]
    ks = [random.randint(0, 0xffff_ffff_ffff_ffff) for i in range(n)]
    for s, k in zip(ss, ks):
        ope = random.randint(0, 3)
        if ope == 0:
            # rotate
            if s < 32:
                v = ror(v, s, bits=64) ^ k
                code += f'v = ror(v, {s}) ^ {k}ULL;\n'
            else:
                v = rol(v, 64 - s, bits=64) ^ k
                code += f'v = rol(v, {64 - s}) ^ {k}ULL;\n'

        elif ope == 1:
            # addsub
            if random.randint(0, 1) == 0:
                v = (v + k) % 0x1_0000_0000_0000_0000
                code += f'v = v + {k}ULL;\n'
            else:
                v = (v - k) % 0x1_0000_0000_0000_0000
                code += f'v = v - {k}ULL;\n'

        elif ope == 2:
            # xor
            v = v ^ k
            code += f'v = v ^ {k}ULL;\n'

        elif ope == 3:
            # bswap
            v = struct.unpack('<Q', struct.pack('>Q', v))[0]
            code += f'v = bswap_64(v);\n'

    code += f'return v != {v}ULL;'
    return code

FUNCS = ""
REFS = []

ans = [
    random.sample(range(NUM-1), k=random.randint(5, 50))
    for _ in range(0, len(flag), 8)
]

for whoami in range(NUM-1):
    func_name = randstr()
    REFS.append(func_name)
    cases = []

    for i in range(0, len(flag), 8):
        if whoami in ans[i//8]:
            cases.append(encode(flag[i:i+8]))
        else:
            cases.append(encode(randstr(8).encode()))

    func = f"""
int {func_name}(const char* flag) {{
  static int {func_name} = 0;
  switch ({func_name}++) {{
"""
    for i, case in enumerate(cases):
        func += f"""
    case {i}: {{
      {case}
    }}
    """
    func += """
    default:
      return 1;
  }
}
    """
    FUNCS += func

code = open("template.c").read()
code = code.replace("#####NUM#####", str(NUM))
code = code.replace("#####FUNCS#####", str(FUNCS))
code = code.replace("#####REFS#####", ','.join(REFS))

#print(FUNCS)
print(code)
