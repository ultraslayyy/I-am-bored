import Command, { Param } from '../cmd';

export default class Import extends Command {
    static commandName = 'import';
    static description = '';
    static params: Param[] = [
        { name: 'uri', type: 'string', positional: true },
        { name: 'file', type: 'string' },
        { name: 'merge', type: 'boolean' },
        { name: 'encrypt', type: 'boolean' }
    ];
    static usage: string[] = ['votp import [<uri>] [flags]'];

    async execute() {

    }
}