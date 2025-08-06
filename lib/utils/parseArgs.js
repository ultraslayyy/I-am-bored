"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.parseCmdArgs = parseCmdArgs;
const node_util_1 = require("node:util");
const globalParams_1 = require("./globalParams");
function paramsToOptions(params) {
    const options = {};
    for (const p of params) {
        options[p.name] = {
            type: p.type === 'boolean' ? 'boolean' : 'string',
            default: p.default,
            multiple: false
        };
        if (p.alias)
            options[p.name].short = p.alias;
    }
    return options;
}
function parseCmdArgs(rawArgs, commandParams) {
    try {
        const globalOptions = paramsToOptions(globalParams_1.globalParams);
        const commandOptions = paramsToOptions(commandParams);
        const globalFlagNames = makeFlagSet(globalParams_1.globalParams);
        const onlyGlobalArgs = extractFlags(rawArgs, globalFlagNames);
        const globalParse = (0, node_util_1.parseArgs)({ args: onlyGlobalArgs, options: globalOptions, allowPositionals: true });
        const globalFlags = globalParse.values;
        if (globalFlags.help) {
            return { $global: globalFlags, $command: {}, _: globalParse.positionals ?? [] };
        }
        const fullParse = (0, node_util_1.parseArgs)({ args: rawArgs, options: { ...globalOptions, ...commandOptions }, allowPositionals: true });
        const $global = {};
        const $command = {};
        for (const key in fullParse.values) {
            if (key in globalOptions) {
                $global[key] = fullParse.values[key];
            }
            else {
                $command[key] = fullParse.values[key];
            }
        }
        const _ = fullParse.positionals ?? [];
        return { $global, $command, _ };
    }
    catch (err) {
        console.error(err instanceof Error ? err.message : String(err));
    }
}
function makeFlagSet(params) {
    const names = new Set();
    for (const p of params) {
        if (p.positional)
            continue;
        names.add(`--${p.name}`);
        if (p.alias)
            names.add(`-${p.alias}`);
    }
    return names;
}
function extractFlags(rawArgs, validFlags) {
    return rawArgs.filter(arg => validFlags.has(arg));
}
