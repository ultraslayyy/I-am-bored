#!/usr/bin/env node
import { addPackage } from './commands/add';
import { installPackages } from './commands/install';
import { removePackage } from './commands/remove';

const args = process.argv.slice(2);
const command = args[0];

switch (command) {
    case 'install':
        installPackages();
        break;
    case 'add':
        let input = args[1];
        if (!input) {
            console.error('Please specify a package to add.');
            process.exit(1);
        }

        let pkg: string;
        let ver: string | undefined;

        if (input.startsWith('@')) {
            const atIndex = input.indexOf('@', 1);
            if (atIndex === -1) {
                pkg = input;
            } else {
                pkg = input.slice(0, atIndex);
                ver = input.slice(atIndex + 1);
            }
        } else {
            const atIndex = input.indexOf('@');
            if (atIndex === -1) {
                pkg = input;
            } else {
                pkg = input.slice(0, atIndex);
                ver = input.slice(atIndex + 1);
            }
        }

        addPackage(pkg, ver || 'latest', true);
        break;
    case 'emulate':
        console.warn('Not implemented yet.');
        break;
    case 'remove':
        const packageName = args[1];
        if (!packageName) {
            console.error('Please specify a package to add.');
            process.exit(1);
        }

        removePackage(packageName);
    case 'help':
    default:
        console.log(`
vex - Experimental Package Manager (v0.0.1)

Usage:
  vex install              Install dependencies
  vex add <package>        Add a new package
  vex emulate npm|yarn     Use fallback installer (WIP)
  vex help                 Show help
        `)
}