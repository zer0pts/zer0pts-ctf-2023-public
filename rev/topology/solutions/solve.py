from tqdm import tqdm
from ptrlib import *
import archinfo
import angr
import re

path = "../distfiles/topology"
#path = "topology"

elf = ELF(path)
counters, funcs = {}, {}
for symbol, address in elf.symbols().items():
    if re.fullmatch(b"[a-zA-Z0-9]{8}\.\d+", symbol):
        counters[symbol[:8].decode()] = address
    if re.fullmatch(b"[a-zA-Z0-9]{8}", symbol):
        funcs[symbol.decode()] = address

ANSWERS = {}
def hook_ret(state):
    global ANSWERS
    instr = state.memory.load(state.regs.rip, 1)
    if state.solver.eval(instr) == 0xc3:
        state.solver.add(state.regs.rax == 0)
        ANSWERS[FUNCTION].append(state.solver.eval(FLAG, cast_to=bytes))

p = angr.Project(path, auto_load_libs=False)
cfg = p.analyses.CFGFast()

ADDR_STR = 0xdead0000
FUNCTION = None
for symbol in tqdm(funcs):
    addr_start = 0x400000+funcs[symbol]
    FUNCTION = symbol
    ANSWERS[FUNCTION] = []
    print(symbol, hex(addr_start))

    # Hook ret instruction
    function = cfg.functions[addr_start]
    for block in function.blocks:
        end_addr = block.addr + block.size - 1
        p.hook(end_addr, hook_ret, length=0)

    for part in range(10):
        state = p.factory.blank_state(
            addr=addr_start,
            add_options={angr.options.ZERO_FILL_UNCONSTRAINED_REGISTERS} \
            | {angr.options.ZERO_FILL_UNCONSTRAINED_MEMORY},
        )

        # Set initial state
        state.regs.rdi = ADDR_STR
        state.memory.store(0x400000+counters[symbol],
                           state.solver.BVV(part, 32),
                           endness=archinfo.Endness.LE)
        FLAG = state.solver.BVS('v', 64)
        state.memory.store(state.regs.rdi, FLAG)

        sim = p.factory.simgr(state)
        sim.run()

    print(ANSWERS[FUNCTION])

flag = b""
for i in range(10):
    counts = {}
    for func in funcs:
        ans = ANSWERS[func][i]
        if ans in counts:
            counts[ans] += 1
        else:
            counts[ans] = 1

    flag += sorted(counts.items(), key=lambda x: -x[1])[0][0]
    print(flag)
