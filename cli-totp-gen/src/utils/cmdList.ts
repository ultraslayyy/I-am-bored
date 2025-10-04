export const commands: string[] = [
    'add',
    'bump',
    'export',
    'generate',
    'help',
    'import',
    'list',
    'qr',
    'remove',
    'validate'
];

export const aliases: { [a: string]: string } = {
    a: 'add',
    ad: 'add',

    b: 'bump',
    bmp: 'bump',

    e: 'export',
    exp: 'export',

    g: 'generate',
    gen: 'generate',

    h: 'help',

    i: 'import',
    imp: 'import',

    l: 'list',
    li: 'list',
    lst: 'list',

    q: 'qr',
    qrcode: 'qr',

    r: 'remove',
    rm: 'remove',
    rem: 'remove',
    
    v: 'validate',
    val: 'validate'
}

export function cmdRef(cmd: string) {
    if (commands.includes(cmd)) return cmd;
    if (aliases[cmd]) return aliases[cmd];
}