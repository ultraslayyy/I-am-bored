int fibonacci(int n) {
    if (n < 0) {
        return 0;
    }

    if (n < 2) return n;

    int a = 0;
    int b = 1;
    for (int i = 2; i <= n; ++i) {
        int c = a;
        a = b;
        b = c + b;
    }

    return b;
}