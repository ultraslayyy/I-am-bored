import 'dart:io';
import 'dart:math';

void main(List<String> args) {
  final rng = Random();
  final num = rng.nextInt(101);

  while (true) {
    stdout.write('Guess: ');
    final input = stdin.readLineSync();

    if (input == null) {
      print('Please enter a valid number');
      continue;
    }
    final guess = int.tryParse(input);
    if (guess == null) {
      print('Please enter a valid number');
      continue;
    }

    if (guess > num) {
      print('Lower');
    } else if (guess < num) {
      print('Higher');
    } else {
      print('You got it! The number was $num');
      break;
    }
  }
}