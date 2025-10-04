export const commands: string[] = [
    'create',
    'help'
];

export const aliases: { [a: string]: string } = {
    c: 'create',
    h: 'h'
}

export function cmdRef(cmd: string) {
    if (commands.includes(cmd)) return cmd;
    if (aliases[cmd]) return aliases[cmd];
}