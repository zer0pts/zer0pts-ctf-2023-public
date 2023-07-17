from pwn import *
from itertools import combinations

r = process(["python", "server.py"])

r.recvline()
N = ZZ[I](r.recvline()[3:])

print(N)

facs = list(factor(N))
for i in range(len(facs)):
	facs[i] = facs[i][0]
print(facs)

prods = []
for i in range(1, len(facs) + 1):
	for j in combinations(facs, i):
		mul = prod(j)
		if is_prime(ZZ(abs(mul[0]))) and is_prime(ZZ(abs(mul[1]))) and mul.norm() == ZZ(N):
			print(mul)
			r.sendline(str(abs(mul[0])).encode("utf-8"))
			r.sendline(str(abs(mul[1])).encode("utf-8"))
			r.interactive()

