/*
 * BODMAS Calculator in C  (expression evaluator)
 * Author: Ayesha Aleem
 *
 * Type a full expression like:  5 + 3 * 2          -> 11
 *                               (5 + 3) * 2        -> 16
 *                               2 ^ 3 ^ 2 + sqrt(16) / 2
 *
 * BODMAS order followed:
 *   B - Brackets           ( )
 *   O - Orders             ^ and sqrt()
 *   D/M - Division, Multiplication, Modulus   / * %   (left to right)
 *   A/S - Addition, Subtraction               + -     (left to right)
 *
 * How it works (recursive descent parser):
 *   Each precedence level has its own function. Lower-priority
 *   operators (+ -) call higher-priority ones (* /), which call
 *   even higher ones (^), which call brackets/numbers. This is
 *   exactly what makes the evaluation respect BODMAS.
 *
 * Compile:  gcc calculator_bodmas.c -o bodmas -lm
 * Run:      ./bodmas
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define INPUT_SIZE 256

static const char *p;        /* current position in the expression */
static int error_flag;       /* 1 if something went wrong          */
static char error_msg[80];   /* what went wrong                    */

/* Remember only the FIRST error */
static void set_error(const char *msg) {
    if (!error_flag) {
        error_flag = 1;
        snprintf(error_msg, sizeof(error_msg), "%s", msg);
    }
}

static void skip_spaces(void) {
    while (isspace((unsigned char)*p)) p++;
}

/* Forward declaration because brackets call back into parse_expr */
static double parse_expr(void);

/* ---- Level 1: numbers, ( brackets ), sqrt( ) ---- */
static double parse_primary(void) {
    skip_spaces();

    /* Bracket: ( expression ) */
    if (*p == '(') {
        p++;
        double v = parse_expr();
        skip_spaces();
        if (*p == ')') p++;
        else set_error("Missing closing bracket ')'");
        return v;
    }

    /* sqrt( expression ) */
    if (strncmp(p, "sqrt", 4) == 0) {
        p += 4;
        skip_spaces();
        if (*p != '(') {
            set_error("sqrt needs brackets, e.g. sqrt(16)");
            return 0;
        }
        p++;
        double v = parse_expr();
        skip_spaces();
        if (*p == ')') p++;
        else set_error("Missing closing bracket ')'");
        if (v < 0) {
            set_error("Square root of a negative number");
            return 0;
        }
        return sqrt(v);
    }

    /* Number: 12, 3.5, .5, 1e3 */
    if (isdigit((unsigned char)*p) || *p == '.') {
        char *end;
        double v = strtod(p, &end);
        p = end;
        return v;
    }

    if (*p == '\0') set_error("Expression ended unexpectedly");
    else            set_error("Expected a number or '('");
    return 0;
}

static double parse_unary(void);

/* ---- Level 2: power  a ^ b  (right to left: 2^3^2 = 2^9) ---- */
static double parse_power(void) {
    double base = parse_primary();
    skip_spaces();
    if (*p == '^') {
        p++;
        double exponent = parse_unary();   /* allows 2^-3 */
        double r = pow(base, exponent);
        if (isnan(r) || isinf(r)) set_error("Power result is undefined or too large");
        return r;
    }
    return base;
}

/* ---- Level 3: unary minus/plus  (so -2^2 = -4, as in maths) ---- */
static double parse_unary(void) {
    skip_spaces();
    if (*p == '-') { p++; return -parse_unary(); }
    if (*p == '+') { p++; return  parse_unary(); }
    return parse_power();
}

/* ---- Level 4: * / %  (left to right) ---- */
static double parse_term(void) {
    double v = parse_unary();
    while (!error_flag) {
        skip_spaces();
        char op = *p;
        if (op != '*' && op != '/' && op != '%') break;
        p++;
        double r = parse_unary();
        if (op == '*') {
            v *= r;
        } else if (r == 0) {
            set_error(op == '/' ? "Division by zero" : "Modulus by zero");
        } else if (op == '/') {
            v /= r;
        } else {
            v = fmod(v, r);
        }
    }
    return v;
}

/* ---- Level 5: + -  (left to right, lowest priority) ---- */
static double parse_expr(void) {
    double v = parse_term();
    while (!error_flag) {
        skip_spaces();
        if (*p != '+' && *p != '-') break;
        char op = *p++;
        double r = parse_term();
        v = (op == '+') ? v + r : v - r;
    }
    return v;
}

/* Evaluate a whole line. Returns 1 on success, 0 on error. */
static int evaluate(const char *text, double *result) {
    p = text;
    error_flag = 0;
    error_msg[0] = '\0';

    double v = parse_expr();
    skip_spaces();

    if (!error_flag && *p != '\0') {
        if (*p == ')') set_error("Extra closing bracket ')'");
        else           set_error("Unexpected character in expression");
    }
    if (!error_flag && (isnan(v) || isinf(v))) set_error("Result is undefined or too large");

    *result = v;
    return !error_flag;
}

static void print_help(void) {
    printf("\nSupported:  +  -  *  /  %%  ^  ( )  sqrt( )\n");
    printf("Examples:   5 + 3 * 2        = 11\n");
    printf("            (5 + 3) * 2      = 16\n");
    printf("            2 ^ 3 ^ 2        = 512\n");
    printf("            -2 ^ 2           = -4\n");
    printf("            sqrt(16) + 10 / 4 = 6.5\n");
    printf("Commands:   help, ans (last answer), exit\n\n");
}

/* Replace the word "ans" in the input with the last answer */
static void substitute_ans(const char *in, char *out, size_t size, double last) {
    size_t o = 0;
    for (const char *s = in; *s && o + 1 < size; ) {
        if (strncmp(s, "ans", 3) == 0) {
            int n = snprintf(out + o, size - o, "(%.15g)", last);
            if (n < 0 || (size_t)n >= size - o) break;
            o += (size_t)n;
            s += 3;
        } else {
            out[o++] = *s++;
        }
    }
    out[o] = '\0';
}

int main(void) {
    char line[INPUT_SIZE];
    char expr[INPUT_SIZE * 2];
    double result, last_answer = 0;

    printf("=== BODMAS Calculator ===\n");
    printf("Type an expression, 'help' for examples, 'exit' to quit.\n");

    while (1) {
        printf("\n> ");
        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\nGoodbye!\n");
            break;
        }

        /* strip the newline */
        line[strcspn(line, "\r\n")] = '\0';

        /* skip empty lines */
        const char *t = line;
        while (isspace((unsigned char)*t)) t++;
        if (*t == '\0') continue;

        if (strcmp(t, "exit") == 0 || strcmp(t, "quit") == 0) {
            printf("Goodbye!\n");
            break;
        }
        if (strcmp(t, "help") == 0) {
            print_help();
            continue;
        }

        substitute_ans(t, expr, sizeof(expr), last_answer);

        if (evaluate(expr, &result)) {
            printf("= %.10g\n", result);
            last_answer = result;
        } else {
            printf("Error: %s\n", error_msg);
        }
    }
    return 0;
}
