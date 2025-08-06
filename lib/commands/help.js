"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const cmd_1 = __importDefault(require("../cmd"));
const cli_1 = __importDefault(require("../cli"));
const defaultHelp_1 = __importDefault(require("../utils/defaultHelp"));
class help extends cmd_1.default {
    async execute($global, $command, _) {
        if (!_[0]) {
            if ($command.all) {
                return console.log(await (0, defaultHelp_1.default)(this.udt, true));
            }
            return console.log(await this.udt.constructor.usage);
        }
        try {
            const cmdModule = await cli_1.default.cmd(_[0]);
            if (!cmdModule?.default) {
                console.error(`Command '${_[0]}' not found`);
                return;
            }
            const Command = cmdModule.default;
            const command = new Command(this.udt);
            console.log(command.help);
        }
        catch (err) {
            console.error(err instanceof Error ? err.message : String(err));
        }
    }
}
help.commandName = 'help';
help.description = 'Show help';
help.params = [
    { name: 'command', type: 'string', positional: true, description: 'Show help for a command. Equivalent to "vex <command> -h"' },
    { name: 'all', type: 'boolean', alias: 'a', description: 'Show usages for all commands' }
];
help.usage = ['udt help [<command>]'];
exports.default = help;
