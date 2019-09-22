#include <test_proj/incl.hpp>
#include <test_proj/incl_second.hpp>
#include <test_proj/range.hpp>

class Class {
public:
    int Method() {
        return 0;
    }

private:
    int so_private_;
};

class OutOfLine {
public:
    int OtherMethod();
};

int OutOfLine::OtherMethod() {
    return -1;
}

class NoInlineclass {
};

int main() {
    int x = 1 + 1;
    int a;
    a = x + 2;
    if (a < 2) {
        a = 51;
    }
    return x - 2;
}
