/************************ calc.c ************************
 * Author: Jeremy Lawrence
 * 
 * This file contains the implementation of a calculator.
 * In order to run this program, GTK4 must be downloaded.
 * This can be downloaded at https://www.gtk.org/
 * 
 *******************************************************/

#include <gtk/gtk.h> /* GTK Toolkit (version 4 required) */
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

/* Constants defining floating point precision */
#define TOL 0.0000001
#define TOT_DIGITS 12
#define MAX_PRECISION 7

#define pi 3.141592654

/* Enum representing binary operations, and default (no operation) */
typedef enum {
    DIV, MUL, ADD, SUB, DEFAULT
} operator;

/* Performs binary operation on a and b */
#define bin_op(a, op, b) \
    (((op) == DIV) ? ((a) / (b)) : \
     ((op) == MUL) ? ((a) * (b)) : \
     ((op) == ADD) ? ((a) + (b)) : \
     ((op) == SUB) ? ((a) - (b)) : b)

/* Given an operator, returns the ASCII character representing it, as a
 * string, or the string "\0" if the operator is invalid or DEFAULT */
#define op_to_str(op) \
    (((op) == DIV) ? ("÷") : \
     ((op) == MUL) ? ("\u00D7") : \
     ((op) == ADD) ? ("+") : \
     ((op) == SUB) ? ("-") : "\0")

/* Given a string, returns the operator it represents. If the string
 * does not represent an operator, returns the default operator */
#define str_to_op(str) \
    ((strcmp((str), ("÷")) == 0) ? (DIV) : \
     (strcmp((str), ("\u00D7")) == 0) ? (MUL) : \
     (strcmp((str), ("+")) == 0) ? (ADD) : \
     (strcmp((str), ("-")) == 0) ? (SUB) : DEFAULT)

/* Enum representing special unary operations */
typedef enum {
    FAC, SQT, CBT, SGN, PCT, SQR, CUB, SIN, COS, TAN, NUL
} special;

/* Performs binary operation on a and b */
#define un_op(a, op) \
    (((op) == FAC) ? (tgamma((a) + 1)) : \
     ((op) == SQT) ? (sqrt(a)) : \
     ((op) == CBT) ? (cbrt(a)) : \
     ((op) == SGN) ? (0 - (a)) : \
     ((op) == PCT) ? ((a) / (float)100) : \
     ((op) == SQR) ? ((a) * (a)) : \
     ((op) == CUB) ? ((a) * (a) * (a)) : \
     ((op) == SIN) ? (sin(a)) : \
     ((op) == COS) ? (cos(a)) : \
     ((op) == TAN) ? (tan(a)) : 0)

#define str_to_special(str) \
    ((strcmp((str), ("x!")) == 0) ? (FAC) : \
     (strcmp((str), ("\u221Ax")) == 0) ? (SQT) : \
     (strcmp((str), ("\u221Bx")) == 0) ? (CBT) : \
     (strcmp((str), ("+/-")) == 0) ? (SGN) : \
     (strcmp((str), ("%")) == 0) ? (PCT) : \
     (strcmp((str), ("x²")) == 0) ? (SQR) : \
     (strcmp((str), ("x³")) == 0) ? (CUB) : \
     (strcmp((str), ("sin")) == 0) ? (SIN) : \
     (strcmp((str), ("cos")) == 0) ? (COS) : \
    (strcmp((str), ("tan")) == 0) ? (TAN) : NUL)
     

/* Object storing information about the calculator's current state */
typedef struct Data {
    /* Is true if calculator is in decimal fraction input mode, for
     * example when the user is entering 2.55. False otherwise. */
    bool decimal;

    /* In decimal fraction input mode, this holds the number of digits after
     * the decimal point. Is used to display the correct number digits. This
     * number is zero if not in decimal fraction input mode. */
    int decimals;

    /* Current operation being performed. For example, if the user inputs
     * "2 + 2 =", then op = DEFAULT until "+" is pressed, at which point
     * op = ADD until "=" is entered. */
    operator op;

    double num;    /* Stores the current number being entered */
    double result; /* Result of operations since the last "Clear" or "=" */

    GtkWidget *f;  /* Frame object acting as calculator's display screen */
} Data;

/* Displays given string on calculator using more concise syntax */
static void display_str(Data *data, char *display)
{
    gtk_frame_set_label(GTK_FRAME(data->f), (const char *)display);
}

