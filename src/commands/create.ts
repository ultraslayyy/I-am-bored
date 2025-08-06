import Command, { Param } from '../cmd';
import readline from 'readline';

export default class create extends Command {
    static commandName: string = 'create';
    static description: string = 'Create a new project from a template';
    static params: Param[] = [
        { name: 'template', type: 'string', positional: true, required: true, description: 'Template to use as boilerplate. Use "udt help create" for a full list.' },
        { name: 'force', type: 'boolean', alias: 'f', description: 'Force override of files with same name' }
    ];
    static usage: string[] = ['udt create <template>'];

    templates: string[] = ['cli'];

    async execute($global: Record<string, any>, $command: Record<string, any>, _: string[], customParams: string[] = []) {
        
    }

    get help() {
        const defaultHelp = super.help;
        return `${defaultHelp}

Templates:
${this.templates.join(', ')}`;
    }
}