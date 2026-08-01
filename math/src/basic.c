double m_abs(double x) {
    if (x < 0) {
        return -x;
    }
    return x;
}

double m_min(double a, double b) {
    if (a > b) {
        return b;
    } else {
        return a;
    }
}

double m_max(double a, double b) {
    if (a < b) {
        return b;
    } else {
        return a;
    }
}

double m_clamp(double x, double min, double max) {
    if (min < x) {
        if (x < max) {
            return x;
        }
        return max;
    } else {
        return min;
    }
}

double m_sign(double x) {
    if (x > 0) {
        return 1;
    } else if (x < 0) {
        return -1;
    } else {
        return 0;
    }
}