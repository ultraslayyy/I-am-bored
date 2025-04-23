#!/usr/bin/env node
"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const commander_1 = require("commander");
const net_1 = require("net");
const fs_1 = require("fs");
const os_1 = require("os");
const log_1 = require("./log");
const path_1 = __importDefault(require("path"));
const program = new commander_1.Command();
program
    .name('ultra-file-transfer')
    .description('CLI-based P2P file transfer tool')
    .version('1.0.0');
program
    .command('send')
    .description('Send a file to a peer')
    .requiredOption('--file <path>', 'Path file to send')
    .requiredOption('--host <ip>', 'Destination IP address')
    .requiredOption('--port <port>', 'Destination port', parseInt)
    .action((options) => {
    const { file, host, port } = options;
    const fileName = path_1.default.basename(file);
    const filePath = path_1.default.resolve(file);
    const fileSize = (0, fs_1.statSync)(filePath).size;
    const startTime = Date.now();
    const client = (0, net_1.createConnection)({ host, port }, () => {
        client.write(fileName + '\n');
        const stream = (0, fs_1.createReadStream)(file);
        stream.pipe(client);
        log_1.log.info(`Sending file: ${fileName}`);
        stream.on('end', () => {
            const endTime = Date.now();
            const duration = ((endTime - startTime) / 1000).toFixed(2);
            log_1.log.success(`File "${fileName}" send successfully`);
            log_1.log.info(`File size: ${fileSize} bytes`);
            log_1.log.info(`Time taken: ${duration} seconds`);
        });
    });
});
program
    .command('receive')
    .description('Receive a file from a peer')
    .requiredOption('--port <port>', 'Port to listen on', parseInt)
    .action((options) => {
    const { port } = options;
    const server = (0, net_1.createServer)((socket) => {
        let fileName = '';
        let receivedName = false;
        let fileStream;
        const startTime = Date.now();
        socket.on('data', (chunk) => {
            if (!receivedName) {
                const strChunk = chunk.toString();
                const newlineIndex = strChunk.indexOf('\n');
                if (newlineIndex !== -1) {
                    fileName = strChunk.substring(0, newlineIndex);
                    fileStream = (0, fs_1.createWriteStream)(fileName);
                    fileStream.write(chunk.slice(newlineIndex + 1));
                    socket.pipe(fileStream);
                    receivedName = true;
                    log_1.log.info(`Receiving file: ${fileName}`);
                }
            }
        });
        socket.on('end', () => {
            fileStream?.end();
            const endTime = Date.now();
            const duration = ((endTime - startTime) / 1000).toFixed(2);
            const fileSize = (0, fs_1.statSync)(fileName).size;
            log_1.log.success(`File "${fileName}" received successfully`);
            log_1.log.info(`File size: ${fileSize} bytes`);
            log_1.log.info(`Time taken: ${duration} seconds`);
        });
    });
    server.listen(port, () => {
        log_1.log.info(`Listening for incoming file on port ${port}`);
    });
});
program
    .command('ip')
    .description("Displays this computer's local IP address")
    .action(() => {
    const nets = (0, os_1.networkInterfaces)();
    const results = [];
    for (const name of Object.keys(nets)) {
        for (const net of nets[name] || []) {
            if (net.family === 'IPv4' && !net.internal) {
                results.push(net.address);
            }
        }
    }
    if (results.length === 0) {
        log_1.log.warning('No external IPv4 address found.');
    }
    else {
        const msg = `Local IP address(es): ${results.join(', ')}`;
        log_1.log.success(msg);
    }
});
program.parse(process.argv);
