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
    static usage: string[] = [];
    static customParams: boolean = false;
    udt: import('./cli').default;

    constructor(udt: import('./cli').default) {
        this.udt = udt;
    }

    abstract execute($global: Record<string, any>, $command: Record<string, any>, _: string[], customParams?: string[]): Promise<void> | void;

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

    get customParams() {
        return (this.constructor as typeof Command).customParams;
    }

    get help() {
        let positionals: string[] = [];
        for (const param of this.params) {
            if (param.positional === true) {
                positionals.push(`${param.required ? '' : '['}<${param.name}>${param.required ? '' : ']'}`);
            }
        }

        const rows = this.params.map(p => {
            if (!p.positional) {
                const placeholder = p.type === 'string' || p.type === 'number' ? ~p.default ? `<${p.name}>` : `[<${p.name}>]` : '';
                const flag = `--${p.name} ${placeholder}`;
                const requiredTag = p.required ? '[required]' : '';
                return { flag, requiredTag, description: p.description }
            }
            return undefined;
        }).filter((r): r is NonNullable<typeof r> => r !== undefined);

        const flagWidth = Math.max(...rows.map(r => r.flag.length)) + 2;
        const tagWidth = Math.max(...rows.map(r => r.requiredTag.length)) + 2;

        const formatted = rows.map(r => {
            return (
                r.flag.padEnd(flagWidth) +
                r.requiredTag.padEnd(tagWidth) +
                r.description
            );
        });

        return `udt ${this.name} ${positionals}
        
Options:
${this.params.length !== 0 ? formatted.join('\n') : 'None'}`;
    }
}

export default Command;