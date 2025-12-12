import readline from 'node:readline';

async function main() {
    const rl = readline.createInterface({
        input: process.stdin,
        output: process.stdout
    });

    const num = Math.floor(Math.random() * 100) + 1;
    let guess: number = 0;
    
    while (true) {
        await rl.question('Guess: ', (data) => guess = Number(data));

        if (isNaN(guess)) {
            console.log('You must enter a valid number');
            continue;
        }

        if (guess > num) {
            console.log('Lower');
        } else if (guess < num) {
            console.log('Higher');
        } else {
            console.log(`You got it! The number was ${num}`);
            break;
        }
    }
}

main();