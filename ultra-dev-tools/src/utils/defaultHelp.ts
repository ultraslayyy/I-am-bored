import UDT from '../cli';
import { commands } from './cmdList';

export default async function (udt: UDT, all?: boolean) {
    let allCommands;
    if (all) {
        allCommands = await usages(udt);
    } else {
        allCommands = commands.join(', ');
    }

    return `udt <command>
    
Usage:

udt create <template>    Create a new project from a template
udt help                 Show help

All commands:
${allCommands}

udt@${(udt.constructor as typeof UDT).version}`;
}

async function usages(udt: UDT) {
    let cmds: [string, string][] = [];
    for (const c of commands) {
        const cmdModule = await UDT.cmd(c);
        if (cmdModule?.default) {
            const Command = cmdModule.default;
            const command = new Command(udt);
            const usage = command.constructor.usage;

            cmds.push([c, usage[0]]);
        }
    }

    const maxLength = Math.max(...cmds.map(([name]) => name.length));
    const lines = cmds.map(([name, desc]) => `${name.padEnd(maxLength + 8)}${desc}`);

    return lines.join('\n');
}