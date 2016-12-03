#pragma once

// debug options to test 
#define DEBUG_VALUES_ON false
#define DEBUG_FIELDS_ON false

// pebble color
#ifdef PBL_COLOR
#define BG_COLOUR GColorYellow
#else
#define BG_COLOUR GColorWhite
#endif

#ifdef PBL_PLATFORM_EMERY 

#define FONT_DIGITS_XXL fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS)
#define FONT_DIGITS_LRG fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS)
#define FONT_DIGITS_MED fonts_get_system_font(FONT_KEY_LECO_38_BOLD_NUMBERS)
#define FONT_DIGITS_SML fonts_get_system_font(FONT_KEY_LECO_32_BOLD_NUMBERS)
#define FONT_DIGITS_TNY LECO_REG_17
#define MULTIPLIER (6/5)

#else

#define FONT_DIGITS_XXL fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS)
#define FONT_DIGITS_LRG fonts_get_system_font(FONT_KEY_LECO_38_BOLD_NUMBERS)
#define FONT_DIGITS_MED fonts_get_system_font(FONT_KEY_LECO_32_BOLD_NUMBERS)
#define FONT_DIGITS_SML fonts_get_system_font(FONT_KEY_LECO_26_BOLD_NUMBERS_AM_PM)
#define FONT_DIGITS_TNY LECO_REG_17
#define MULTIPLIER 1

#endif

