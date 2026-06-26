import sys

OPS = {
    'HALT': 0,
    'PUSH': 1,
    'ADD': 2,
    'PRINT': 3,
    'MUL': 4,
    'DIV': 5,
    'SUB': 6,
    'JMP': 10
}

def pass1(lines):
    pc = 0
    labels = {}

    for line in lines:
        line = line.strip()
        if not line:
            continue

        if line.endswith(':'):
            labels[line[:-1]] = pc
            continue

        parts = line.split()
        op = parts[0].upper()

        if op == 'PUSH':
            pc += 2
        elif op in ('ADD', 'PRINT', 'MUL', 'DIV', 'SUB', 'HALT'):
            pc += 1
        elif op == 'JMP':
            pc += 2

    return labels

def pass2(lines, labels):
    out = []

    for line in lines:
        line = line.strip()
        if not line or line.endswith(':'):
            continue
        
        parts = line.split()
        op = parts[0].upper()

        if op == 'HALT':
            out.append(OPS['HALT'])
        elif op == 'PUSH':
            out.append(OPS['PUSH'])
            out.append(int(parts[1]))
        elif op == 'ADD':
            out.append(OPS['ADD'])
        elif op == 'PRINT':
            out.append(OPS['PRINT'])
        elif op == 'MUL':
            out.append(OPS['MUL'])
        elif op == 'DIV':
            out.append(OPS['DIV'])
        elif op == 'SUB':
            out.append(OPS['SUB'])
        elif op == 'JMP':
            target = labels[parts[1]]
            out.append(OPS['JMP'])
            out.append(target)

    return out

def main():
    prg = sys.argv[1]
    out = sys.argv[2]

    if not out:
        out = f'{sys.argv[1].split('.')[0]}.bin'

    inp = open(prg).read().splitlines()

    labels = pass1(inp)
    bytecode = pass2(inp, labels)

    with open(out, 'wb') as f:
        f.write(bytes(bytecode))

    print('Compiled:', bytecode)

if __name__ == '__main__':
    main()