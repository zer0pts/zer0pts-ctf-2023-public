import struct

def assemble(code):
    def p16(v):
        return struct.pack('<h', v)
    insns = []
    labels = {}
    pos = 0
    for line in code.split('\n'):
        line = line.strip()
        if line == '': continue
        if line.startswith(';'): continue
        if line.endswith(':'):
            labels[line[:-1]] = pos
            continue

        ops = line.split()
        if len(ops) > 1 and ops[1].startswith(";"):
            ops = [ops[0]]
        insns.append(ops)

        if ops[0].lower() == 'push':
            pos += 1 + 10
        elif len(ops) == 2:
            pos += 1 + 2
        else:
            pos += 1

    pos = 0
    asm = b''
    for insn in insns:
        if insn[0].lower() == 'push':
            pos += 1 + 10
        elif len(insn) == 2:
            pos += 1 + 2
        else:
            pos += 1

        match insn[0].lower():
            case 'ld0': asm += b'\x21'
            case 'ld1': asm += b'\x22'
            case 'ldp': asm += b'\x23'
            case 'ldl2t': asm += b'\x24'
            case 'ldl2e': asm += b'\x25'
            case 'ldlg2': asm += b'\x26'
            case 'ldln2': asm += b'\x27'
            case 'xch1': asm += b'\x31'
            case 'xch2': asm += b'\x32'
            case 'xch3': asm += b'\x33'
            case 'xch4': asm += b'\x34'
            case 'xch5': asm += b'\x35'
            case 'xch6': asm += b'\x36'
            case 'xch7': asm += b'\x37'
            case 'dup': asm += b'\x38'
            case 'push':
                v = int(insn[1], 16) if insn[1][:2] == '0x' else int(insn[1])
                asm += b'\x39' + int.to_bytes(v, 10, 'little')
            case 'pop': asm += b'\x3a'
            case 'add': asm += b'\x41'
            case 'sub': asm += b'\x42'
            case 'mul': asm += b'\x43'
            case 'div': asm += b'\x44'
            case 'neg': asm += b'\x45'
            case 'sqrt': asm += b'\x51'
            case 'sin': asm += b'\x52'
            case 'cos': asm += b'\x53'
            case 'rnd': asm += b'\x54'
            case 'ret': asm += b'\x61'
            case 'call': asm += b'\x62' + p16(labels[insn[1]] - pos)
            case 'jmp': asm += b'\x63' + p16(labels[insn[1]] - pos)
            case 'jeq': asm += b'\x64' + p16(labels[insn[1]] - pos)
            case 'jne': asm += b'\x65' + p16(labels[insn[1]] - pos)
            case 'jae': asm += b'\x66' + p16(labels[insn[1]] - pos)
            case 'ja' : asm += b'\x67' + p16(labels[insn[1]] - pos)
            case 'jbe': asm += b'\x68' + p16(labels[insn[1]] - pos)
            case 'jb' : asm += b'\x69' + p16(labels[insn[1]] - pos)
            case 'hlt': asm += b'\x71'
            case 'get': asm += b'\x72'
            case 'put': asm += b'\x73'
            case _: raise NotImplementedError(insn[0])

    return asm

asm = assemble(open("code.S").read())

with open("code.bin", "wb") as f:
    f.write(asm)
