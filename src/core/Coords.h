#pragma once

class Coords {
public:
    double x;
    double y;

    Coords() : x(0.0), y(0.0) {}

    Coords(double x, double y)
        : x(x), y(y) {}
};