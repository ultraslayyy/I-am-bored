import Command, { Param } from '../cmd';
import UDT from '../cli';
import defaultHelp from '../utils/defaultHelp';

export default class help extends Command {
    static commandName = 'help';
    static description = 'Show help';
    static params: Param[] = [
        { name: 'command', type: 'string', positional: true, description: 'Show help for a command. Equivalent to "vex <command> -h"' },
        { name: 'all', type: 'boolean', alias: 'a', description: 'Show usages for all commands' }
    ];
    static usage = ['udt help [<command>]'];

    async execute($global: Record<string, any>, $command: Record<string, any>, _: string[]) {
        if (!_[0]) {
            if ($command.all) {
                return console.log(await defaultHelp(this.udt, true));
            }

            return console.log(await (this.udt.constructor as typeof UDT).usage);
        }

        try {
            const cmdModule = await UDT.cmd(_[0]);
            if (!cmdModule?.default) {
                console.error(`Command '${_[0]}' not found`);
                return;
            }

            const Command = cmdModule.default;
            const command = new Command(this.udt);

            console.log(command.help);
        } catch (err) {
            console.error(err instanceof Error ? err.message : String(err));
        }
    }
}