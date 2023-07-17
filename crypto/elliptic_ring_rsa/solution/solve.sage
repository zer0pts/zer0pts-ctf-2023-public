import string
import random

class EllipticRingElement:
	point = None
	def __init__(self, point):
		self.point = point
	
	def __add__(self, other):
		if self.point == dict():
			return other
		if other.point == dict():
			return self
		res = self.point.copy()
		for k in other.point.keys():
			if k in res:
				res[k] += other.point[k]
				if res[k] == 0:
					res.pop(k)
			else:
				res[k] = other.point[k]
				if res[k] == 0:
					res.pop(k)
		return EllipticRingElement(res)
	
	def __mul__(self, other):
		if self.point == dict() or other.point == dict():
			return self.point()
		res = dict()
		for k1 in other.point.keys():
			for k2 in self.point.keys():
				E = k1 + k2
				k = other.point[k1] * self.point[k2]
				if E in res:
					res[E] += k
					if res[E] == 0:
						res.pop(E)
				else:
					res[E] = k
					if res[E] == 0:
						res.pop(E)
		return EllipticRingElement(res)
	
	def __repr__(self):
		st = ""
		for k in self.point.keys():
			st += f"{self.point[k]}*({k[0]}, {k[1]}) + "
		return st[:-3]
	
class EllipticRing:
	E = None
	Base = None
	def __init__(self, E):
		self.E = E
		self.Base = E.base()

	def __call__(self, pt):
		for P in pt:
			pt[P] = self.Base(pt[P])
		return EllipticRingElement(pt)
	
	def zero(self):
		return EllipticRingElement(dict())
	
	def one(self):
		return EllipticRingElement({E(0): self.Base(1)})
	
	def pow(self, x, n):
		res = self.one()
		while n:
			if (n & 1):
				res = res * x
			x = x * x
			n >>= 1
		return res
	
	def encode(self, m, length):
		left = random.randint(0, length)
		pad1 = "".join(random.choices(string.ascii_letters, k=left)).encode("utf-8")
		pad2 = "".join(random.choices(string.ascii_letters, k=length+len(m)-left)).encode("utf-8")
		m = pad1 + m + pad2

		Ps = []
		while len(Ps) < length:
			PP = self.E.random_element()
			if PP not in Ps:
				Ps.append(PP)
		Ps = sorted(Ps)

		M = dict()
		for coef, pt in zip(m, Ps):
			M[pt] = self.Base(coef)
		return EllipticRingElement(M)


##################################################################### solve part
with open("output.txt") as f:
	p = ZZ(f.readline()[3:])
	Fp = GF(p)

	C = f.readline()[3:]
	a = Fp(f.readline()[3:])
	b = Fp(f.readline()[3:])
	e = ZZ(f.readline()[3:])

E = EllipticCurve(Fp, [a, b])
ER = EllipticRing(E)

terms = C.split(" + ")
C = dict()
for i in range(len(terms)):
	term = terms[i].split("*")
	coef = Fp(term[0])
	pt = eval(term[1])
	x = Fp(pt[0])
	y = Fp(pt[1])
	if x == 0 and y == 1:
		pt = E(0, 1, 0)
	else:
		pt = E(x, y)
	C[pt] = coef
C = ER(C)

o = E.order()
g = E.gens()[0]
print("gens:", E.gens())
print("g: ", g)
print("o: ", o)
print()

rationals = [E(0), g]
for i in range(o - 2):
	rationals.append(rationals[-1] + g)

Px.<x> = PolynomialRing(Fp)

def point_to_polynomial(point, rationals):
	f = Px(0)
	for k in point.keys():
		f += point[k] * x^rationals.index(k)
	return f

def polynomial_to_point(f, rationals):
	d = dict()
	cnt = 0
	for i in f:
		if i != 0:
			d[rationals[cnt]] = i
		cnt += 1
	return ER(d)

f = point_to_polynomial(C.point, rationals)

fi = x^o - 1

factors = list(factor(fi))
for i in range(len(factors)):
	factors[i] = factors[i][0]

ms = []
fmods = []
for fmod in factors:
	n = fmod.degree()
	s = p^n - 1
	d = inverse_mod(e, s)
	m = pow(f, d, fmod)
	ms.append(m)
	fmods.append(fmod)

M = crt(ms, fmods)

M = polynomial_to_point(M, rationals)
print("M:", M)
print()


## decode
points = M.point
spoints = sorted(points.items())
for _, coef in spoints:
	print(chr(coef), end="")
print()
