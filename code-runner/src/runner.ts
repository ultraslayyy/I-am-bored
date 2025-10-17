// https://github.com/ultraslayyy/I-am-bored/tree/info/code-runner/src/runner.ts
// Please don't delete the above line, to credit me. But I can't stop you.
// runner version: 0.1.0
import fs from 'node:fs';
import path from 'node:path';
import { exec } from 'node:child_process';

export interface TestCaseOptions {
    parallel?: boolean;

    /**
     * Limit for number of simultaneous processes if parellel is true.
     * @requires parallel
     * @default 10
     */
    concurrencyLimit?: number;

    cases: TestCase[];
}

export interface TestCase {
    input: string;
    expected?: string;
}

export interface RunResult {
    stdout: string;
    stderr: string;
    exitCode: number | null;
    success: boolean;
    timeMs: number;
    memoryKb: number;
}

export interface TestCaseResult extends RunResult {
    input: string;
    expected?: string;
    passed?: boolean;
}

interface LangConfig {
    ext: string;
    compile?: string;
    run: string;
    dockerImage: string;
}

export type Lang = 'python' | 'python3' | 'javascript' | 'typescript' | 'cpp' | 'c' | 'java' | 'csharp' | 'go' | 'kotlin' | 'swift' | 'rust' | 'ruby' | 'php' | 'dart' | 'scala' | 'elixir' | 'erlang' | 'racket' | 'bash' | 'mysql' | 'ms sql' | 'postgres' | 'oracle' | 'pandas'

const LANGS: Record<Lang, LangConfig> = {
    python:     { ext: '.py', run: 'python3 {file}', dockerImage: 'python:2.7.18' },
    python3:    { ext: '.py', run: 'python {file}', dockerImage: 'python:3.14' },
    javascript: { ext: '.js', run: 'node {file}', dockerImage: 'node:20' },
    typescript: { ext: '.ts', compile: 'tsc {file}', run: 'node {base}.js', dockerImage: 'node:20' },
    cpp:        { ext: '.cpp', compile: 'g++ {file} -o {base}.out', run: './{base}.out', dockerImage: 'gcc:latest' },
    c:          { ext: '.c', compile: 'g++ {file} -o {base}.out', run: './{base}.out', dockerImage: 'gcc:latest' },
    java:       { ext: '.java', compile: 'javac {file}', run: 'java {base}', dockerImage: 'openjdk:21' },
    csharp:     { ext: '.cs', compile: 'mcs {file}', run: 'mono {base}.exe', dockerImage: 'mono:latest' },
    go:         { ext: '.go', run: 'go run {file}', dockerImage: 'golang:1.22' },
    kotlin:     { ext: '.kt', compile: 'kotlinc {file} -include-runtime -d {base}.jar', run: 'java -jar {base}.jar', dockerImage: 'openjdk:21' },
    swift:      { ext: '.swift', run: 'swift {file}', dockerImage: 'swift:5.10' },
    rust:       { ext: '.rs', compile: 'rustc {file} -o {base}.out', run: './{base}.out', dockerImage: 'rust:1.79' },
    ruby:       { ext: '.rb', run: 'ruby {file}', dockerImage: 'ruby:3.3' },
    php:        { ext: '.php', run: 'php {file}', dockerImage: 'php:8.3' },
    dart:       { ext: '.dart', run: 'dart {file}', dockerImage: 'dart:3.4' },
    scala:      { ext: '.scala', run: 'scala {file}', dockerImage: 'hseeberger/scala-sbt' },
    elixir:     { ext: '.exs', run: 'elixir {file}', dockerImage: 'elixir:1.17' },
    erlang:     { ext: '.erl', compile: 'erlc {file}', run: 'erl -noshell -s {base} main -s init stop', dockerImage: 'erlang:26' },
    racket:     { ext: '.rkt', run: 'racket {file}', dockerImage: 'racket/racket:latest' },
    bash:       { ext: '.sh', run: 'bash {file}', dockerImage: 'bash:latest' },
    mysql:      { ext: '.sql', run: "mysql -u root -e 'source {file}'", dockerImage: 'mysql:8' },
    'ms sql':   { ext: '.sql', run: 'sqlcmd -i {file}', dockerImage: 'mcr.microsoft.com/mssql/server:2022-latest' },
    postgres:   { ext: '.sql', run: 'psql -U postgres -f {file}', dockerImage: 'postgres:16' },
    oracle:     { ext: '.sql', run: "sqlplus -S user/pass@db @'{file}'", dockerImage: 'gvenzl/oracle-xe' },
    pandas:     { ext: '.py', run: 'python3 {file}', dockerImage: 'python:3.14' }
}

