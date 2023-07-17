import struct
import os

key = struct.pack('<Q', 0x1145141919810931)
print(key)
plain = b"zer0pts{d0n'7_4lw4y5_7ru57_d3c0mp1l3r}\n"
plain += b'\x00' * (0x80 - len(plain))

S = [i for i in range(0x100)]
j = 0
for i in range(0x100):
    j = (j + S[i] + key[i % 8]) % 0x100
    S[i], S[j] = S[j], S[i]

cipher = b''
i = j = 0
for n in range(0x80):
    i = (i + 1) % 0x100
    j = (j + S[i]) % 0x100
    S[i], S[j] = S[j], S[i]
    rnd = S[(S[i] + S[j]) % 0x100]
    cipher += bytes([rnd ^ plain[n]])

print(", ".join(map(hex, cipher)))

print(", ".join(map(hex, os.urandom(0x80))))
