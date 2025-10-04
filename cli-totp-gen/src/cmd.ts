export interface Param {
    name: string;
    type: 'string' | 'number' | 'boolean';
    required?: boolean;
    positional?: boolean;
    description?: string;
    default?: any;
    alias?: string;
}

export abstract class Command {
    static commandName: string;
    static description: string | null = null;
    static params: Param[] = [];
    static usage: string[];
    votp: import('./cli').default;

    constructor(votp: import('./cli').default) {
        this.votp = votp;
    }

    abstract execute(cmdArgs: Record<string, any>, globals: Record<string, any>, _: Record<string, any>): Promise<void> | void;

    get name() {
        return (this.constructor as typeof Command).commandName;
    }

    get description() {
        return (this.constructor as typeof Command).description;
    }

    get params() {
        return (this.constructor as typeof Command).params;
    }

    get usage() {
        return (this.constructor as typeof Command).usage;
    }

    get help() {
        let positionals: string[] = [];
        for (const param of this.params) {
            if (param.positional === true) {
                positionals.push(`${param.required ? '' : '['}<${param.name}>${param.required ? '' : ']'}`);
            }
        }

        const rows = this.params.map(p => {
            const placeholder = p.type === 'string' || p.type === 'number' ?  !p.default ? `<${p.name}>` : `[<${p.name}>]` : '';
            const flag = `--${p.name} ${placeholder}`;
            const requiredTag = p.required ? "[required]" : "";
            return { flag, requiredTag, description: p.description };
        });

        const flagWidth = Math.max(...rows.map(r => r.flag.length)) + 2;
        const tagWidth = Math.max(...rows.map(r => r.requiredTag.length)) + 2;

        const formatted = rows.map(r => {
            return (
                r.flag.padEnd(flagWidth) +
                r.requiredTag.padEnd(tagWidth) +
                r.description
            );
        });

        return `votp ${(this.constructor as typeof Command).commandName} ${positionals}

Options:
${formatted.join('\n')}`;
    }
}

export default Command;