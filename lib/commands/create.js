"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const cmd_1 = __importDefault(require("../cmd"));
class create extends cmd_1.default {
    constructor() {
        super(...arguments);
        this.templates = ['cli'];
    }
    async execute($global, $command, _, customParams = []) {
    }
    get help() {
        const defaultHelp = super.help;
        return `${defaultHelp}

Templates:
${this.templates.join(', ')}`;
    }
}
create.commandName = 'create';
create.description = 'Create a new project from a template';
create.params = [
    { name: 'template', type: 'string', positional: true, required: true, description: 'Template to use as boilerplate. Use "udt help create" for a full list.' }
];
create.usage = ['udt create <template>'];
exports.default = create;
