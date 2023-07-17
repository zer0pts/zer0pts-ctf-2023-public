from math import pi, e, log, log2, log10, sqrt, sin, cos
from ptrlib import ShortestPath

def T(state):
    const = [1.0, pi, log2(e), log2(10), log10(2), log(2)]
    for c in const:
        #yield (state / c, 2, f'div({c})')
        yield (state * c, 2, f'mul({c})')
        yield (state - c, 2, f'sub({c})')
        yield (state + c, 2, f'add({c})')
    if state > 0:
        yield (sqrt(state), 1, 'sqrt')
    yield (round(state), 1, 'rnd')

sp = ShortestPath(T)
cost, path = sp[0][0x20]
print(cost)
print(path.value)
