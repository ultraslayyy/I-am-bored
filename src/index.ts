#!/usr/bin/env node
const args = process.argv.slice(2);
const command = args[0];

switch (command) {
    case 'install':
        console.log('Running install');
        break;
    case 'add':
        const pkg = args[1];
        if (!pkg) {
            console.error('Please specify a package to add.');
            process.exit(1);
        }
        console.log(`Adding package: ${pkg}`);
        break;
    case 'help':
    default:
        console.log(`
vex - Custom Package Manager (v0.0.1)

Usage:
  vex install           Install dependencies
  vex add <package>     Add a new package
  vex help              Show help
    `)
}