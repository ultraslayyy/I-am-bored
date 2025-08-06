"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.Command = void 0;
class Command {
    constructor(udt) {
        this.udt = udt;
    }
    get name() {
        return this.constructor.commandName;
    }
    get description() {
        return this.constructor.description;
    }
    get params() {
        return this.constructor.params;
    }
    get usage() {
        return this.constructor.usage;
    }
    get customParams() {
        return this.constructor.customParams;
    }
    get help() {
        let positionals = [];
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
                return { flag, requiredTag, description: p.description };
            }
            return undefined;
        }).filter((r) => r !== undefined);
        const flagWidth = Math.max(...rows.map(r => r.flag.length)) + 2;
        const tagWidth = Math.max(...rows.map(r => r.requiredTag.length)) + 2;
        const formatted = rows.map(r => {
            return (r.flag.padEnd(flagWidth) +
                r.requiredTag.padEnd(tagWidth) +
                r.description);
        });
        return `udt ${this.name} ${positionals}
        
Options:
${this.params.length !== 0 ? formatted.join('\n') : 'None'}`;
    }
}
exports.Command = Command;
Command.description = null;
Command.params = [];
Command.usage = [];
Command.customParams = false;
exports.default = Command;
