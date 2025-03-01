#ifndef TYPES_SSFTP
#define TYPES_SSFTP

// So that we don't have to write `unsigned char` everywhere
typedef unsigned char Byte;

/*
    Used for coloring the output text
    These are ANSI escape codes, see https://en.wikipedia.org/wiki/ANSI_escape_code

    For example, printing RED_START followed by some text will print the text in red
    Printing RESET_COLOR will reset the color to default
*/
#define RED_START "\033[1;31m"
#define GREEN_START "\033[1;32m"
#define YELLOW_START "\033[1;33m"
#define RESET_COLOR "\033[0m"

#endif