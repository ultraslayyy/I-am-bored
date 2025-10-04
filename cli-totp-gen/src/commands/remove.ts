import Command, { Param } from '../cmd';
import { deleteToken } from '../utils/storage';

export default class remove extends Command {
    static commandName = 'remove';
    static description = '';
    static params: Param[] = [
        { name: 'label', type: 'string', required: true, positional: true },
        { name: 'force', type: 'boolean' },
        { name: 'backup', type: 'string' },
        { name: 'quiet', type: 'boolean' }
    ];
    static usage: string[] = ['votp remove <label> [flags]'];

    async execute(cmdArgs: Record<string, any>, globals: Record<string, any>, _: string[]) {
        deleteToken(_[0]);
    }
}