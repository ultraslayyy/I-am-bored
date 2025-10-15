import fs from 'node:fs';
import path from 'node:path';
import { exec, spawn } from 'node:child_process';

export interface RunResult {
    stdout: string;
    stderr: string;
    exitCode: number | null;
    success: boolean;
}

interface LangConfig {
    ext: string;
    compile?: string;
    run: string;
    dockerImage: string;
}

const LANGS: Record<string, LangConfig> = {
    python:     { ext: '.py', run: 'python3 {file}', dockerImage: 'python:3.14' },
    python3:    { ext: '.py', run: 'python {file}', dockerImage: 'python:2.7.18' },
    javascript: { ext: '.js', run: 'node {file}', dockerImage: 'node:20' },
    typescript: { ext: '.ts', compile: 'tsc {file}', run: 'node {base}.js', dockerImage: 'node:20' }, // Custom runner_typescript image
    cpp:        { ext: '.cpp', compile: 'g++ {file} -o {base}.out', run: './{base}.out', dockerImage: 'gcc:latest' },
    c:          { ext: '.c', compile: 'g++ {file} -o {base}.out', run: './{base}.out', dockerImage: 'gcc:latest' },
    java:       { ext: '.java', compile: 'javac {file}', run: 'java {base}', dockerImage: 'openjdk:21' },
    csharp:     { ext: '.cs', compile: 'mcs {file}', run: 'mono {base}.exe', dockerImage: 'mono:latest' },
    go:         { ext: '.go', run: 'go run {file}', dockerImage: 'golang:1.22' },
    kotlin:     { ext: '.kt', compile: 'kotlinc {file} -include-runtime -d {base}.jar', run: 'java -jar {base}.jar', dockerImage: 'openjdk:21' }, // Custom runner_kotlin image
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
    mysql:      { ext: '.sql', run: 'mysql -u root -e \'source {file}\'', dockerImage: 'mysql:8' },
    'ms sql':   { ext: '.sql', run: 'sqlcmd -i {file}', dockerImage: 'mcr.microsoft.com/mssql/server:2022-latest' }, // Custom runner_mssql image
    postgres:   { ext: '.sql', run: 'psql -U postgres -f {file}', dockerImage: 'postgres:16' },
    oracle:     { ext: '.sql', run: "sqlplus -S user/pass@db @'{file}'", dockerImage: 'gvenzl/oracle-xe' }, // Custom runner_oracle image
    pandas:     { ext: '.py', run: 'python3 {file}', dockerImage: 'python:3.14' }, // Custom runner_pandas image
}

function formatCmd(template: string, filePath: string): string {
    const base = filePath.replace(/\.[^/.]+$/, '');
    return template.replace(/\{file\}/g, filePath).replace(/\{base\}/g, base);
}

export async function runCodeLocal(code: string, language: string, input = '', filenamePrefix?: string): Promise<RunResult> {
    const lang = LANGS[language.toLowerCase()];
    if (!lang) throw new Error(`Unsupported language: ${language}`);

    const tempDir = path.resolve('temp');
    if (!fs.existsSync(tempDir)) fs.mkdirSync(tempDir);
    const filePath = path.join(tempDir, `${filenamePrefix ? `${filenamePrefix} -` : ''}main${lang.ext}`);
    fs.writeFileSync(filePath, code);

    if (lang.compile) {
        const compileCmd = formatCmd(lang.compile, filePath);
        const compileRes = await execAsync(compileCmd);
        if (compileRes.exitCode !== 0) return compileRes;
    }

    const runCmd = formatCmd(lang.run, filePath);
    return execAsync(runCmd, input);
}

export async function runCodeDocker(code: string, language: string, input = '', filenamePrefix?: string) {
    const lang = LANGS[language.toLowerCase()];
    if (!lang) throw new Error(`Unsupported language: ${language}`);

    const customContainers: Record<string, string> = {
        typescript: 'runner_typescript',
        pandas: 'runner_pandas',
        kotlin: 'runner_kotlin',
        'ms sql': 'runner_mssql',
        oracle: 'runner_oracle'
    }

    const containerName = language.toLowerCase() in customContainers ? customContainers[language.toLowerCase()] : lang.dockerImage;

    const isCustom = language.toLowerCase() in customContainers;
    const containerOrImage = isCustom ? customContainers[language.toLowerCase()] : lang.dockerImage;

    const codeDir = path.resolve('code');
    if (!fs.existsSync(codeDir)) fs.mkdirSync(codeDir);
    const filePath = path.join(codeDir, `${filenamePrefix ? `${filenamePrefix} -` : ''}main${lang.ext}`);
    fs.writeFileSync(filePath, code);
    
    const runCmd = formatCmd(lang.run, `/app/${path.basename(filePath)}`);

    const dockerCmd = isCustom
        ? `docker exec -i ${containerOrImage} bash -c "echo \\\"$INPUT\\\" | ${runCmd}"`
        : `docker run --rm --network none --cpus=.5 -m 256m -v "${codeDir}:/app" -w /app ${containerOrImage} bash -c "echo \\\"$INPUT\\\" | ${runCmd}"`

    return execAsync(dockerCmd, input);
}

function execAsync(cmd: string, input = ''): Promise<RunResult> {
    return new Promise((resolve) => {
        const proc = exec(cmd, { timeout: 5000 }, (error, stdout, stderr) => {
            resolve({
                stdout,
                stderr,
                exitCode: (error as any)?.code ?? 0,
                success: !error
            });
        });
        if (input) {
            proc.stdin?.write(input);
            proc.stdin?.end();
        }
    });
}