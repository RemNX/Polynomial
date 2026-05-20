#ifndef TYPES_H
#define TYPES_H

namespace MathConstants {
    constexpr double ROOT_PRECISION = 1e-10;
    constexpr int MAX_ROOT_LOOP = 50;
    constexpr double EPSILON = 1e-12;
    constexpr double ZERO_THRESHOLD = 1e-8;
}

namespace UIConstants {
    constexpr int MAX_DEGREE = 6;
    constexpr int NUMBER_POINTS = 1000;
    constexpr double DEFAULT_X_MIN = -4.0;
    constexpr double DEFAULT_X_MAX = 4.0;
    constexpr double DEFAULT_Y_MARGIN_FACTOR = 1.05;
    constexpr double ZOOM_MARGIN_FACTOR = 0.01;
}

#endif // TYPES_H
