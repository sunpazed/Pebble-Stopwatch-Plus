#include <pebble.h>
#include "fonts.h"

void init_fonts() {
  LECO_REG_17 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_LECO_REG_17));
  LECO_REG_32 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_LECO_REG_32));
  LECO_REG_13 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_LECO_REG_13));
  LECO_LIGHT_16 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_LECO_LIGHT_16));
#ifdef PBL_PLATFORM_EMERY 
  LECO_REG_54 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_LECO_REG_54));
#endif
}