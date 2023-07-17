import struct
import os

key = struct.pack('<Q', 0x1145141919810931)
with open("../distfiles/chall", "rb") as f:
    f.seek(0x2f80)
    cipher = f.read(0x80)

S = [i for i in range(0x100)]
j = 0
for i in range(0x100):
    j = (j + S[i] + key[i % 8]) % 0x100
    S[i], S[j] = S[j], S[i]

plain = b''
i = j = 0
for n in range(0x80):
    i = (i + 1) % 0x100
    j = (j + S[i]) % 0x100
    S[i], S[j] = S[j], S[i]
    rnd = S[(S[i] + S[j]) % 0x100]
    plain += bytes([rnd ^ cipher[n]])

print(plain.rstrip(b"\0"))
