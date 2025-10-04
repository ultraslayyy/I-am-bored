import Command, { Param } from '../cmd';

export default class bump extends Command {
    static commandName: string = 'bump';
    static description = '';
    static params: Param[] = [
        { name: 'label', type: 'string', required: true, positional: true, description: 'Label of otp key to increase "counter" of' },
        { name: 'amount', type: 'number', default: 1, description: 'Amount to increase by (default: 1)' },
        { name: 'set', type: 'number', description: 'Set the "counter" to a specific number' },
        { name: 'preview', type: 'boolean', description: 'Show new counter/token without saving' }
    ];
    static usage: string[] = ['votp bump <label> [flags]'];

    async execute() {

    }
}