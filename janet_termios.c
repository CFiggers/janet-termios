/*** includes ***/

#include <janet.h>
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h> /* defines STDIN_FILENO, system calls,etc */

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#endif

/*** defines ***/

#ifndef _WIN32
struct world_atom
{
    struct termios orig_termios;
};

typedef struct world_atom world_atom;

world_atom world;
#endif

enum editorKey
{
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    PAGE_UP,
    PAGE_DOWN,
    HOME_KEY,
    END_KEY,
    DEL_KEY
};

/*** terminal ***/

void disable_raw_mode()
{
#ifndef _WIN32
    if (-1 == tcsetattr(STDIN_FILENO, TCSAFLUSH, &world.orig_termios))
        fprintf(stderr, "tcsetattr");
#endif
};

void enable_raw_mode()
{
#ifndef _WIN32
    if (-1 == tcgetattr(STDIN_FILENO, &world.orig_termios))
        fprintf(stderr, "tcgetattr");
    atexit(disable_raw_mode);

    struct termios raw = world.orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= (OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1; // Every 10th of second redraw / skip the read (stop block).

    if (-1 == tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw))
        fprintf(stderr, "tcsetattr");
#endif
};

int getCursorPosition(int *rows, int *cols)
{
    char buf[32];
    unsigned int i = 0;

    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
        return -1;

    while (i < sizeof(buf) - 1)
    {
        if (read(STDIN_FILENO, &buf[i], 1) != 1)
            break;
        if (buf[i] == 'R')
            break;
    }

    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[')
        return -1;
    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2)
        return -1;
    return 0;
};

int read_key()
{
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
    {
        if (nread == -1 && errno != EAGAIN)
            fprintf(stderr, "read");
    }

    if (c == '\x1b')
    {
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) != 1)
            return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1)
            return '\x1b';
        if (seq[0] == '[')
        {
            if (seq[1] >= '0' && seq[1] <= '9')
            {
                if (read(STDIN_FILENO, &seq[2], 1) != 1)
                    return '\x1b';
                if (seq[2] == '~')
                {
                    switch (seq[1])
                    {
                    case '1':
                        return HOME_KEY;
                    case '3':
                        return DEL_KEY;
                    case '4':
                        return END_KEY;
                    case '5':
                        return PAGE_UP;
                    case '6':
                        return PAGE_DOWN;
                    case '7':
                        return HOME_KEY;
                    case '8':
                        return END_KEY;
                    }
                }
            }
            else
            {
                switch (seq[1])
                {
                case 'A':
                    return ARROW_UP;
                case 'B':
                    return ARROW_DOWN;
                case 'C':
                    return ARROW_RIGHT;
                case 'D':
                    return ARROW_LEFT;
                case 'H':
                    return HOME_KEY;
                case 'F':
                    return END_KEY;
                }
            }
        }
        else if (seq[0] == 'O')
        {
            switch (seq[1])
            {
            case 'H':
                return HOME_KEY;
            case 'F':
                return END_KEY;
            }
        }

        return '\x1b';
    }
    else
    {
        return c;
    }
}

int get_window_size(int *rows, int *cols)
{
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
            return -1;
        return getCursorPosition(rows, cols);
    }
    else
    {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
};

/*** janet functions ***/

static Janet read_key_wrapped(int32_t argc, Janet *argv)
{
    (void)argv;
    janet_fixarity(argc, 0);

    return janet_wrap_number(read_key());
}

static Janet enable_raw_mode_wrapped(int32_t argc, Janet *argv)
{
    (void)argv;
    janet_fixarity(argc, 0);

    enable_raw_mode();

    return janet_wrap_nil();
}

static Janet disable_raw_mode_wrapped(int32_t argc, Janet *argv)
{
    (void)argv;
    janet_fixarity(argc, 0);

    disable_raw_mode();

    return janet_wrap_nil();
}

static Janet stream_char_wrapped(int32_t argc, Janet *argv)
{
    int nread;
    char c;

#ifdef _WIN32
    c = _getch();
#else
    enable_raw_mode();

    while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
    {
        if (nread == -1 && errno != EAGAIN)
            fprintf(stderr, "read");
    }

    disable_raw_mode();
#endif

    return janet_wrap_number(c);
};

static Janet await_char_wrapped(int32_t argc, Janet *argv)
{
    int nread;
    char c;

#ifdef _WIN32
    c = fgetc(stdin);
    fflush(stdin);
#else
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
    {
        if (nread == -1 && errno != EAGAIN)
            fprintf(stderr, "read");
    }
#endif

    return janet_wrap_number(c);
};

static Janet get_window_size_wrapped(int32_t argc, Janet *argv)
{
    (void)argv;
    janet_fixarity(argc, 0);

    int rows;
    int cols;
    get_window_size(&rows, &cols);

    JanetTable *window_size = janet_table(4);
    janet_table_put(window_size, janet_ckeywordv("rows"), janet_wrap_integer(rows));
    janet_table_put(window_size, janet_ckeywordv("cols"), janet_wrap_integer(cols));
    return janet_wrap_table(window_size);
};

static const JanetReg janet_termio_cfuns[] = {
    {"stream-char", stream_char_wrapped,
     "(stream-char)\n\n"
     "Blocks waiting for a char to be typed into the terminal. "
     "Expects raw mode to be disabled."},
    {"await-char", await_char_wrapped,
     "(await-char)\n\n"
     "Blocks waiting for a char to be typed into the terminal and "
     "then sent with Enter. Expects raw mode to be disabled."},
    {"read-key", read_key_wrapped,
     "(read-key)\n\n"
     "Reads a char from input. Expects raw mode to be enabled."},
    {"get-window-size", get_window_size_wrapped,
     "(get-window-size)\n\n"
     "Returns an array containing the dimensions size of the "
     "current window. Access using `:rows` and `:cols`."},
    {"enable-raw-mode", enable_raw_mode_wrapped,
     "(enable-raw-mode)\n\n"
     "Enables raw mode in the active terminal."},
    {"disable-raw-mode", disable_raw_mode_wrapped,
     "(disable-raw-mode)\n\n"
     "Disables raw mode in the active terminal."},
    {NULL, NULL, NULL}};

JANET_MODULE_ENTRY(JanetTable *env)
{
    janet_cfuns(env, "janet-termios", janet_termio_cfuns);
};