import struct

with open("../distfiles/fvm", "rb") as f:
    f.seek(0x2004)
    asm = f.read(635)

def disassemble(asm):
    i = 0
    while i < len(asm):
        addr = i
        op = asm[i]
        i += 1
        mnemonic = [f'??? ({op:02x})']
        if 0x61 < op < 0x70:
            dest = struct.unpack('<h', asm[i:i+2])[0]
            i += 2
        match op:
            case 0x21: mnemonic = ['LOAD', '0']
            case 0x22: mnemonic = ['LOAD', '1']
            case 0x23: mnemonic = ['LOAD', 'pi']
            case 0x24: mnemonic = ['LOAD', 'log2(10)']
            case 0x25: mnemonic = ['LOAD', 'log2(e)']
            case 0x26: mnemonic = ['LOAD', 'log10(2)']
            case 0x27: mnemonic = ['LOAD', 'ln(2)']
            case x if 0x31 <= x <= 0x37: mnemonic = ['XCHG', f'st({x-0x30})']
            case 0x38: mnemonic = ['DUP']
            case 0x39:
                t = int.from_bytes(asm[i:i+10], 'little')
                i += 10
                mnemonic = ['PUSH', f'0x{t:010x}']
            case 0x3a: mnemonic = ['POP']
            case 0x41: mnemonic = ['ADD']
            case 0x42: mnemonic = ['SUB']
            case 0x43: mnemonic = ['MUL']
            case 0x44: mnemonic = ['DIV']
            case 0x45: mnemonic = ['NEG']
            case 0x51: mnemonic = ['SQRT']
            case 0x52: mnemonic = ['SIN']
            case 0x53: mnemonic = ['COS']
            case 0x54: mnemonic = ['RND']
            case 0x61: mnemonic = ['RET']
            case 0x62: mnemonic = ['CALL', f'loc_{i+dest:04x}']
            case 0x63: mnemonic = ['JMP', f'loc_{i+dest:04x}']
            case 0x64: mnemonic = ['JEQ', f'loc_{i+dest:04x}']
            case 0x65: mnemonic = ['JNE', f'loc_{i+dest:04x}']
            case 0x66: mnemonic = ['JAE', f'loc_{i+dest:04x}']
            case 0x67: mnemonic = ['JA', f'loc_{i+dest:04x}']
            case 0x68: mnemonic = ['JBE', f'loc_{i+dest:04x}']
            case 0x62: mnemonic = ['JB', f'loc_{i+dest:04x}']
            case 0x71: mnemonic = ['HLT']
            case 0x72: mnemonic = ['GET']
            case 0x73: mnemonic = ['PUT']
            
        print(f'{addr:04x}:\t' + '\t'.join(mnemonic))

disassemble(asm)
