#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef linux
#include <termios.h>
#include <unistd.h>
#endif

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <bf_source_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        fprintf(stderr, "Could not open file %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    fseek(fp, 0, SEEK_END);
    const long size = ftell(fp);
    rewind(fp);

    char *buf = calloc(1, size);
    if (!buf) {
        fprintf(stderr, "Could not allocate memory for file %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    if (fread(buf, size, 1, fp) != 1) {
        fprintf(stderr, "Could not read file %s into buffer\n", argv[1]);
        return EXIT_FAILURE;
    }
    fclose(fp);

#ifdef linux
    // Change terminal output to not be cooked/line buffered
    struct termios old, new;
    tcgetattr(STDIN_FILENO, &new);
    old = new;  // Save terminal settings

    new.c_lflag &= ~(ICANON); // Turn off canonical mode ("line buffering")
    tcsetattr(STDIN_FILENO, TCSANOW, &new);
#endif

    uint8_t data[30000], *data_ptr = data;
    uint8_t bracket_count;

    for (char *cmd_ptr = buf; (cmd_ptr - buf) < size; cmd_ptr++) {
        switch(*cmd_ptr) {
            case '<': data_ptr--;            break; // Move data pointer to the left
            case '>': data_ptr++;            break; // Move data pointer to the right
            case '+': (*data_ptr)++;         break; // Increment cell at the data pointer
            case '-': (*data_ptr)--;         break; // Decrement cell at the data pointer
            case ',': *data_ptr = getchar(); break; // Input byte to data pointer

            case '.': 
                putchar(*data_ptr);
                fflush(stdout); // Show updates immediately
                break; // Output byte at data pointer

            case '[':
                // If current byte at data pointer is 0, go forward to matching bracket, and move past it
                if (*data_ptr == 0) {
                    bracket_count = 1;
                    while (bracket_count > 0) {
                        cmd_ptr++;
                        if (*cmd_ptr == '[') bracket_count++;
                        if (*cmd_ptr == ']') bracket_count--;
                    }
                }
                break;

            case ']':
                // If current byte at data pointer is not 0, go back to previous matching bracket,
                //  and move past it
                if (*data_ptr != 0) {
                    bracket_count = 1;
                    while (bracket_count > 0) {
                        cmd_ptr--;
                        if (*cmd_ptr == '[') bracket_count--;
                        if (*cmd_ptr == ']') bracket_count++;
                    }
                }
                break;
        }
    }

#ifdef linux
    // Restore terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &old);
#endif

    putchar('\n');  // Add ending newline for shell output
    return EXIT_SUCCESS;
}

