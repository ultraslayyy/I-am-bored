int numbers[] = {10, 20, 30, 40};
int count = 4;

int sum_array() {
    int sum = 0;
    
    for (int i = 0; i < count; ++i) {
        sum += numbers[i];
    }

    return sum;
}