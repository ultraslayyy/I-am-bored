import Command, { Param } from '../cmd';
import fs from 'node:fs';
import path from 'node:path';
import readline from 'node:readline';
import { multipleChoice } from '../utils/logging';
import { spawn } from 'node:child_process';

export default class create extends Command {
    static commandName: string = 'create';
    static description: string = 'Create a new project from a template';
    static params: Param[] = [
        { name: 'template', type: 'string', positional: true, required: true, description: 'Template to use as boilerplate. Use "udt help create" for a full list.' },
        { name: 'dir', type: 'string', alias: 'd', description: 'Directory to place the template in. (default: .)' },
        { name: 'force', type: 'boolean', alias: 'f', default: false, description: 'Force override of files with same name' },
        { name: 'name', type: 'string', description: 'The name of the package' },
        { name: 'bin', type: 'string', description: 'The bin command for this package' },
        { name: 'internal', type: 'string', description: 'The capitalised internal name' },
        { name: 'author', type: 'string', description: 'The author of the package' }
    ];
    static usage: string[] = ['udt create <template>'];

    templates: string[] = ['cli'];

    async execute($global: Record<string, any>, $command: Record<string, any>, _: string[], customParams: string[] = []) {
        const targetDir = $command.dir || process.cwd();

        const templateModule = await import(`../utils/templates/${_[0]}.js`);
        const generateTemplate = templateModule.default;
        const questions = templateModule.questions || {}
        const installDeps = templateModule.installDeps || true;

        if (typeof generateTemplate !== 'function') {
            console.error(`Template "${_[0]}" must export a default function`);
            return;
        }

        const answers: Record<string, string> = {}
        if (Object.keys(questions).length > 0) {
            const rl = readline.createInterface({
                input: process.stdin,
                output: process.stdout
            });

            for (const [varName, prompt] of Object.entries(questions)) {
                if ($command[varName]) {
                    answers[varName] = $command[varName];
                } else {
                    answers[varName] = await new Promise(resolve => {
                        rl.question(`${prompt}: `, resolve);
                    });
                }
            }

            rl.close();
        }

        const template = generateTemplate(answers);

        for (const [relativePath, content] of Object.entries(template)) {
            const outputPath = path.join(targetDir, relativePath);
            fs.mkdirSync(path.dirname(outputPath), { recursive: true });
            fs.writeFileSync(outputPath, content as string, 'utf-8');
        }

        console.log(`Template ${_[0]}" created in ${targetDir}`);

        if (installDeps) {
            const yes = await multipleChoice(['Yes', 'No'], 'Do you want to install dependencies?');
            if (yes === 'Yes') {

                const child = spawn('npm install', {
                    cwd: targetDir,
                    shell: true,
                    stdio: 'inherit'
                });

                ['SIGINT', 'SIGTERM', 'SIGHUP'].forEach(sig => {
                    process.on(sig, () => {
                        if (!child.killed) child.kill(sig as NodeJS.Signals);
                    });
                });

                child.on('exit', (code, signal) => {
                    if (signal) {
                        process.kill(process.pid, signal);
                    } else {
                        process.exit(code ?? 0);
                    }
                });

                child.on('error', err => {
                    console.error('Failed to spawn install script:', err);
                    process.exit(1);
                });
            }
        }
    }

    get help() {
        const defaultHelp = super.help;
        return `${defaultHelp}

Templates:
${this.templates.join(', ')}`;
    }
}