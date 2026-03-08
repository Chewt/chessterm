#ifndef CONFIG_H
#define CONFIG_H

#define DEFAULT_LIGHT_COLOR 249
#define DEFAULT_DARK_COLOR 238

#define CONFIG_STR_LIGHT_COLOR "board_color_light="
#define CONFIG_STR_DARK_COLOR "board_color_dark="

typedef struct {
    int board_color_light;
    int board_color_dark;
} Config;

Config read_config_file(char* config_path);

#endif // CONFIG_H
