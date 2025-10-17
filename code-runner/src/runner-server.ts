// https://github.com/ultraslayyy/i-am-bored/code-runner/src/runner-server.ts
// Please don't delete the above line, to credit me. But I can't stop you.
import net from 'node:net';
import { runCodeLocal, runCodeDocker, type TestCaseOptions, type RunResult, type TestCaseResult, type Lang } from './runner.js';

const PORT = 4000;

const server = net.createServer(socket => {
    let buffer = '';

    socket.on('data', chunk => {
        buffer += chunk.toString();

        let boundary: number;
        while ((boundary = buffer.indexOf('\n')) !== -1) {
            const raw = buffer.slice(0, boundary);
            buffer = buffer.slice(boundary + 1);

            try {
                const req = JSON.parse(raw);
                if (req.language) req.language = req.language.toLowerCase();
                
                handleRequest(req)
                    .then(res => socket.write(JSON.stringify(res) + '\n'))
                    .catch(err => socket.write(JSON.stringify({ success: false, error: err.message }) + '\n'));
            } catch {
                socket.write(JSON.stringify({ success: false, error: 'Invalid JSON' }) + '\n');
            }
        }
    });
});

server.listen(PORT, () => {
    console.log(`Runner TCP server listening on port ${PORT}`);
});

interface RunnerRequest {
    code: string;
    language: Lang;
    input?: string;
    testCases?: TestCaseOptions;
    runner?: string;
    filenamePrefix?: string;
}

async function handleRequest(req: RunnerRequest): Promise<{ success: boolean, result?: RunResult | TestCaseResult[], error?: string }> {
    try {
        let result: RunResult | TestCaseResult[];

        if (req.runner === 'docker') {
            if (req.testCases) {
                result = await runCodeDocker({ code: req.code, language: req.language, testCases: req.testCases, filenamePrefix: req.filenamePrefix ?? '' });
            } else {
                result = await runCodeDocker({ code: req.code, language: req.language, input: req.input ?? '', filenamePrefix: req.filenamePrefix ?? '' });
            }
        } else {
            if (req.testCases) {
                result = await runCodeLocal({ code: req.code, language: req.language, testCases: req.testCases, filenamePrefix: req.filenamePrefix ?? '' });
            } else {
                result = await runCodeLocal({ code: req.code, language: req.language, input: req.input ?? '', filenamePrefix: req.filenamePrefix ?? '' });
            }
        }

        return { success: true, result };
    } catch (err: any) {
        return { success: false, error: err.message };
    }
}