function formatCmd(template: string, filePath: string): string {
    const base = filePath.replace(/\.[^/.]+$/, '');
    return template.replace(/\{file\}/g, filePath).replace(/\{base\}/g, base);
}

export async function runCodeLocal({ code, language, testCases, filenamePrefix }: { code: string, language: Lang, testCases: TestCaseOptions, filenamePrefix?: string }): Promise<TestCaseResult[]>;
export async function runCodeLocal({ code, language, filenamePrefix, input }: { code: string, language: Lang, filenamePrefix?: string, input?: string }): Promise<RunResult>;
export async function runCodeLocal({ code, language, testCases, filenamePrefix, input = '' }: { code: string, language: Lang, testCases?: TestCaseOptions, filenamePrefix?: string, input?: string }): Promise<RunResult | TestCaseResult[]> {
    const lang = LANGS[language];

    const tempDir = path.resolve('temp');
    if (!fs.existsSync(tempDir)) fs.mkdirSync(tempDir, { recursive: true });
    const uniqueId = Date.now() + '-' + Math.random().toString(36).slice(2, 8);
    const filePath = path.join(tempDir, `${filenamePrefix ?? 'run'}-${uniqueId}${lang.ext}`);
    fs.writeFileSync(filePath, code);

    if (lang.compile) {
        const compileCmd = formatCmd(lang.compile, filePath);
        const compileRes = await execAsync(compileCmd);
        if (compileRes.exitCode !== 0) return testCases ? [compileRes as TestCaseResult] : compileRes;
    }

    const runCmd = formatCmd(lang.run, filePath);

    if (testCases) {
        let results: TestCaseResult[] = [];
        if (testCases.parallel) {
            const tasks = testCases.cases.map((test) => async (): Promise<TestCaseResult> => {
                const res = await execAsync(runCmd, test.input);
                const cleanedOut = res.stdout.trim();
                const expected = test.expected?.trim();
                return {
                    ...res,
                    input: test.input,
                    ...(expected ? { expected } : {}),
                    passed: expected ? cleanedOut === expected : false
                }
            });

            results = await runWithLimit(tasks, testCases.concurrencyLimit ?? 10);
        } else {
            for (const test of testCases.cases) {
                const res = await execAsync(runCmd, test.input);
                const cleanedOut = res.stdout.trim();
                const expected = test.expected?.trim();
                results.push({
                    ...res,
                    input: test.input,
                    ...(expected ? { expected } : {}),
                    passed: expected ? cleanedOut === expected : false
                });
            }
        }

        return results;
    } else {
        return execAsync(runCmd, input);
    }
}

