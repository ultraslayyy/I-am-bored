import Command, { Param } from '../cmd';
import { generateOtp } from '../utils/otp';
import { findToken } from '../utils/storage';

export default class generate extends Command {
    static commandName: string = 'generate';
    static description = '';
    static params: Param[] = [
        { name: 'label', type: 'string', required: true, positional: true },
        { name: 'time', type: 'number' },
        { name: 'window', type: 'number' },
        { name: 'padding', type: 'boolean' },
        { name: 'copy', type: 'boolean' },
        { name: 'raw', type: 'boolean' }
    ];
    static usage: string[] = ['votp generate <label> [flags]'];

    async execute(cmdArgs: Record<string, any>, globals: Record<string, any>, _: string[]) {
        const token = findToken(_[0]);
        if (token) {
            console.log(generateOtp(token));
        } else {
            throw new Error(`Token with label "${_[0]}" does not exist`);
        }
    }
}