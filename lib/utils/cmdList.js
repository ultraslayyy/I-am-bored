"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.aliases = exports.commands = void 0;
exports.cmdRef = cmdRef;
exports.commands = [
    'create',
    'help'
];
exports.aliases = {
    c: 'create',
    h: 'h'
};
function cmdRef(cmd) {
    if (exports.commands.includes(cmd))
        return cmd;
    if (exports.aliases[cmd])
        return exports.aliases[cmd];
}
