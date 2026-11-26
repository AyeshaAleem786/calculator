/*
 * Scientific-style Calculator in C
 * Author: Ayesha Aleem
 *
 * Features:
 *  - Add, subtract, multiply, divide, modulus, power
 *  - Square root, factorial
 *  - Input validation (no crash on wrong input)
 *  - History of the last 10 calculations
 *
 * Compile:  gcc calculator.c -o calculator -lm
 * Run:      ./calculator        (Windows: calculator.exe)
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define HISTORY_SIZE 10
#define LINE_SIZE 64

/* ---------- History storage ---------- */
char history[HISTORY_SIZE][LINE_SIZE];
int history_count = 0;

void add_to_history(const char *entry) {
    if (history_count == HISTORY_SIZE) {
        /* shift everything up, drop the oldest */
        for (int i = 1; i < HISTORY_SIZE; i++) {
            memcpy(history[i - 1], history[i], LINE_SIZE);
        }
        history_count--;
    }
    snprintf(history[history_count++], LINE_SIZE, "%s", entry);
}

void show_history(void) {
    if (history_count == 0) {
        printf("\nNo calculations yet.\n");
        return;
    }
    printf("\n--- History (oldest to newest) ---\n");
    for (int i = 0; i < history_count; i++) {
        printf("%d. %s\n", i + 1, history[i]);
    }
}

/* ---------- Safe input helpers ---------- */
void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        /* discard */
    }
}

double read_number(const char *prompt) {
    double n;
    while (1) {
        printf("%s", prompt);
        if (scanf("%lf", &n) == 1) {
            clear_input_buffer();
            return n;
        }
        if (feof(stdin)) {
            printf("\nInput ended. Exiting.\n");
            exit(0);
        }
        printf("Invalid input. Please enter a number.\n");
        clear_input_buffer();
    }
}

int read_choice(void) {
    int ch;
    while (1) {
        printf("Enter your choice: ");
        if (scanf("%d", &ch) == 1) {
            clear_input_buffer();
            return ch;
        }
        if (feof(stdin)) {
            printf("\nInput ended. Exiting.\n");
            exit(0);
        }
        printf("Invalid choice. Please enter a number from the menu.\n");
        clear_input_buffer();
    }
}

/* ---------- Calculator operations ---------- */
double add(double a, double b)      { return a + b; }
double subtract(double a, double b) { return a - b; }
double multiply(double a, double b) { return a * b; }

/* returns 1 on success, 0 on error (division by zero) */
int divide(double a, double b, double *result) {
    if (b == 0) return 0;
    *result = a / b;
    return 1;
}

/* modulus works on whole numbers; returns 0 on error */
int modulus(double a, double b, long long *result) {
    long long x = (long long)a;
    long long y = (long long)b;
    if (y == 0) return 0;
    *result = x % y;
    return 1;
}

/* factorial for 0..20 (21! overflows unsigned long long) */
int factorial(double value, unsigned long long *result) {
    if (value < 0 || value > 20 || value != floor(value)) return 0;
    unsigned long long f = 1;
    for (int i = 2; i <= (int)value; i++) {
        f *= i;
    }
    *result = f;
    return 1;
}

/* ---------- Menu ---------- */
void print_menu(void) {
    printf("\n=========== CALCULATOR ===========\n");
    printf(" 1. Addition        (a + b)\n");
    printf(" 2. Subtraction     (a - b)\n");
    printf(" 3. Multiplication  (a * b)\n");
    printf(" 4. Division        (a / b)\n");
    printf(" 5. Modulus         (a %% b, whole numbers)\n");
    printf(" 6. Power           (a ^ b)\n");
    printf(" 7. Square root\n");
    printf(" 8. Factorial       (0 to 20)\n");
    printf(" 9. Show history\n");
    printf(" 0. Exit\n");
    printf("==================================\n");
}

int main(void) {
    int choice;
    double a, b, result;
    char entry[LINE_SIZE];

    printf("Welcome to the C Calculator!\n");

    while (1) {
        print_menu();
        choice = read_choice();

        if (choice == 0) {
            printf("Thank you for using the calculator. Goodbye!\n");
            break;
        }

        switch (choice) {
            case 1:
                a = read_number("Enter first number: ");
                b = read_number("Enter second number: ");
                result = add(a, b);
                printf("Result: %g + %g = %g\n", a, b, result);
                snprintf(entry, LINE_SIZE, "%g + %g = %g", a, b, result);
                add_to_history(entry);
                break;

            case 2:
                a = read_number("Enter first number: ");
                b = read_number("Enter second number: ");
                result = subtract(a, b);
                printf("Result: %g - %g = %g\n", a, b, result);
                snprintf(entry, LINE_SIZE, "%g - %g = %g", a, b, result);
                add_to_history(entry);
                break;

            case 3:
                a = read_number("Enter first number: ");
                b = read_number("Enter second number: ");
                result = multiply(a, b);
                printf("Result: %g * %g = %g\n", a, b, result);
                snprintf(entry, LINE_SIZE, "%g * %g = %g", a, b, result);
                add_to_history(entry);
                break;

            case 4:
                a = read_number("Enter dividend: ");
                b = read_number("Enter divisor: ");
                if (divide(a, b, &result)) {
                    printf("Result: %g / %g = %g\n", a, b, result);
                    snprintf(entry, LINE_SIZE, "%g / %g = %g", a, b, result);
                    add_to_history(entry);
                } else {
                    printf("Error: Division by zero is not allowed.\n");
                }
                break;

            case 5: {
                long long r;
                a = read_number("Enter first number: ");
                b = read_number("Enter second number: ");
                if (modulus(a, b, &r)) {
                    printf("Result: %lld %% %lld = %lld\n",
                           (long long)a, (long long)b, r);
                    snprintf(entry, LINE_SIZE, "%lld %% %lld = %lld",
                             (long long)a, (long long)b, r);
                    add_to_history(entry);
                } else {
                    printf("Error: Modulus by zero is not allowed.\n");
                }
                break;
            }

            case 6:
                a = read_number("Enter base: ");
                b = read_number("Enter exponent: ");
                result = pow(a, b);
                if (isnan(result) || isinf(result)) {
                    printf("Error: Result is undefined or too large.\n");
                } else {
                    printf("Result: %g ^ %g = %g\n", a, b, result);
                    snprintf(entry, LINE_SIZE, "%g ^ %g = %g", a, b, result);
                    add_to_history(entry);
                }
                break;

            case 7:
                a = read_number("Enter a number: ");
                if (a < 0) {
                    printf("Error: Square root of a negative number is not real.\n");
                } else {
                    result = sqrt(a);
                    printf("Result: sqrt(%g) = %g\n", a, result);
                    snprintf(entry, LINE_SIZE, "sqrt(%g) = %g", a, result);
                    add_to_history(entry);
                }
                break;

            case 8: {
                unsigned long long f;
                a = read_number("Enter a whole number (0-20): ");
                if (factorial(a, &f)) {
                    printf("Result: %d! = %llu\n", (int)a, f);
                    snprintf(entry, LINE_SIZE, "%d! = %llu", (int)a, f);
                    add_to_history(entry);
                } else {
                    printf("Error: Enter a whole number between 0 and 20.\n");
                }
                break;
            }

            case 9:
                show_history();
                break;

            default:
                printf("Invalid choice! Please select from the menu.\n");
        }
    }

    return 0;
}
