#include <stdio.h>

// Simple function with no control flow
int simple_function(int x) {
    return x + 1;
}

// Function with if statement
int conditional_function(int x) {
    if (x > 0) {
        return x * 2;
    } else {
        return 0;
    }
}

// Function with loop
int loop_function(int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += i;
    }
    return sum;
}

// Function with multiple conditions
int complex_function(int a, int b, int c) {
    if (a > 0 && b > 0) {
        if (c > 0) {
            return a + b + c;
        } else {
            return a + b;
        }
    } else if (a > 0 || b > 0) {
        return a + b;
    } else {
        return 0;
    }
}