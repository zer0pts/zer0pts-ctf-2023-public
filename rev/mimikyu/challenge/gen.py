import ctypes
import struct
from Crypto.Util.number import isPrime, inverse

def u32(a):
    if len(a) < 4:
        a += b'\0'*3
    return struct.unpack("<I", a)[0]

def next_prime(p):
    while not isPrime(p):
        p += 1
    return p

libc = ctypes.cdll.LoadLibrary("/usr/lib/x86_64-linux-gnu/libc.so.6")
libc.srand(0xfa1e0ff3)

flag = b"zer0pts{L00k_th3_1nt3rn4l_0f_l1br4r13s!}"

assert len(flag) % 4 == 0, f"len(flag): {len(flag)}"

e = 65537
out = []
for i in range(0, len(flag), 4):
    m = u32(flag[i:i+4])
    p = next_prime(libc.rand() % 0x10000)
    q = next_prime(libc.rand() % 0x10000)
    r = next_prime(libc.rand() % 0x10000)
    e = next_prime(libc.rand() % 0x10000)
    print(hex(m), hex(e), hex(p*q*r), hex(pow(m, e, p*q*r)))
    out.append(pow(m, e, p*q*r))

print(out)

for c in "%Zd\0":
    print(hex(ord(c) ^ 42))
