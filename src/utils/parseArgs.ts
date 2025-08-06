import { parseArgs } from 'node:util';
import { Param } from '../cmd';
import { globalParams } from './globalParams';

export interface ParsedArgs {
    _: string[];
    $global: Record<string, any>;
    $command: Record<string, any>;
}

function paramsToOptions(params: Param[]) {
    const options: Record<string, any> = {}

    for (const p of params) {
        options[p.name] = {
            type: p.type === 'boolean' ? 'boolean' : 'string',
            default: p.default,
            multiple: false
        }
        if (p.alias) options[p.name].short = p.alias;
    }

    return options;
}

export function parseCmdArgs(rawArgs: string[], commandParams: Param[]) {
    try {
        const globalOptions = paramsToOptions(globalParams);
        const commandOptions = paramsToOptions(commandParams);

        const globalFlagNames = makeFlagSet(globalParams);
        const onlyGlobalArgs = extractFlags(rawArgs, globalFlagNames);

        const globalParse = parseArgs({ args: onlyGlobalArgs, options: globalOptions, allowPositionals: true });
        const globalFlags = globalParse.values;

        if (globalFlags.help) {
            return { $global: globalFlags, $command: {}, _: globalParse.positionals ?? [] }
        }
        const fullParse = parseArgs({ args: rawArgs, options: { ...globalOptions, ...commandOptions }, allowPositionals: true });

        const $global: Record<string, any> = {};
        const $command: Record<string, any> = {};
        for (const key in fullParse.values) {
            if (key in globalOptions) {
                $global[key] = fullParse.values[key];
            } else {
                $command[key] = fullParse.values[key];
            }
        }

        const _ = fullParse.positionals ?? [];

        return { $global, $command, _ }
    } catch (err) {
        console.error(err instanceof Error ? err.message : String(err));
    }
}

function makeFlagSet(params: Param[]) {
    const names = new Set<string>();
    for (const p of params) {
        if (p.positional) continue;
        names.add(`--${p.name}`);
        if (p.alias) names.add(`-${p.alias}`);
    }
    return names;
}

function extractFlags(rawArgs: string[], validFlags: Set<string>) {
    return rawArgs.filter(arg => validFlags.has(arg));
}