"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.default = default_1;
const cli_1 = __importDefault(require("../cli"));
const cmdList_1 = require("./cmdList");
async function default_1(udt, all) {
    let allCommands;
    if (all) {
        allCommands = await usages(udt);
    }
    else {
        allCommands = cmdList_1.commands.join(', ');
    }
    return `udt <command>
    
Usage:

udt create <template>    Create a new project from a template
udt help                 Show help

All commands:
${allCommands}

udt@${udt.constructor.version}`;
}
async function usages(udt) {
    let cmds = [];
    for (const c of cmdList_1.commands) {
        const cmdModule = await cli_1.default.cmd(c);
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
