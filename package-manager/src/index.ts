#!/usr/bin/env node
import { addPackage, parseAddArgs } from './commands/add';
import { init } from './commands/init';
import { installPackages } from './commands/install';
import { removePackage } from './commands/remove';

const [, , command, ...args] = process.argv;
async function main() {
    switch (command) {
        case 'install':
        case 'i':
            installPackages(args);
            break;
        case 'add':
            if (!args) {
                console.error('Please specify a package to add.');
                process.exit(1);
            }

            const pkgs = await parseAddArgs(args);
            await addPackage(pkgs, true);
            break;
        case 'remove':
            const packageName = args[1];
            if (!packageName) {
                console.error('Please specify a package to add.');
                process.exit(1);
            }

            await removePackage(packageName);
        case 'init':
            await init(args);
            break;
        case 'help':
        default:
            console.log(`
vex - Experimental Package Manager (v0.0.1)

Usage:
  vex install              Install dependencies
  vex add <package>        Add a new package
  vex init [-y|--yes]      Creates a new package.json
  vex help                 Show help
            `);
    }
}

main();