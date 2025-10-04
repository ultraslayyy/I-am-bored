import { cmdRef } from './utils/cmdList';
import defaultHelp from './utils/defaultHelp';
import packageFile from '../package.json';
import { parseCmdArgs } from './utils/parseArgs';

export default class Votp {
    argv: string[] = [];

    constructor({ argv = [] } = {}) {
        this.argv = argv;
    }

    static async cmd(cmd: string) {
        const c = cmdRef(cmd);
        if (!c) {
            console.error(`Unknown command '${cmd}'`);
            return null;
        }
        try {
            return await import(`./commands/${c}.js`);
        } catch (err) {
            console.error(`Failed to load command module: './commands/${c}.js'`);
            return null;
        }
    }

    async execute(cmd: string, args = this.argv) {
        try {
            const cmdModule = await Votp.cmd(cmd);
            if (!cmdModule?.default) {
                console.error(`Command '${cmd}' not found.`);
                return;
            }

            const Command = cmdModule.default;
            const command = new Command(this);

            const parsed = parseCmdArgs(args, Command.params);
            
            await command.execute(parsed?.$command, parsed?.$global, parsed?._);
        } catch (err) {
            console.error(err instanceof Error ? err.message : String(err));
        }
    }

    static get version() {
        return packageFile.version;
    }

    get usage() {
        return defaultHelp(this);
    }
}