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
const cli_progress_1 = __importDefault(require("cli-progress"));
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
    .option('--buffer-size [size]', 'Optional buffer size (in KB), default is 64', parseInt)
    .action((options) => {
    const { file, host, port, bufferSize } = options;
    const fileName = path_1.default.basename(file);
    const filePath = path_1.default.resolve(file);
    const fileSize = (0, fs_1.statSync)(filePath).size;
    const startTime = Date.now();
    const client = (0, net_1.createConnection)({ host, port }, () => {
        client.setNoDelay(true);
        const header = JSON.stringify({ fileName, fileSize }) + '\n';
        client.write(header);
        const sizeBuffer = bufferSize ? bufferSize : 64;
        const stream = (0, fs_1.createReadStream)(filePath, { highWaterMark: sizeBuffer * 1024 });
        const progressBar = new cli_progress_1.default.SingleBar({
            format: 'Sending {bar} | {percentage}% | {value}/{total} bytes | Speed: {speed} | ETA: {eta}s',
            hideCursor: true
        }, cli_progress_1.default.Presets.shades_classic);
        let bytesSent = 0;
        let lastBytes = 0;
        let lastTime = Date.now();
        progressBar.start(fileSize, 0, {
            speed: '0 MB/s',
            eta: 'Calculating...'
        });
        const updateStats = () => {
            const now = Date.now();
            const timeDiff = (now - lastTime) / 1000;
            const bytesDiff = bytesSent - lastBytes;
            const speed = bytesDiff / timeDiff;
            const remaining = fileSize - bytesSent;
            const eta = speed > 0 ? (remaining / speed).toFixed(1) : '∞';
            progressBar.update(bytesSent, {
                speed: `${(speed / (1024 * 1024)).toFixed(2)} MB/s`,
                eta
            });
            lastTime = now;
            lastBytes = bytesSent;
        };
        const statInterval = setInterval(updateStats, 500);
        stream.on('data', (chunk) => {
            bytesSent += chunk.length;
        });
        stream.pipe(client, { end: false });
        stream.on('end', () => {
            client.end();
            clearInterval(statInterval);
            updateStats();
            progressBar.stop();
            const endTime = Date.now();
            log_1.log.success(`File "${fileName}" send successfully`);
            log_1.log.info(`File size: ${fileSize} bytes`);
            log_1.log.info(`Time taken: ${((endTime - startTime) / 1000).toFixed(2)} seconds`);
        });
    });
});
program
    .command('receive')
    .description('Receive a file from a peer')
    .requiredOption('--port <port>', 'Port to listen on', parseInt)
    .option('--out-dir [directory]', 'Optional output directory')
    .option('--buffer-size [size]', 'Optional buffer size (in KB), default is 64', parseInt)
    .action((options) => {
    const { port, outDir, bufferSize } = options;
    const server = (0, net_1.createServer)((socket) => {
        let fileName = '';
        let receivedBytes = 0;
        let fileSize = 0;
        let fileStream;
        const startTime = Date.now();
        let progressBar;
        let lastBytes = 0;
        let lastTime = Date.now();
        const updateStats = () => {
            const now = Date.now();
            const timeDiff = (now - lastTime) / 1000;
            const bytesDiff = receivedBytes - lastBytes;
            const speed = bytesDiff / timeDiff;
            const remaining = fileSize - receivedBytes;
            const eta = speed > 0 ? (remaining / speed).toFixed(1) : '∞';
            progressBar?.update(receivedBytes, {
                speed: `${(speed / (1024 * 1024)).toFixed(2)} MB/s`,
                eta
            });
        };
        const statInterval = setInterval(updateStats, 500);
        let buffer = '';
        socket.on('data', (chunk) => {
            if (!fileStream) {
                buffer += chunk.toString();
                const newlineIndex = buffer.indexOf('\n');
                if (newlineIndex !== -1) {
                    const header = buffer.slice(0, newlineIndex);
                    const remaining = chunk.slice(Buffer.byteLength(header) + 1);
                    try {
                        const parsed = JSON.parse(header);
                        fileName = parsed.fileName;
                        fileSize = parsed.fileSize;
                        const outputPath = outDir ? path_1.default.join(outDir, fileName) : fileName;
                        const sizeBuffer = bufferSize ? bufferSize : 64;
                        fileStream = (0, fs_1.createWriteStream)(outputPath, { highWaterMark: sizeBuffer * 1024 });
                        progressBar = new cli_progress_1.default.SingleBar({
                            format: `Receiving {bar} | {percentage}% | {value}/{total} bytes | Speed: {speed} | ETA: {eta}s`,
                            hideCursor: true
                        }, cli_progress_1.default.Presets.shades_classic);
                        progressBar.start(fileSize, 0, {
                            speed: '0 MB/s',
                            eta: 'Calculating...'
                        });
                        log_1.log.info(`Receiving file: ${fileName} (${fileSize} bytes)`);
                        fileStream.write(remaining);
                        receivedBytes += remaining.length;
                        socket.pipe(fileStream, { end: true });
                        socket.resume();
                    }
                    catch (err) {
                        log_1.log.error(`Failed to parse file header: ${err}`);
                        socket.destroy();
                        clearInterval(statInterval);
                    }
                }
            }
            else {
                receivedBytes += chunk.length;
            }
        });
        socket.on('end', () => {
            fileStream?.end();
            clearInterval(statInterval);
            updateStats();
            progressBar?.stop();
            const endTime = Date.now();
            log_1.log.success(`File "${fileName}" received successfully`);
            log_1.log.info(`File size: ${fileSize} bytes`);
            log_1.log.info(`Time taken: ${((endTime - startTime) / 1000).toFixed(2)} seconds`);
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
