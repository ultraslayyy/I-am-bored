// Matrix multiplication
// C = a * b
void matmul_scalar(float *a, float *b, float *c, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            float sum = 0;

            for (int k = 0; k < n; ++k) {
                sum += a[i*n+k] * b[k*n+j];
            }

            c[i*n+j] = sum;
        }
    }
}