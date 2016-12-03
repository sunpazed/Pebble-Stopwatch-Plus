#include <pebble.h>
#include "icons.h"

void load_actionbar_images() {
  icon_play = gbitmap_create_with_resource(RESOURCE_ID_ICON_PLAY);
  icon_split = gbitmap_create_with_resource(RESOURCE_ID_ICON_SPLIT);
  icon_pause = gbitmap_create_with_resource(RESOURCE_ID_ICON_PAUSE);
  icon_fields = gbitmap_create_with_resource(RESOURCE_ID_ICON_FIELDS);
  icon_trash = gbitmap_create_with_resource(RESOURCE_ID_ICON_TRASH);
}

void load_field_images() {
  icon_time = gbitmap_create_with_resource(RESOURCE_ID_ICON_TIME);
  icon_pace = gbitmap_create_with_resource(RESOURCE_ID_ICON_PACE);
  icon_hr   = gbitmap_create_with_resource(RESOURCE_ID_ICON_HR);
  icon_running = gbitmap_create_with_resource(RESOURCE_ID_ICON_RUNNING);
}