/* Gets the string from the calculator display */
static char *get_display(Data *data)
{
    return (char *)gtk_frame_get_label(GTK_FRAME(data->f));
}

/* Converts double to string given number of digits of precision. */
static char *precise_num2str(double num, int precision)
{
    static char buffer[TOT_DIGITS + 1]; /* reusable static buffer */
    snprintf(buffer, sizeof(buffer), "%.*f", precision, num);
    return buffer;
}

/* Creates a string representation of the given number */
static char *num2str(double num, bool decimal, int decimals)
{
    if (decimal) return precise_num2str(num, decimals);
    if (num == 0) return precise_num2str(0, 0); 

    /* num can be displayed with 0-7 digits of precision */
    for (int i = 0; i <= MAX_PRECISION; i++) {

        /* round to i digits after decimal point */
        double factor = pow(10, i);
        int shifted = round(num * factor);
        double rounded = (double)shifted / factor;

        /* check whether num is close to rounded counterpart */
        if (fabs(num - rounded) < TOL) {
            return precise_num2str(rounded, i);
        }
    }

    int int_part = (int)num;
    /* number of digits before the decimal point */
    int int_digits = (int_part == 0) ? 0 : ceil(log10(abs(int_part)));

    /* handle large numbers or high precision */
    int precision = TOT_DIGITS - int_digits;
    if (precision < 0) precision = 0;
    return precise_num2str(num, precision);
}

/* Displays a given number on the calculator's screen. */
static void display_num(Data *data)
{
    printf("%f\n", data->num);
    char *num_displayed = num2str(data->num, data->decimal, data->decimals);
    display_str(data, num_displayed);
}

/* Handles numerical input into calculator */
static void entering(GtkWidget *widget, gpointer user_data)
{
    Data *data = (Data *)user_data;
    char *prev = get_display(data); /* previous display */
    char *button_label = (char *)gtk_button_get_label(GTK_BUTTON(widget));
    double entered_num = strtof(button_label, NULL);

    /* if previous display shows an operator */
    if (str_to_op(prev) != DEFAULT) {
        display_str(data, button_label);
        data->num = entered_num;
        return;
    }

    /* ignore leading zeros */
    if (strcmp(prev, "0") == 0 && entered_num == 0) {
        data->num = 0;
        return;
    }

    /* if previous display shows inf or nan */
    if (strcmp(prev, "inf") == 0 || strcmp(prev, "nan") == 0) {
        display_str(data, button_label);
        data->num = 0;
        data->result = 0;
    }

    /* handle decimal fraction input mode */
    if (data->decimal) {
        data->decimals++;
        double factor = pow(10, data->decimals);

        if (data->num < 0) { /* negative decimal */
            data->num -= (entered_num / factor);
        } else {
            data->num += (entered_num / factor);
        }
    }
    else { /* append entered digit to the displayed integer */
        data->num = data->num * 10 + entered_num;
    }

    display_num(data);
}

static void special_op(GtkWidget *widget, gpointer user_data)
{
    char *button_label = (char *)gtk_button_get_label(GTK_BUTTON(widget));
    special op = str_to_special(button_label);

    Data *data = (Data *)user_data;
    data->decimal = false; /* exit decimal fraction input mode */
    data->decimals = 0;

    char *prev_num = get_display(data);

    /* this button is ignored if an operation is being performed */
    if (str_to_op(prev_num) == DEFAULT) {
        data->num = un_op(data->num, op);
        display_num(data);
    }
}

/* Handles the (.) button */
static void point(GtkWidget *widget, gpointer user_data)
{
    Data *data = (Data *)user_data;

    /* ignore repeated decimal points */
    if (data->decimal) return;

    char *prev_num = get_display(data);

    /* if a binary operation is not being performed */
    if (str_to_op(prev_num) == DEFAULT) {
        strcat(prev_num, ".");
        data->decimal = true;
        display_str(data, prev_num);
    }
}

/* Handles binary operator inputs, as well as "=".
 * Note: Division by zero results in "inf" */
