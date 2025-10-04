import Votp from '../cli';
import { commands } from './cmdList';

export default async function (votp: Votp, all?: boolean) {
    let allCommands;
    if (all) {
        allCommands = await usages(votp);
    } else {
        allCommands = commands.join(', ');
    }

    return `votp <command>
    
Usage:

votp help

All commands:
${allCommands}

votp@${(votp.constructor as typeof Votp).version}`;
}

async function usages(votp: Votp) {
    let cmds: [string, string][] = [];
    for (const c of commands) {
        const cmdModule = await Votp.cmd(c);
        if (cmdModule?.default) {
            const Command = cmdModule.default;
            const command = new Command(votp);
            const usage = command.constructor.usage;

            cmds.push([c, usage[0]]);
        }
    }

    const maxLength = Math.max(...cmds.map(([name]) => name.length));
    const lines = cmds.map(([name, desc]) => `${name.padEnd(maxLength + 8)}${desc}`);

    return lines.join('\n');
}