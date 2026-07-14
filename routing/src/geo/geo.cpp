#define _USE_MATH_DEFINES
#include <cmath>

double deg2rad(double deg) {
    return deg * M_PI / 180;
}

double rad2deg(double rad) {
    return rad * 180 / M_PI;
}

double haversine(double lat1, double lon1, double lat2, double lon2) {
    const double R = 6371000.0; // meters

    double dLat = deg2rad(lat2 - lat1);
    double dLon = deg2rad(lon2 - lon1);

    double a = sin(dLat / 2) * sin(dLat / 2) +
               cos(deg2rad(lat1)) * cos(deg2rad(lat2)) *
               sin(dLon / 2) * sin(dLon / 2);

    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return R * c;
}