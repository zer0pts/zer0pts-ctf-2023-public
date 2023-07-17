import pickle
from typing import List
from ptrlib import remote
from tqdm import tqdm
from sage.all import *

load("pgcd.sage")

LOCAL = False

def get_conn():
    return remote("nc localhost 7002")

conn = get_conn()
a = int(conn.recvline().strip().split()[2])
b = int(conn.recvline().strip().split()[2])
e = int(conn.recvline().strip().split()[2])
n = int(conn.recvline().strip().split()[2])
conn.close()

print(f'[+] {a=}')
print(f'[+] {b=}')
print(f'[+] {e=}')
print(f'[+] {n=}')

G = Zmod(n)
F.<x> = PolynomialRing(G)

def reduce(polys: List[Polynomial], mat: Matrix, debug=False):
  n = len(polys)

  start_deg = polys[0].degree()
  for i in range(n):
    if polys[i].degree() < start_deg:
      polys[i] += polys[0]
  assert all([poly.degree() == start_deg for poly in polys])

  def reduce_poly(i, j):
    q = polys[i] // polys[j]
    assert q.degree() <= 1
    mat[i] -= q * mat[j]
    polys[i] -= q * polys[j]

  for divisor_ind in range(n-1):
    PROGRESS.update()
    for dividend_ind in range(n):
      if divisor_ind == dividend_ind: continue
      reduce_poly(dividend_ind, divisor_ind)

  reduce_poly(-2, -1)
  mat[-2] += mat[-1]
  polys[-2] += polys[-1]

  end_deg = polys[0].degree()
  if debug: print(f'{end_deg=} {[poly.degree() for poly in polys]=}')
  assert start_deg - end_deg == n - 1
  assert all([poly.degree() == end_deg for poly in polys])

  return mat

def bulk_gcd(polys: List):
  n = len(polys)
  
  if polys[0].degree() <= n + 1:
    return matrix.identity(F, n)

  m  = polys[0].degree() // 2

  M_1 = bulk_gcd([poly // x^m for poly in polys])
  V = M_1 * matrix(polys).T
  new_polys = list(V.T[0])

  if new_polys[0].degree() <= n + 1:
    return M_1
  
  reduce(new_polys, M_1)

  if new_polys[0].degree() <= n + 1:
    return M_1

  reduced_deg = polys[0].degree() - new_polys[0].degree()
  gained_pollution = ceil(reduced_deg / (n - 1))

  M_2 = bulk_gcd([poly // x^gained_pollution for poly in new_polys])
  return M_2 * M_1

from time import time

ROUND = 30

# GATHER_COUNT = 10 # precalc:  8 mins, gcd: 7.3 secs/round (recovering: 0.6 secs, half-gcd: 6.8 secs)
# GATHER_COUNT = 20 # precalc: 30 mins, gcd: 3.5 secs/round (recovering: 0.6 secs, half-gcd: 2.8 secs)
GATHER_COUNT = 40   # precalc: 98 mins, gcd: 2.1 secs/round (recovering: 0.6 secs, half-gcd: 1.4 secs)

prev_s = x
fs_prime = [prev_s^e]
while len(fs_prime) < GATHER_COUNT:
  prev_s = a * prev_s + b
  fs_prime.append(prev_s^e)

PICKLE_FILE = f"data_{e}_{GATHER_COUNT}.pickle"

if os.path.exists(PICKLE_FILE):
  data = pickle.load(open(PICKLE_FILE, "rb"))
  print("[+] data loaded from cache")
  assert data["a"] == a
  assert data["b"] == b
  assert data["e"] == e
  assert data["n"] == n
else:
  print("[+] calculating matrix...")
  start = time()
  PROGRESS = tqdm(total=int(e * (GATHER_COUNT - 1) // GATHER_COUNT), smoothing=0)
  M = bulk_gcd(fs_prime)
  PROGRESS.close()
  end = time()
  print(f"[+] calculation takes: {end - start:.3f}")
  data = {}
  data["a"] = a
  data["b"] = b
  data["e"] = e
  data["n"] = n
  data["M"] = [[list(poly) for poly in row] for row in M]
  pickle.dump(data, open(PICKLE_FILE, "wb"))
  print(f"[+] saved to {PICKLE_FILE}")

M = matrix(F, [[F(poly) for poly in row] for row in data["M"]])

print("[+] checking required degrees")
fs = [f - 1337 for f in fs_prime]
M2 = M[:2]
V2 = M2 * matrix(fs).T
required_degree = max(V2[0,0].degree(), V2[1,0].degree())+1

print(f'[+] {required_degree=}')

fs_reduced = [f[:required_degree+1] for f in fs_prime]

recovering_ts = []
half_gcd_ts = []

def solve_round(rs):
  fs = [f - r for f, r in zip(fs_reduced, rs)]
  start = time()
  V = matrix(fs).T
  V2 = M2 * V
  end = time()
  print(f' | recovering takes: {end - start:.3f} secs'); recovering_ts.append(end - start)
  u, v = V2[0, 0][:required_degree+1], V2[1, 0][:required_degree+1]
  print(f' | {u.degree()=} {v.degree()=}')
  start = time()
  res = pgcd(u, v).monic()
  end = time()
  print(f' \\ half-gcd takes: {end - start:.3f} secs'); half_gcd_ts.append(end - start)
  assert res.degree() == 1
  s = -res.constant_coefficient()
  assert pow(s, e, n) == rs[0]
  return s

conn = get_conn()
start = time()
def elapsed(): return round(time() - start, 3)

for i in range(GATHER_COUNT):
  conn.sendline(b'00')
encrypted_rs_list = []
for i in range(GATHER_COUNT):
  conn.recvuntil(b'> ')
  leaks = [0]
  for j in range(ROUND):
    leaks.append(int(conn.recvlineafter(": ")))
  encrypted_rs = [a ^^ b for a, b in zip(leaks[:-1], leaks[1:])]
  assert(len(encrypted_rs) == ROUND)
  encrypted_rs_list.append(encrypted_rs)

def next(s):
  return (a * s + b) % n

result_key = 0
prev_keys = [0] * GATHER_COUNT

for i in range(ROUND):
  print(f'[+] {elapsed()=} round {i+1} / {ROUND}...')
  rs = [encrypted_rs[i] ^^ prev_keys[j] for j, encrypted_rs in enumerate(encrypted_rs_list)]
  s = int(solve_round(rs))
  prev_keys = []
  while len(prev_keys) < GATHER_COUNT:
    assert pow(s, e, n) == rs[len(prev_keys)]
    prev_keys.append(s)
    s = next(s)
  result_key ^^= pow(s, e, n)
  result_key ^^= s

payload = int(result_key ^^ int.from_bytes(b"Give me the flag!", "big")).to_bytes(128, "big")

print(f'[+] {elapsed()=} send payload: {payload.hex()=}')
conn.sendlineafter("> ", payload.hex())

print(f'[+] {elapsed()=} all done! {elapsed() / ROUND:.3f} secs / round (recovering: {sum(recovering_ts) / ROUND:.2f} secs, half-gcd: {sum(half_gcd_ts) / ROUND:.2f} secs)')
print(conn.recvlineafter("The flag is: ", timeout=5).decode())

conn.close()
