void mat_vex_mult(int *matrix, int *vector, int *result) {
    for (int i = 0; i < 2; ++i) {
        int sum = 0;
        for (int j = 0; j < 2; ++j) {
            sum += matrix[i * 2 + j] * vector[j];
        }
        result[i] = sum;
    }
}