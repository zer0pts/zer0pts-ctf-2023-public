import ctypes
import struct
from Crypto.Util.number import isPrime, inverse

def next_prime(p):
    while not isPrime(p):
        p += 1
    return p

libc = ctypes.cdll.LoadLibrary("/usr/lib/x86_64-linux-gnu/libc.so.6")
libc.srand(0xfa1e0ff3)

cs = []
with open("../distfiles/mimikyu", "rb") as f:
    f.seek(0x3020)
    for i in range(10):
        cs.append(struct.unpack("<Q", f.read(8))[0])

flag = b''
for c in cs:
    p = next_prime(libc.rand() % 0x10000)
    q = next_prime(libc.rand() % 0x10000)
    r = next_prime(libc.rand() % 0x10000)
    e = next_prime(libc.rand() % 0x10000)
    d = inverse(e, (p-1)*(q-1)*(r-1))
    m = pow(c, d, p*q*r)
    flag += int.to_bytes(m, 4, 'little')

print(flag)
