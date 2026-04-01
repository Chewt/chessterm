#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "io.h"

// Get an integer config line value
int get_int_option(char* option, char* file_content) {
    // Copy file_contents and search through copy
    char* content_copy = malloc(strlen(file_content) + 1);
    strcpy(content_copy, file_content);
    char* save_ptr_lines = NULL;
    char* line = strtok_r(content_copy, "\n", &save_ptr_lines);
    while (line != NULL) {
        if (!strncmp(option, line, strlen(option))) {
            int value = atoi(line + strlen(option)); 
            free(content_copy);
            return value;
        }
        line = strtok_r(NULL, "\n", &save_ptr_lines);
    }
    free(content_copy);
    return -1;
}

// Get an integer config line value
char* get_str_option(char* option, char* file_content) {
    // Copy file_contents and search through copy
    char* content_copy = malloc(strlen(file_content) + 1);
    strcpy(content_copy, file_content);
    char* save_ptr_lines = NULL;
    // TODO: This will not find lines that do not end in newlines, for example
    // if a config has only one line in it and does not end in a newline.
    // Technically POSIX standard specifies that text files must always end in
    // newline characters, but ideally I should account for the case where there
    // isn't one anyway.
    char* line = strtok_r(content_copy, "\n", &save_ptr_lines);
    while (line != NULL) {
        if (!strncmp(option, line, strlen(option))) {
            size_t value_length = strlen(line + strlen(option));
            char* value = malloc(value_length + 1);
            strcpy(value, line + (strlen(option)));
            free(content_copy);
            return value;
        }
        line = strtok_r(NULL, "\n", &save_ptr_lines);
    }
    free(content_copy);
    return NULL;
}

Config default_config() {
    Config config;
    config.color_mode = DEFAULT_COLOR_MODE;
    config.board_color_light = DEFAULT_LIGHT_COLOR_256;
    config.board_color_dark = DEFAULT_DARK_COLOR_256;
    config.piece_art = ASCII;
    return config;
}

char* locate_config() {
    char* homedir = getenv("HOME");
    if (homedir == NULL)
        return NULL;
    // Try to access config file
    char* config_path = malloc(256);
    snprintf(config_path, 256, "%s/.config/chessterm/config", homedir);
    FILE* f = fopen(config_path, "r");
    if (f == NULL) {
        free(config_path);
        return NULL;
    }
    fclose(f);
    return config_path;
}

Config read_config_file(char* config_path) {
    // Try to locate config path if one is not passed
    int config_path_needs_freed = 0;
    if (config_path == NULL) {
        config_path_needs_freed = 1;
        config_path = locate_config();
    }

    // Get contents of config file
    FILE* f = fopen(config_path, "r");
    if (config_path != NULL && config_path_needs_freed) free(config_path);
    if (f == NULL) {
        return default_config();
    }
    fseek(f, 0, SEEK_END);
    int file_size = ftell(f);
    if (file_size == -1) {
        return default_config();
    }
    rewind(f);
    char* file_content = malloc(file_size + 1);
    size_t bytes_read = fread(file_content, 1, file_size, f);
    file_content[bytes_read] = '\0';
    fclose(f);

    // Read config options
    Config config = default_config();
    int light_color = get_int_option(CONFIG_STR_LIGHT_COLOR, file_content);
    int dark_color = get_int_option(CONFIG_STR_DARK_COLOR, file_content);
    int color_mode = get_int_option(CONFIG_STR_COLOR_MODE, file_content);
    char* art_style = get_str_option(CONFIG_STR_PIECE_ART, file_content);

    // Apply valid config options
    if (color_mode != -1) config.color_mode = color_mode;
    if (light_color != -1) {
        config.board_color_light = light_color;
    } else if (color_mode == 16) {
        config.board_color_light = DEFAULT_LIGHT_COLOR_16;
    }
    if (dark_color != -1) {
        config.board_color_dark = dark_color;
    } else if (color_mode == 16) {
        config.board_color_dark = DEFAULT_DARK_COLOR_16;
    }
    if (art_style != NULL) {
        fprintf(stderr, "Found option: %s\n", art_style);
        if (!strncmp(art_style, PIECE_ART_ASCII, strlen(PIECE_ART_ASCII)))
            config.piece_art = ASCII;
        else if (!strncmp(art_style, PIECE_ART_UNICODE, strlen(PIECE_ART_UNICODE)))
            config.piece_art = UNICODE;
        else if (!strncmp(art_style, PIECE_ART_BRAILLE, strlen(PIECE_ART_BRAILLE)))
            config.piece_art = BRAILLE;
        free(art_style);
    }

    free(file_content);
    return config;
}
