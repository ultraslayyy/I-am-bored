export default function ({ name, internal, binName = name.replace(' ', '-'), author = '' }: { name: string, internal: string, binName: string, author: string }) {
    return {
        //#region cwd
        'package.json': `{
  "name": "${name}",
  "version": "0.0.1",
  "main": "./bin/${binName}-cli.js",
  "scripts": {
    "build": "tsc"
  },
  "bin": {
    "${binName}": "./bin/${binName}-cli.js"
  },
  "keywords": [
    "cli"
  ],
  "author": "${author}",
  "license": "MIT",
  "type": "commonjs",
  "devDependencies": {
    "@types/node": "^24.2.0",
    "typescript": "^5.9.2"
  },
  "files": ["lib", "bin"],
  "engines": {
    "node": ">=16.19.0"
  }
}`,
        'tsconfig.json': `{
  "compilerOptions": {
    "target": "es2020",
    "module": "commonjs",
    "rootDir": "./src",
    "resolveJsonModule": true,
    "outDir": "./lib",
    "esModuleInterop": true,
    "forceConsistentCasingInFileNames": true,
    "strict": true,
    "skipLibCheck": true
  },
  "include": ["src"]
}`,
        //#endregion
        //#region bin/
        [`bin/${binName}-cli.js`]: `#!/usr/bin/env node
try {
    const { enableCompileCache } = require('node:module');
    enableCompileCache?.();
} catch {}
 
const ${internal} = require('../lib/cli.js');

(async () => {
    const argv = process.argv.slice(2);
    const ${binName} = new ${internal}.default({ argv });

    if (argv.length === 0) {
        await ${binName}.execute('help');
        return;
    }

    const cmd = argv[0];
    const args = argv.slice(1);

    await ${binName}.execute(cmd, args);
})();`,
        [`bin/${binName}`]: `#!/bin/sh
basedir=$(dirname "$(echo "$0" | sed -e 's,\\,/,g')")

case \`uname\` in
    *CYGWIN*|*MINGW*|*MSYS*)
        if command -v cygpath > /dev/null 2>&1; then
            basedir=\`cygpath -w "$basedir"\`
        fi
    ;;
esac

if [ -x "$basedir/node" ]; then
    exec "$basedir/node"  "$basedir/node_modules/${name}/bin/${binName}-cli.js" "$@"
else
    exec node  "$basedir/node_modules/${name}/bin/${binName}-cli.js" "$@"
fi`,
        [`bin/${binName}.cmd`]: `@ECHO off
GOTO start
:find_dp0
SET dp0=%~dp0
EXIT /b
:start
SETLOCAL
CALL :find_dp0

IF EXIST "%dp0%\\node.exe" (
    SET "_prog=%dp0%\\node.exe"
) ELSE (
    SET "_prog=node"
    SET PATHEXT=%PATHEXT:;.JS;=;%
)

endLocal & goto #_undefined_# 2>NUL || title %COMSPEC% & "%_prog%"  "%dp0%\\node_modules\\${name}\\bin\\${binName}-cli.js" %*`,
        [`bin/${binName}.ps1`]: `#!/usr/bin/env pwsh
$basedir=Split-Path $MyInvocation.MyCommand.Definition -Parent

$exe=""
if ($PSVersionTable.PSVersion -lt "6.0" -or $IsWindows) {
  # Fix case when both the Windows and Linux builds of Node
  # are installed in the same directory
  $exe=".exe"
}
$ret=0
if (${internal}-Path "$basedir/node$exe") {
  # Support pipeline input
  if ($MyInvocation.ExpectingInput) {
    $input | & "$basedir/node$exe"  "$basedir/node_modules/${name}/bin/${binName}-cli.js" $args
  } else {
    & "$basedir/node$exe"  "$basedir/node_modules/${name}/bin/${binName}-cli.js" $args
  }
  $ret=$LASTEXITCODE
} else {
  # Support pipeline input
  if ($MyInvocation.ExpectingInput) {
    $input | & "node$exe"  "$basedir/node_modules/${name}/bin/${binName}-cli.js" $args
  } else {
    & "node$exe"  "$basedir/node_modules/${name}/bin/${binName}-cli.js" $args
  }
  $ret=$LASTEXITCODE
}
exit $ret`,
        //#endregion
        //#region src/
        'src/cli.ts': `import { cmdRef } from './utils/cmdList';
import { parseCmdArgs } from './utils/parseArgs';
import packageFile from '../package.json';
import defaultHelp from './utils/defaultHelp';

export default class ${internal} {
    argv: string[] = [];

    constructor({ argv = [] } = {}) {
        this.argv = argv;
    }

    static async cmd(cmd: string) {
        const c = cmdRef(cmd);
        if (!c) {
            console.error(\`Unknown command '\${cmd}'\`);
            return null;
        }
        try {
            return await import(\`./command/\${c}.js\`);
        } catch {
            console.error(\`Failed to load command module: './commands/\${c}.js'\`);
            return null; 
        }
    }

    async execute(cmd: string, args = this.argv) {
        try {
            const cmdModule = await (this.constructor as typeof ${internal}).cmd(cmd);
            if (!cmdModule.default) {
                console.error(\`Command '\${cmd}' not found.\`);
                return;
            }

            const Command = cmdModule.default;
            const command = new Command(this);

            if (command.customParams) {
            
            } else {
                const parsed = parseCmdArgs(args, Command.params);
                if (!parsed) return;

                if (parsed.$global.help) {
                    await this.execute('help', [cmdRef(cmd) ?? '']);
                    return;
                }

                await command.execute(parsed.$global, parsed.$command, parsed._);
            }
        } catch (err) {
            console.error(err instanceof Error ? err.message : String(err)); 
        }
    }

    static get version() {
        return packageFile.version;
    }

    static get usage() {
        return defaultHelp(new this);
    }
}`,
        'src/cmd.ts': `export interface Param {
    name: string;
    type: 'string' | 'number' | 'boolean';
    required?: boolean;
    positional?: boolean;
    description?: string;
    default?: any;
    alias?: string;
}

export abstract class Command {
    static commandName: string;
    static description: string = '';
    static params: Param[] = [];
    static usage: string[] = [];
    static customParams: boolean = false;
    ${binName}: import('./cli').default;

    constructor(${binName}: import('./cli').default) {
        this.${binName} = ${binName};
    }

    abstract execute($global: Record<string, any>, $command: Record<string, any>, _: string[], customParams?: string[]): Promise<void> | void;

    get name() {
        return (this.constructor as typeof Command).commandName;
    }

    get description() {
        return (this.constructor as typeof Command).description;
    }

    get params() {
        return (this.constructor as typeof Command).params;
    }

    get usage() {
        return (this.constructor as typeof Command).usage;
    }

    get customParams() {
        return (this.constructor as typeof Command).customParams;
    }

    get help() {
        let positionals: string[] = [];
        for (const param of this.params) {
            if (param.positional === true) {
                positionals.push(\`\${param.required ? '' : '['}<\${param.name}>\${param.required ? '' : ']'}\`);
            }
        }

        const rows = this.params.map(p => {
            if (!p.positional) {
                const placeholder = p.type === 'string' || p.type === 'number' ? ~p.default ? \`<\${p.name}>\` : \`[<\${p.name}>]\` : '';
                const flag = \`--\${p.name} \${placeholder}\`;
                const requiredTag = p.required ? '[required]' : '';
                return { flag, requiredTag, description: p.description }
            }
            return undefined;
        }).filter((r): r is NonNullable<typeof r> => r !== undefined);

        const flagWidth = Math.max(...rows.map(r => r.flag.length)) + 2;
        const tagWidth = Math.max(...rows.map(r => r.requiredTag.length)) + 2;

        const formatted = rows.map(r => {
            return (
                r.flag.padEnd(flagWidth) +
                r.requiredTag.padEnd(tagWidth) +
                r.description
            );
        });

        return \`${binName} \${this.name} \${positionals}

Options:
\${this.params.length !== 0 ? formatted.join('\\n') : 'None'}\`;
    }
}

export default Command;`,
        //#region src/commands/
        'src/commands/help.ts': `import Command, { Param } from '../cmd';
import ${internal} from '../cli';
import defaultHelp from '../utils/defaultHelp';

export default class help extends Command {
    static commandName = 'help';
    static description = 'Show help';
    static params: Param[] = [
        { name: 'command', type: 'string', positional: true, description: 'Show help for a command. Equivalent to "${binName} <command> -h"' },
        { name: 'all', type: 'boolean', alias: 'a', description: 'Show usages for all commands' }
    ];
    static usage = ['${binName} help [<command>]'];

    async execute($global: Record<string, any>, $command: Record<string, any>, _: string[]) {
        if (!_[0]) {
            if ($command.all) {
                return console.log(await defaultHelp(this.${binName}), true)
            }

            return console.log(await (this.${binName}.constructor as typeof ${internal}).usage);
        }

        try {
            const cmdModule = await ${internal}.cmd(_[0]);
            if (!cmdModule?.default) {
                console.error(\`Command '\${_[0]}' not found\`);
                return;
            }

            const Command = cmdModule.default;
            const command = new Command(this.${binName});

            console.log(command.help);
        } catch (err) {
            console.error(err instanceof Error ? err.message : String(err)); 
        }
    }
}`,
        //#endregion
        //#region src/utils/
        'src/utils/cmdList.ts': `export const commands: string[] = [
    'help'
];

export const aliases: Record<string, string> = {
    h: 'help'
}
    
export function cmdRef(cmd: string) {
    if (commands.includes(cmd)) return cmd;
    if (aliases[cmd]) return aliases[cmd];
}`,
        'src/utils/defaultHelp.ts': `import ${internal} from '../cli';
import { commands } from './cmdList';

export default async function (${binName}: ${internal}, all?: boolean) {
    let allCommands;
    if (all) {
        allCommands = await usages(${binName});
    } else {
        allCommands = commands.join(', ');
    }

    return \`${binName} <command>

Usage:

${binName} help             Show help

All commands:
\${allCommands}

${binName}@\${(${binName}.constructor as typeof ${internal}).version}\`;
}

async function usages(${binName}: ${internal}) {
    let cmds: [string, string][] = [];
    for (const c of commands) {
        const cmdModule = await ${internal}.cmd(c);
        if (cmdModule?.default) {
            const Command = cmdModule.default;
            const command = new Command(${binName});
            const usage = command.constructor.usage;

            cmds.push([c, usage[0]]);
        }
    }

    const maxLength = Math.max(...cmds.map(([name]) => name.length));
    const lines = cmds.map(([name, desc]) => \`\${name.padEnd(maxLength + 8)}\${desc}\`);

    return lines.join('\\n');
}`,
        'src/utils/globalParams.ts': `import { Param } from '../cmd';

export const globalParams: Param[] = [
    { name: 'help', type: 'boolean', alias: 'h', description: 'Show help' }
];`,
        'src/utils/parseArgs.ts': `import { parseArgs } from 'node:util';
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
        names.add(\`--\${p.name}\`);
        if (p.alias) names.add(\`-\${p.alias}\`);
    }
    return names;
}

function extractFlags(rawArgs: string[], validFlags: Set<string>) {
    return rawArgs.filter(arg => validFlags.has(arg));
}`
        //#endregion
        //#endregion
    }
}

export const questions: Record<string, string> = {
    name: 'What is the name of this tool (Appears on npm)',
    binName: 'What is the name of this tool\'s bin command',
    internal: 'What is the internal capitalised bin name',
    author: 'Who is the author of this package'
}