import re
from math import sin, cos, pi
from ptrlib import *

def to_float(buf):
    sock = Process("./t2f")
    sock.send(buf)
    val = float(sock.recvline())
    sock.close()
    return val

def restore(mul, add):
    print(mul, add)
    sock = Process("./restore")
    sock.sendline(str(mul))
    sock.sendline(str(add))
    blk = sock.recvlineafter("FOUND: ").decode()
    sock.close()
    return blk

flag = ''
with open("disasm.txt", "r") as f:
    i = 0
    mul, add = 0, 0
    for line in f:
        if 'PUSH' in line:
            r = re.findall("0x([0-9a-f]+)", line)
            buf = int.to_bytes(int(r[0], 16), 10, 'little')
            val = to_float(buf)
            if i % 2 == 0:
                mul = val
            else:
                add = val
                flag += restore(mul, add)
                print(flag)
            i += 1

print("-"*20)
print(flag + "}")
print("-"*20)
