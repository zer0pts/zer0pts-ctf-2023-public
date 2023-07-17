import math
from pwn import *

def S(z):
	return -1/z

def T(z):
	return z + 1

def U(z):
	return z - 1

def gen_random_hash(n):
	r = bytes([getrandbits(8) for _ in range(0, n)])
	return r

def to_hash(st):
	res = ""
	for s in st:
		sts = bin(s)[2:].zfill(8)
		for x in sts:
			if x == "0":
				res += "S"
			else:
				res += "T"
	return res

def hash_inv(st):
	res = ""
	for s in st:
		if s == "S":
			res = "S" + res
		else:
			res = "U" + res
	res = res.replace("U", "STSTS").replace("SS", "").replace("TSTSTS", "")
	return res

def hash(z, h):
	res = z
	for s in h:
		if s == "S":
			res = S(res)
		elif s == "T":
			res = T(res)
		elif s == "U":
			res = U(res)
		else:
			exit()
	return res

def hash_eq(h1, h2, CC):
	for _ in range(100):
		zr = CC.random_element()
		h1zr = hash(zr, h1)
		h2zr = hash(zr, h2)
		print(f"abs: {abs(h1zr - h2zr)}")
		if abs(h1zr - h2zr) > 1e-15:
			return False
	return True

def in_fundamental(z):
	if -0.5 <= z.real() and z.real() <= 0.5 and abs(z) >= 1:
		return True
	return False

def gen_to_fundamental_hash(z):
	res = ""
	while True:
		while True:
			if z.real() > 0.5:
				res += "U"
				z = U(z)
			elif z.real() < -0.5:
				res += "T"
				z = T(z)
			else:
				break
		if abs(z) < 1:
			res += "S"
			z = S(z)
		if in_fundamental(z):
			break
	res = res.replace("U", "STSTS").replace("SS", "").replace("TSTSTS", "")
	return res

p = process(["sage", "./server.sage"])

CC = ComplexField(256)
for _ in range(100):
	zi = CC(eval(p.recvline().decode("utf-8").strip().split(": ")[1]))
	h1zi = CC(eval(p.recvline().decode("utf-8").strip().split(": ")[1]))
	p.recvuntil("your hash> ")

	print("zi:", zi)
	print("h1zi:", h1zi)

	hz = gen_to_fundamental_hash(zi)
	hh1zi = gen_to_fundamental_hash(h1zi)
	hh1zi_inv = hash_inv(hh1zi)

	# print("hz:", hz)
	# print("hh1zi_inv:", hh1zi_inv)

	res = hz + hh1zi_inv

	p.sendline(res.encode("utf-8"))

p.interactive()
