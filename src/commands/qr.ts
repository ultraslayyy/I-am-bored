import Command, { Param } from '../cmd';

export default class qr extends Command {
    static commandName = 'qr';
    static description = '';
    static params: Param[] = [
        { name: 'label', type: 'string', required: true, positional: true, description: 'The label of the otp key to generate a qr code for' },
        { name: 'format', type: 'string', default: 'ascii', description: 'Format to export as. "ascii" | "png" | "svg" (default: ascii)' },
        { name: 'output', type: 'string', description: 'Where to store the png/svg, if format is "png" or "svg" (default: qr-<label>.png)' },
        { name: 'margin', type: 'number', description: 'Whitespace around the QR' },
        { name: 'invert', type: 'boolean', description: 'Invert terminal colors (ascii format only)' },
        { name: 'copy-uri', type: 'boolean', description: 'Copy otpauth URI to clipboard' }
    ];
    static usage = ['votp qr <label> [flags]'];

    async execute(cmdArgs: Record<string, any>, globals: Record<string, any>) {
        if (globals.help) {
            console.log(`Usage: votp qr <label> [options]`);
            console.log(`Description: ${qr.description}`);
            console.log(`Options:`);
            for (const p of qr.params) {
                const aliasStr = p.alias ? `, -${p.alias}` : '';
                console.log(`  -${p.name}${aliasStr} (${p.type})${p.required ? '  [required]' : ''} ${p.description ? `- ${p.description}` : ''}`);
            }
            return;
        }
        return;
    }
}