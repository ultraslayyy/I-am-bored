import { Command } from 'commander';
import { createServer, createConnection } from 'net';
import { createReadStream, createWriteStream, statSync } from 'fs';
import { networkInterfaces } from 'os';
import { log } from './log';
import path from 'path';

const program = new Command();

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
        const fileName = path.basename(file);
        const filePath = path.resolve(file);
        const fileSize = statSync(filePath).size;

        const startTime = Date.now();

        const client = createConnection({ host, port }, () => {
            client.write(fileName + '\n');
            const stream = createReadStream(file);
            stream.pipe(client);
            log.info(`Sending file: ${fileName}`);

            stream.on('end', () => {
                const endTime = Date.now();
                const duration = ((endTime - startTime) / 1000).toFixed(2);
                log.success(`File "${fileName}" send successfully`);
                log.info(`File size: ${fileSize} bytes`);
                log.info(`Time taken: ${duration} seconds`);
            });
        });
    });

program
    .command('receive')
    .description('Receive a file from a peer')
    .requiredOption('--port <port>', 'Port to listen on', parseInt)
    .action((options) => {
        const { port } = options;
        const server = createServer((socket) => {
            let fileName = '';
            let receivedName = false;
            let fileStream: ReturnType<typeof createWriteStream>;
            const startTime = Date.now();

            socket.on('data', (chunk) => {
                if (!receivedName) {
                    const strChunk = chunk.toString();
                    const newlineIndex = strChunk.indexOf('\n');
                    if (newlineIndex !== -1) {
                        fileName = strChunk.substring(0, newlineIndex);
                        fileStream = createWriteStream(fileName);
                        fileStream.write(chunk.slice(newlineIndex + 1));
                        socket.pipe(fileStream);
                        receivedName = true;

                        log.info(`Receiving file: ${fileName}`);
                    }
                }
            });

            socket.on('end', () => {
                fileStream?.end();
                const endTime = Date.now();
                const duration = ((endTime - startTime) / 1000).toFixed(2);
                const fileSize = statSync(fileName).size;

                log.success(`File "${fileName}" received successfully`);
                log.info(`File size: ${fileSize} bytes`);
                log.info(`Time taken: ${duration} seconds`);
            });
        });

        server.listen(port, () => {
            log.info(`Listening for incoming file on port ${port}`);
        });
    });

program
    .command('ip')
    .description("Displays this computer's local IP address")
    .action(() => {
        const nets = networkInterfaces();
        const results: string[] = [];

        for (const name of Object.keys(nets)) {
            for (const net of nets[name] || []) {
                if (net.family === 'IPv4' && !net.internal) {
                    results.push(net.address);
                }
            }
        }

        if (results.length === 0) {
            log.warning('No external IPv4 address found.');
        } else {
            const msg = `Local IP address(es): ${results.join(', ')}`;
            log.success(msg);
        }
    })

program.parse(process.argv);