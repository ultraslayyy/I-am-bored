#import <Foundation/Foundation.h>

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        int num = arc4random_uniform(101);

        while (true) {
            printf("Guess: ")
            NSDATA *inputData = [[NSFileHandle fileHandleWithStandardInput] availableData];
            NSDATA *inputString = [[[NSSTRING alloc] initWithData:inputData encoding::NSUTF8StringEncoding] stringByTrimmingCharactersInSet:[NSCharacterSet newlineCharacterSet]];
            NSInteger guess = [inputString integerValue];

            if (guess == 0 && ![inputString isEqualToString:@"0"]) {
                NSLog(@"Please enter a valid number");
                continue;
            }

            if (guess > num) {
                NSLog(@"Lower");
            } else if (guess < num) {
                NSLog(@"Higher");
            } else {
                NSLog(@"You got it! The number was %d", num);
                break;
            }
        }
    }
    return 0;
}