import Command, { Param } from '../cmd';

export default class Export extends Command {
    static commandName = 'export';
    static description = '';
    static params: Param[] = [
        { name: 'label', type: 'string', required: true, positional: true, description: 'Label of otp key to export' },
        { name: 'format', type: 'string', default: 'otpauth', description: 'Format to export the key in. "otpauth" | "json" | "uri" | "raw" (default: otpauth)' },
        { name: 'pretty', type: 'boolean' },
        { name: 'output', type: 'string' }
    ];
    static usage: string[] = ['votp export <label> [flags]'];

    async execute() {

    }
}