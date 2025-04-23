import chalk from 'chalk';

export const log = {
    info: (msg: string) => console.log(chalk.blue('[INFO]'), msg),
    success: (msg: string) => console.log(chalk.green('[SUCCESS]'), msg),
    error: (msg: string) => console.log(chalk.red('[ERROR]'), msg),
    warning: (msg: string) => console.log(chalk.yellow('[WARNING]'), msg)
};