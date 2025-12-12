import readline from 'node:readline';

async function main() {
    const rl = readline.createInterface({
        input: process.stdin,
        output: process.stdout
    });

    const num = Math.floor(Math.random() * 100) + 1;
    let guess;
    
    while (true) {
        await rl.question('Guess: ', (data) => guess = data);

        if (isNaN(Number(guess))) {
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