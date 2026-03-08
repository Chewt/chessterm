#ifndef CONFIG_H
#define CONFIG_H

#define DEFAULT_COLOR_MODE 256

#define DEFAULT_LIGHT_COLOR_256 179
#define DEFAULT_DARK_COLOR_256 58
#define DEFAULT_LIGHT_COLOR_16 44
#define DEFAULT_DARK_COLOR_16 45

#define CONFIG_STR_LIGHT_COLOR "board_color_light="
#define CONFIG_STR_DARK_COLOR "board_color_dark="
#define CONFIG_STR_COLOR_MODE "color_mode="
#define CONFIG_STR_PIECE_ART "piece_art="

#define PIECE_ART_ASCII "ascii"
#define PIECE_ART_UNICODE "unicode"

typedef struct {
    int color_mode;
    int board_color_light;
    int board_color_dark;
    int piece_art;
} Config;

Config read_config_file(char* config_path);
Config default_config();

#endif // CONFIG_H
