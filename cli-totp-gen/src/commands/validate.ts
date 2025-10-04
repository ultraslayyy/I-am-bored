import Command, { Param } from '../cmd';

export default class validate extends Command {
    static commandName = 'validate';
    static description = '';
    static params: Param[] = [
        { name: 'label', type: 'string', required: true, positional: true },
        { name: 'token', type: 'string', required: true, positional: true },
        { name: 'window', type: 'number' },
        { name: 'time', type: 'number' },
        { name: 'verbose', type: 'boolean' }
    ];
    static usage: string[] = ['votp validate <label> <token> [flags]'];

    async execute() {
        
    }
}