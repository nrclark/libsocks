#include "config.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "scan.h"

static unsigned int find_start(const char *data)
{
    unsigned int x;

    for (x = 0; data[x] != '\x00'; x++) {
        if (isspace(data[x]) == 0) {
            break;
        }
    }

    return x;
}

static unsigned int trim_right(char *data)
{
    unsigned int x;

    if (data[0] == '\x00') {
        return 0;
    }

    for (x = 0; data[x] != '\x00'; x++) {}
    for (x = x - 1; isspace(data[x]); x--) {
        data[x] = '\x00';
    }

    return x + 1;
}

static void shift_left(char *data)
{
    for (unsigned int x = 1; data[x - 1] != '\x00'; x++) {
        data[x - 1] = data[x];
    }
}

/** Returns 0 if a line holds a section header that matches 'target', and -1
 * otherwise. */
static int match_section(const char *line, unsigned int line_length,
                         const char *target)
{
    char section[line_length + 1];
    int result;
    int length;

    if ((line == NULL) || (target == NULL)) {
        return -1;
    }

    result = sscanf(line, "[%s", section);

    if (result < 1) {
        return -1;
    }

    length = strlen(section);
    if (section[length - 1] == ']') {
        section[length - 1] = '\x00';
    }

    result = strncmp(section, target, (size_t) length + 1);
    return result;
}

/** Returns 0 if a line holds a key-value pair that matches 'key'. The
 * resulting value is stored in a newly-allocated buffer for *value. If
 * no match is found, -1 is returned and *value is unchanged. */
static int match_key(const char *line, unsigned int line_length,
                     const char *key, char **value)
{
    char keybuf[line_length + 1];
    char valbuf[line_length + 1];
    char quotes[2] = {'"', '\''};
    int result;
    unsigned int length;

    keybuf[line_length] = '\x00';
    valbuf[line_length] = '\x00';

    line += find_start(line);
    result = sscanf(line, "%[^\t\n =]%*[\t =]%[^\n]", keybuf, valbuf);

    if (result < 2) {
        return -1;
    }

    if (strcmp(keybuf, key) != 0) {
        return -1;
    }

    length = trim_right(valbuf);

    for (unsigned int x = 0; x < sizeof(quotes); x++) {
        if ((valbuf[0] == quotes[x]) && (valbuf[result - 1] == quotes[x])) {
            shift_left(valbuf);
            length--;
        }
    }

    *value = malloc(length + 1);
    if (*value == NULL) {
        perror("allocation failure");
        return -2;
    }

    memcpy(*value, valbuf, length + 1);
    return 0;
}

int find_key(const char *filename, const char *section, const char *key,
             char **value)
{
    FILE *infile;
    char *line = NULL;
    char *trimmed;
    size_t len = 0;
    ssize_t result = 0;
    char section_matched = 1;

    if ((key == NULL) || (value == NULL) || (filename == NULL)) {
        return -1;
    }

    infile = fopen(filename, "r");

    if (infile == NULL) {
        perror("couldn't open file for writing");
        return -1;
    }

    while (1) {
        result = getline(&line, &len, infile);

        if (result == -1) {
            free(line);
            break;
        }

        if (line == NULL) {
            perror("null line pointer");
            return -1;
        }

        trimmed = line + find_start(line);
        len = trim_right(trimmed);

        switch (trimmed[0]) {
            case '\x00':
            case '#':
                break;

            case '[':
                section_matched = 0;
                if (match_section(trimmed, len, section) == 0) {
                    section_matched = 1;
                }
                break;

            default:
                if (section_matched == 0) {
                    break;
                }

                result = match_key(trimmed, len, key, value);
                if (result == 0) {
                    fclose(infile);
                    free(line);
                    return 0;
                }

                break;
        }
        free(line);
        line = NULL;
    }

    fclose(infile);
    return -1;
}

static void run_test(const char *section, const char *key)
{
    char *data = NULL;

    int result = find_key("test.conf", section, key, &data);

    if (result != 0) {
        printf("Result: %d, couldn't match key: [%s]\n", result, key);
        return;
    }

    printf("Result: %d, value: [%s] = [%s]\n", result, key, data);
    free(data);
}
int main(void)
{
    //char buffer[512] = {0};
    //char *data = buffer;
    run_test(NULL, "shit");
    run_test("global", "data");
    run_test(NULL, "data");
    //int result = find_key("test.conf", "global", "data", &data);
    //printf("Result: %d, value: [%s] = [%s]\n", result, "data", data);
    return 0;
}
