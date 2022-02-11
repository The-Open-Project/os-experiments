#include "./stdio.h"
#include "./stdarg.h"
#include "./stdbool.h"

extern int print_char(char c);
extern int print_string(const char *s);

struct _FILE
{
    bool placeholder;
};

int putc(int c, FILE *stream)
{
    print_char(c);
    return c;
}

int putchar(int c)
{
    return putc(c, NULL);
}

int puts(const char *s)
{
    print_string(s);
    print_char('\n');
    return 0;
}

int printf(const char *format, ...)
{
    // TODO: implement format string

    return fprintf(stdout, format, NULL);
}

int fprintf(FILE *stream, const char *format, ...)
{
    // ignore stream
    va_list args;
    va_start(args, format);

    int result = vfprintf(stream, format, args);

    va_end(args);
    return result;
}

int vprintf(const char *format, va_list args)
{
    return vfprintf(stdout, format, args);
}

int vfprintf(FILE *stream, const char *format, va_list args)
{
    // ignore args list for a moment and just print format string
    return print_string(format);
}