export async function runCodeDocker({ code, language, testCases, filenamePrefix }: { code: string, language: Lang, testCases: TestCaseOptions, filenamePrefix?: string }): Promise<TestCaseResult[]>;
export async function runCodeDocker({ code, language, filenamePrefix, input }: { code: string, language: Lang, filenamePrefix?: string, input: string }): Promise<RunResult>;
export async function runCodeDocker({ code, language, testCases, filenamePrefix, input = '' }: { code: string, language: Lang, testCases?: TestCaseOptions, filenamePrefix?: string, input?: string }): Promise<RunResult | TestCaseResult[]> {
    const lang = LANGS[language];

    const customContainers: Record<string, string> = {
        typescript: 'runner_typescript',
        pandas: 'runner_pandas',
        kotlin: 'runner_kotlin',
        'ms sql': 'runner_mssql',
        oracle: 'runner_oracle'
    }

    const isCustom = language.toLowerCase() in customContainers;
    const containerOrImage = isCustom ? customContainers[language.toLowerCase()] : lang.dockerImage;

    const codeDir = path.resolve('code');
    if (!fs.existsSync(codeDir)) fs.mkdirSync(codeDir);
    const uniqueId = Date.now() + '-' + Math.random().toString(36).slice(2, 8);
    const filePath = path.join(codeDir, `${filenamePrefix ?? 'run'}-${uniqueId}${lang.ext}`);
    fs.writeFileSync(filePath, code);

    const runCmd = formatCmd(lang.run, `/app/${path.basename(filePath)}`);

    const dockerCmd = isCustom
        ? `docker exec -i ${containerOrImage} bash -c "echo \\\"$INPUT\\\" | ${runCmd}"`
        : `docker run --rm --network none --cpus=.5 -m 256m -v "${codeDir}:/app" -w /app ${containerOrImage} bash -c "echo \\\"$INPUT\\\" | ${runCmd}"`

    if (testCases) {
        let results: TestCaseResult[] = [];
        if (testCases.parallel) {
            const tasks = testCases.cases.map((test) => async (): Promise<TestCaseResult> => {
                const rawRes = await execAsync(dockerCmd, test.input);
                const res = formatDockerTimeMem(rawRes);
                const cleanedOut = res.stdout.trim();
                const expected = test.expected?.trim();
                return {
                    ...res,
                    input: test.input,
                    ...(expected ? { expected } : {}),
                    passed: expected ? cleanedOut === expected : false
                }
            });
            
            results = await runWithLimit(tasks, testCases.concurrencyLimit ?? 10);
        } else {
            for (const test of testCases.cases) {
                const rawRes = await execAsync(dockerCmd, test.input);
                const res = formatDockerTimeMem(rawRes);
                const cleanedOut = res.stdout.trim();
                const expected = test.expected?.trim();
                results.push({
                    ...res,
                    input: test.input,
                    ...(expected ? { expected } : {}),
                    passed: expected ? cleanedOut === expected : false
                });
            }
        }

        return results;
    } else {
        const result = await execAsync(dockerCmd, input);
        return formatDockerTimeMem(result);
    }

    function formatDockerTimeMem(res: RunResult) {
        let timeMs = 0;
        let memoryKb = 0;
        const timeMatch = res.stderr.match(/TIME:([\d.]+)/);
        const memMatch = res.stderr.match(/MEM:(\d+)/);
        if (timeMatch) timeMs = parseFloat(timeMatch[1]!) * 1000;
        if (memMatch) memoryKb = parseInt(memMatch[1]!);

        return { ...res, timeMs, memoryKb }
    }
}

function execAsync(cmd: string, input = ''): Promise<RunResult> {
    return new Promise((resolve) => {
        const start = process.hrtime.bigint();
        const proc = exec(cmd, { timeout: 5000 }, (error, stdout, stderr) => {
            const end = process.hrtime.bigint();
            const timeMs = Number(end - start) / 1e6;

            resolve({
                stdout: stdout.slice(0, 8192),
                stderr: stderr.slice(0, 8192),
                exitCode: (error as any)?.code ?? 0,
                success: !error,
                timeMs,
                memoryKb: Math.round(process.memoryUsage().rss / 1024)
            });
        });
        if (input) {
            proc.stdin?.write(input);
            proc.stdin?.end();
        }
    });
}

async function runWithLimit<T>(tasks: (() => Promise<T>)[], limit: number): Promise<T[]> {
    const results: T[] = [];
    let active = 0;
    let index = 0;

    return new Promise((resolve, reject) => {
        const next = () => {
            if (index === tasks.length && active === 0) return resolve(results);
            while (active < limit && index < tasks.length) {
                const currentIndex = index++;
                const task = tasks[currentIndex];
                active++;
                task?.()
                    .then(res => (results[currentIndex] = res))
                    .catch(reject)
                    .finally(() => {
                        active--;
                        next();
                    });
            }
        }
        next();
    });
}