from pwn import *
import re
import sys
from Crypto.Util.number import long_to_bytes

sys.setrecursionlimit(30000)

def hash(b):
	return hashlib.sha512(str(b).encode("utf-8")).digest()

p = process(["sage", "./server.sage"])

p.sendline(b"50")

log.info(p.recvuntil(b": "))

n = int(p.recvline().strip())
log.info(f"n: {n}")

Bn = BraidGroup(n)
gs = Bn.gens()

def braid_eval(x):
	res = gs[0] / gs[0]
	x = x.split("*")
	for i in x:
		res *= sage_eval(i, locals={"gs": gs})
	return res

log.info(p.recvuntil(b": "))
u = p.recvuntil(b": ")[:-3].strip()
u = re.sub("s([0-9]+)", "gs[\\1]", u.decode())
u = sage_eval(u, locals={"gs": gs})
log.info(f"u: {u}")

v = p.recvuntil(b": ")[:-3].strip()
v = re.sub("\\\\\n", "", v.decode())
v = re.sub("s([0-9]+)", "gs[\\1]", v)
log.info(f"{len(v)}")
v = sage_eval(v, locals={"gs": gs})
log.info(f"v: {v}")

w = p.recvuntil(b": ")[:-3].strip()
w = re.sub("\\\\\n", "", w.decode())
w = re.sub("s([0-9]+)", "gs[\\1]", w)
w = sage_eval(w, locals={"gs": gs})
# log.info(f"w: {w}")

d = int(p.recvline().strip())
log.info(f"d: {d}")

factors = []
for i in u.Tietze():
	factors.append(i - 1)
log.info(f"factors: {factors}")

x1 = gs[0] / gs[0]
x2 = gs[0] / gs[0]
cnt = 0
for i in factors:
	if i < (n // 2) - 1:
		x1 *= gs[i]
	elif i >= (n // 2) + 1:
		x2 *= gs[i]
	else:
		cnt += 1
z = gs[n // 2 - 1]^cnt
print(x1)
print(x2)
print(z)
print(x1 * x2 * z == u)

r1 = v * z^-1 * x2^-1
r2 = x1^-1 * w * z^-1
r = r1 * r2 * z
r = prod(r.right_normal_form())
r = hash(r)

dd = list(long_to_bytes(d).decode())
xx = []
for i in range(len(r)):
	xx.append(chr(ord(dd[i]) ^^ r[i]))
flag = "".join(xx).encode("utf-8")
log.info(f"flag: {flag}")

p.interactive()
