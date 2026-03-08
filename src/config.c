#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

// Get an integer config line value
int get_int_option(char* option, char* file_content) {
    // Copy file_contents and search through copy
    char* content_copy = malloc(strlen(file_content) + 1);
    strcpy(content_copy, file_content);
    fprintf(stderr, "Finding %s...\n", option);
    char* save_ptr_lines = NULL;
    char* line = strtok_r(content_copy, "\n", &save_ptr_lines);
    while (line != NULL) {
        fprintf(stderr, "Checking line: \"%s\"\n", line);
        if (!strncmp(option, line, strlen(option))) {
            int value = atoi(line + strlen(option)); 
            free(content_copy);
            return value;
        }
        line = strtok_r(NULL, "\n", &save_ptr_lines);
    }
    fprintf(stderr, "Could not find option: %s\n", option);
    free(content_copy);
    return -1;
}

Config default_config() {
    return (Config){ DEFAULT_LIGHT_COLOR, DEFAULT_DARK_COLOR };
}

Config read_config_file(char* config_path) {
    // Get contents of config file
    FILE* f = fopen(config_path, "r");
    if (f == NULL) {
        fprintf(stderr, "Could not find file: %s\n", config_path);
        return default_config();
    }
    fseek(f, 0, SEEK_END);
    size_t file_size = ftell(f);
    if (file_size == -1) {
        fprintf(stderr, "Error seeking file\n");
        return default_config();
    }
    rewind(f);
    char* file_content = malloc(file_size + 1);
    size_t bytes_read = fread(file_content, 1, file_size, f);
    file_content[bytes_read] = '\0';
    fclose(f);

    fprintf(stderr, "File Size: %zu\nFile Content:\n%s\n", file_size, file_content);

    // Read config options
    Config config = default_config();
    int light_color = get_int_option(CONFIG_STR_LIGHT_COLOR, file_content);
    int dark_color = get_int_option(CONFIG_STR_DARK_COLOR, file_content);

    // Apply valid config options
    if (light_color != -1) config.board_color_light = light_color;
    if (dark_color != -1) config.board_color_dark = dark_color;

    free(file_content);
    return config;
}
