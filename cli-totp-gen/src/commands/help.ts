import Command, { Param } from '../cmd';
import Votp from '../cli';
import defaultHelp from '../utils/defaultHelp';

export default class help extends Command {
    static commandName: string = 'help';
    static description = '';
    static params: Param[] = [
        { name: 'command', type: 'string', positional: true },
        { name: 'all', type: 'boolean', alias: 'a' }
    ];
    static usage: string[] = ['votp help [<command>] [flags]'];

    async execute(cmdArgs: Record<string, any>, globals: Record<string, any>, _: Record<string, any>) {
        if (!_[0]) {
            if (cmdArgs.all) {
                return console.log(await defaultHelp(this.votp, true));
            }

            return console.log(await this.votp.usage);
        }

        try {
            const cmdModule = await Votp.cmd(_[0]);
            if (!cmdModule?.default) {
                console.error(`Command '${_[0]}' not found.`);
                return;
            }

            const commandClass = cmdModule.default;
            const command = new commandClass(this.votp);

            console.log(command.help);
        } catch {}
    }
}