static void binary_op(GtkWidget *widget, gpointer user_data)
{
    char *button_label = (char *)gtk_button_get_label(GTK_BUTTON(widget));
    operator op = str_to_op(button_label);

    /* do nothing if no operation is being performed, i.e. if user enters
     * an expression with no binary operation, such as "3 =" */
    if (op == DEFAULT && strcmp(button_label, "=") != 0) {
        return;
    }

    Data *data = (Data *)user_data;
    data->decimal = false; /* exit decimal fraction input mode */
    data->decimals = 0;

    /* evaluate stored expression */
    data->result = bin_op(data->result, data->op, data->num);
    data->op = op;

    /* if "=" was entered, display result */
    if (op == DEFAULT) {
        data->num = data->result;
        display_num(data);
        data->result = 0;
        return;
    }

    display_str(data, op_to_str(op));
}

/* Clears and resets the calculator */
static void clear(GtkWidget *widget, gpointer user_data)
{
    Data *data = (Data *)user_data;
    
    data->decimal = false;
    data->decimals = 0;
    data->op = DEFAULT;
    data->result = 0;
    data->num = 0;

    display_str(data, "0");
}

/* Creates a 1×1 button at coordinate (x,y) and adds to grid */
static void new_button(GtkWidget *grid, char *label, void *callback,
                       gpointer data, int x, int y)
{
    GtkWidget *button = gtk_button_new_with_label(label);
    g_signal_connect(button, "clicked", G_CALLBACK(callback), data);
    gtk_grid_attach(GTK_GRID(grid), button, x, y, 1, 1);
}

/* Callback for the "activate" signal; creates calculator */
static void activate(GtkApplication *app, gpointer user_data)
{
    /* create a new window */
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Calculator");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);

    /* create grid to contain buttons and display screen */
    GtkWidget *grid = gtk_grid_new();
    gtk_window_set_child(GTK_WINDOW(window), grid);

    /* create display screen */
    GtkWidget *frame = gtk_frame_new("0");
    ((Data *)user_data)->f = frame;
    gtk_frame_set_label_align(GTK_FRAME(frame), 1);
    gtk_grid_attach(GTK_GRID(grid), frame, 0, 0, 4, 1);

    /* create buttons for numbers 0-9 */
    new_button(grid, "0", entering, user_data, 1, 7);
    char label[] = "1";
    for (int i = 6; i > 3; i--) {
        for (int j = 0; j < 3; j++) {
            new_button(grid, label, entering, user_data, j, i);
            label[0]++;
        }
    }

    /* create non-numerical buttons */
    new_button(grid, "\u221Ax", special_op, user_data, 0, 1);
    new_button(grid, "\u221Bx", special_op, user_data, 1, 1);
    new_button(grid, "x²", special_op, user_data, 2, 1);
    new_button(grid, "x³", special_op, user_data, 3, 1);
    new_button(grid, "x!", special_op, user_data, 0, 2);
    new_button(grid, "sin", special_op, user_data, 1, 2);
    new_button(grid, "cos", special_op, user_data, 2, 2);
    new_button(grid, "tan", special_op, user_data, 3, 2);
    new_button(grid, "C", clear, user_data, 0, 3);
    new_button(grid, "+/-", special_op, user_data, 1, 3);
    new_button(grid, "%", special_op, user_data, 2, 3);
    new_button(grid, ".", point, user_data, 2, 7);
    new_button(grid, "÷", binary_op, user_data, 3, 3);
    new_button(grid, "\u00D7", binary_op, user_data, 3, 4);
    new_button(grid, "-", binary_op, user_data, 3, 5);
    new_button(grid, "+", binary_op, user_data, 3, 6);
    new_button(grid, "=", binary_op, user_data, 3, 7);

    /* create "Off" button, which exits window */
    GtkWidget *button = gtk_button_new_with_label("Off");
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(gtk_window_destroy),
                                                                       window);

    gtk_grid_attach(GTK_GRID(grid), button, 0, 7, 1, 1);

    /* present the window */
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char *argv[])
{
    /* create instance of a Data object */
    Data *data = (Data *)malloc(sizeof(struct Data));
    data->op = DEFAULT;
    data->num = 0;
    data->result = 0;
    data->f = NULL;

    /* create new application instance */
    GtkApplication *app = gtk_application_new("com.example.GtkApplication",
                                              G_APPLICATION_DEFAULT_FLAGS);

    /* connect "activate" signal to the activate callback */
    g_signal_connect(app, "activate", G_CALLBACK(activate), data);

    /* run the application */
    int status = g_application_run(G_APPLICATION(app), argc, argv);

    /* clean up application resources */
    g_clear_object(&app);
    free(data);

    return status;
}