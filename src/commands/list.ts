import Command, { Param} from '../cmd';

export default class list extends Command {
    static commandName = 'list';
    static description = '';
    static params: Param[] = [
        { name: 'filter', type: 'string' },
        { name: 'json', type: 'boolean' },
        { name: 'pretty', type: 'boolean' },
        { name: 'show-secrets', type: 'boolean' },
    ];
    static usage: string[] = ['votp list [flags]'];

    async execute() {
        
    }
}