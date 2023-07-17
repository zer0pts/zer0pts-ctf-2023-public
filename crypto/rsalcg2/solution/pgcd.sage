MIN = 10

def pdivmod(u, v):
    """
    polynomial version of divmod
    """
    q = u // v
    r = u - q*v
    return (q, r)

def hgcd(u, v, depth=0, min_degree=MIN):
    """
    Calculate Half-GCD of (u, v)
    f and g are univariate polynomial
    http://web.cs.iastate.edu/~cs577/handouts/polydivide.pdf
    """
    x = u.parent().gen()

    if u.degree() < v.degree():
        u, v = v, u

    if 2*v.degree() < u.degree() or u.degree() < min_degree:
        q = u // v
        return matrix([[1, -q], [0, 1]])

    m = u.degree() // 2
    b0 = u // (x^m)
    b1 = v // (x^m)

    R = hgcd(b0, b1, depth=depth+1, min_degree=min_degree)
    DE = R * matrix([[u], [v]])
    d, e = DE[0,0], DE[1,0]
    q, f = pdivmod(d, e)

    g0 = e // x^(m//2)
    g1 = f // x^(m//2)

    S = hgcd(g0, g1, depth=depth+1, min_degree=min_degree)
    return S * matrix([[0, 1], [1, -q]]) * R

def pgcd(u, v):
    """
    fast implementation of polynomial GCD
    using hgcd
    """
    if u.degree() < v.degree():
        u, v = v, u

    if v == 0:
        return u

    if u % v == 0:
        return v

    if u.degree() < MIN:
        while v != 0:
            u, v = v, u % v
        return u

    R = hgcd(u, v)
    B = R * matrix([[u], [v]])
    b0, b1 = B[0,0], B[1,0]
    r = b0 % b1
    if r == 0:
        return b1
    return pgcd(b1, r)
