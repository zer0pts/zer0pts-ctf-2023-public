from ptrlib import *
import os

HOST = os.getenv("HOST", "localhost")
PORT = int(os.getenv("PORT", 10555))

ROUND = 77

#sock = Process(["python", "server.py"], cwd="../distfiles")
sock = Socket(HOST, PORT)

sock.sendlineafter("seed 1: ",  1)
sock.sendlineafter("seed 2: ", -1)

for _ in range(ROUND):
    rnd1 = int(sock.recvlineafter("Random 1: "), 16)
    rnd2 = int(sock.recvlineafter("Random 2: "), 16)

    # Lx1 = kronecker(a + 1, p)
    Lx1 = 1 if ((rnd1 >> 31) & 1) == 1 else -1
    # Lx2 = kronecker(a^31 - a^30 + a^29 + ... - a + 1, p)
    Lx2 = 1 if (rnd2 & 1) == 1 else -1

    print(f"{Lx1} x {Lx2} --> {Lx1 * Lx2}")
    if Lx1 * Lx2 == 1:
        sock.sendlineafter(": ", 1)
    else:
        sock.sendlineafter(": ", 0)

sock.sh()
