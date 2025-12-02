#include <cmath>
#include <iostream>
#include <vector>

class Shape {
public:
    virtual double area() const = 0;

    virtual ~Shape() {}
};

class Circle : public Shape {
private:
    double radius_;
public:
    Circle(double r) : radius_(r) {}

    double area() const override {
        const double pi = std::acos(-1);
        return pi * radius_ * radius_;
    }
};

class Rectangle : public Shape {
private:
    double width_;
    double height_;
public:
    Rectangle(double w, double h) : width_(w), height_(h) {}

    double area() const override {
        return width_ * height_;
    }
};

int main() {
    std::vector<Shape*> shapes;

    shapes.push_back(new Circle(5.0));
    shapes.push_back(new Rectangle(4.0, 6.0));

    for (const Shape* s : shapes) {
        std::cout << "Calculated area: " << s->area() << std::endl;
    }

    for (Shape* s : shapes) {
        delete s;
    }

    return 0;
}