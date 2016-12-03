#include <pebble.h>
#include "main.h"
#include "utils.h"
#include "fonts.h"
#include "icons.h"


#define SYSTEM_METRIC 1
#define SYSTEM_IMPERIAL 2
int sys_unit_type = 0;


// stopwatch state struct
struct StopwatchState {
	double elapsed_time;
	double start_time;
	double resume_time;
  double temp_time;
  double split_time;
  double last_split;
  double pause_time;
  int dist_initial;
  int dist_latest;
  int dist_previous;
  int dist_resume;
  int dist_pause;
  float pace_previous;
  int state_is;
  int mode_is;
  int stop_type;
} __attribute__((__packed__));
// key for state
#define PERSIST_STATE 1

//splits
#define MAX_LAPS 3
#define LAP_STRING_LENGTH 20
static char lap_text[MAX_LAPS][LAP_STRING_LENGTH];

// global state definition for stopwatch
#define STATE_STOP 0
#define STATE_RUN 1
#define STATE_PAUSE 2
int state_is = STATE_STOP;

// mode definition for fields
#define MODE_DIST 0
#define MODE_TIME 1
#define MODE_HR 2
#define MODE_PACE 3
int mode_is = MODE_DIST;

// window + update interval for event loop (fps)
static Window *s_window;
// main window update interval for event loop
#define UPDATE_INTERVAL 40

// tick counters used to update field data at intervals
int tick_count_pace = 0;
int tick_count_dist = 0;
int tick_count_hr = 0;
#define TICK_INTERVAL 10
#define TICK_INTERVAL_PACE 60


// text + image layers
static TextLayer *layer_big_field;
static TextLayer *layer_small_field;
static TextLayer *layer_split_field;
static Layer *gtext_layer;
static Layer *gsplit_layer;
static Layer *status_bar;
static BitmapLayer *field_bitmap_layer;

// debugging layers
static TextLayer *layer_debug_field;
static char text_debug_placeholder[80];

// defines the layout of the digits
int stop_type;
int stop_type = 1;

// health data
bool pebble_has_health_available = false;
bool pebble_has_hr_available = false;
HealthValue field_hr_value;
static char hr_placeholder[12];

// defines the field type
//int field_type = 1;

// keeping track of time
static double elapsed_time = 0;
static bool started = false;
static AppTimer* update_timer = NULL;
static double start_time = 0;
static double resume_time = 0;
static double split_time = 0;
static double last_split = 0;
static double temp_time = 0;
static double pause_time = 0;
static char big_sec_time[] = "00:00:00";
static char sub_sec_time[] = "00";
static char text_split_time[120];
static char AMorPM[4]; 

// store time in slots
int hundreds = 0;
int tenths = 0;
int seconds = 0;
int minutes = 0;
int hours = 0;

// display digits
static char digit_one[]   = "0";
static char digit_two[]   = "0";
static char digit_three[] = "0";
static char digit_four[]  = "0";
static char digit_five[]  = "0";
static char digit_six[]   = "0";

// keeping track of distance
static int dist_initial = 0;
static int dist_latest = 0;
static int dist_previous = 0;
static int dist_resume = 0;
static int dist_pause = 0;
static float pace_previous = 0;

// defining the acton bar
ActionBarLayer *action_bar;

// define the individual fields to be displayed
// fields; time
static char time_buffer[8];
// fields; distance
static char dist_value_buffer[32];
static char dist_field_buffer[40];
// fields; hr
static char hr_field_buffer[8];
// fields; pace
static char pace_field_buffer[] = "PaCE / KM";
static char pace_value_buffer[] = " --:--";

// handlers for topwatch updates
void handle_timer(void* data);
void update_stopwatch();
void update_actionbar();
void update_fields();
void next_field_type();


  
// ----------------------------------------------------------------------------


// callback to render individual digits for the main timer
static void update_gtext_callback(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_frame(layer);

  // white background for non-colour devices, or yellow
  graphics_context_set_fill_color(ctx, BG_COLOUR);

  // text is black
  graphics_context_set_text_color(ctx, GColorBlack);

  // fonts reminder ...
  // FONT_KEY_LECO_20_BOLD_NUMBERS 
  // FONT_KEY_LECO_26_BOLD_NUMBERS_AM_PM 
  // FONT_KEY_LECO_28_LIGHT_NUMBERS 
  // FONT_KEY_LECO_32_BOLD_NUMBERS 
  // FONT_KEY_LECO_36_BOLD_NUMBERS 
  // FONT_KEY_LECO_38_BOLD_NUMBERS 
  // FONT_KEY_LECO_42_NUMBERS 

  if (stop_type == 1) {
  // layout == 4 large digits

  snprintf(digit_one, 8, "%01d", (seconds / 10) % 10);  
  snprintf(digit_two, 8, "%01d", seconds % 10);  

  snprintf(digit_three, 8, "%01d", (hundreds / 10) % 10);  
  snprintf(digit_four, 8, "%01d", hundreds % 10);  

  static int t_width = 26 * MULTIPLIER;
  static int t_height = 40 * MULTIPLIER;
  static int t_spacer = 12 * MULTIPLIER;
  static int t_xoffset = 3 * MULTIPLIER;
  static int t_yoffset = -8 * MULTIPLIER;
  static int kerning = -2 * MULTIPLIER;
    
  graphics_fill_rect(ctx, GRect(0, 0, 112 * MULTIPLIER, t_height * MULTIPLIER), 0, GCornerNone);
  graphics_draw_text(ctx, digit_one, FONT_DIGITS_XXL, GRect(t_xoffset, t_yoffset, t_width, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, digit_two, FONT_DIGITS_XXL, GRect(t_xoffset+t_width+(kerning), t_yoffset, t_width, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, ".", FONT_DIGITS_XXL, GRect(t_xoffset+(t_width*2)+(kerning*2), t_yoffset, t_spacer, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, digit_three, FONT_DIGITS_XXL, GRect(t_xoffset+(t_width*2)+t_spacer+(kerning*3), t_yoffset, t_width, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, digit_four, FONT_DIGITS_XXL, GRect(t_xoffset+(t_width*3)+t_spacer+(kerning*4), t_yoffset, t_width, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  }  
  
  if (stop_type == 2) {
  // layout == 3 large digits, 2 small

  snprintf(digit_one, 8, "%01d", minutes % 10);  
  snprintf(digit_two, 8, "%01d", (seconds / 10) % 10);  
  snprintf(digit_three, 8, "%01d", seconds % 10);  

  snprintf(digit_four, 8, "%01d", (hundreds / 10) % 10);  
  snprintf(digit_five, 8, "%01d", hundreds % 10);
  
  static int t_width = 25;
  static int t_width_sm = 12;
  static int t_height = 36;
  static int t_spacer = 11;
  static int t_xoffset = 7;
  static int t_yoffset = -4;
  static int t_yoffset_sm = 2;
  static int kerning = -2;
    
  graphics_fill_rect(ctx, GRect(0, 0, 112, t_height), 0, GCornerNone);
  graphics_draw_text(ctx, digit_one, FONT_DIGITS_LRG, GRect(t_xoffset, t_yoffset, t_width, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, ":", FONT_DIGITS_LRG, GRect(t_xoffset+t_width+(kerning), t_yoffset, t_spacer, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, digit_two, FONT_DIGITS_LRG, GRect(t_xoffset+(t_width)+t_spacer+(kerning*2), t_yoffset, t_width, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, digit_three, FONT_DIGITS_LRG, GRect(t_xoffset+(t_width*2)+t_spacer+(kerning*3), t_yoffset, t_width, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, digit_four, FONT_DIGITS_TNY, GRect(t_xoffset+(t_width*3)+t_spacer+(-6), t_yoffset_sm, t_width_sm, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, digit_five, FONT_DIGITS_TNY, GRect(t_xoffset+(t_width*3)+t_spacer+(t_width_sm)+(-6), t_yoffset_sm, t_width_sm, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  }
  
  if (stop_type == 3) {
  
  // layout == 4 medium digits, 2 small
  snprintf(digit_one, 8, "%01d", (minutes / 10) % 10);  
  snprintf(digit_two, 8, "%01d", minutes % 10);  
  snprintf(digit_three, 8, "%01d", (seconds / 10) % 10);  
  snprintf(digit_four, 8, "%01d", seconds % 10);  

  snprintf(digit_five, 8, "%01d", (hundreds / 10) % 10);  
  snprintf(digit_six, 8, "%01d", hundreds % 10);
  
  static int t_width = 21;
  static int t_width_sm = 12;
  static int kerning = -2;
  static int t_height = 32;
  static int t_spacer = 10;
  static int t_xoffset = 3;
  static int t_yoffset = 1;
  static int t_yoffset_sm = 6;

  graphics_fill_rect(ctx, GRect(0, 0, 112, t_height), 0, GCornerNone);
  graphics_draw_text(ctx, digit_one, FONT_DIGITS_MED, GRect(t_xoffset, t_yoffset, t_width, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, digit_two, FONT_DIGITS_MED, GRect(t_xoffset+t_width+(kerning*1), t_yoffset, t_width, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, ":", FONT_DIGITS_MED, GRect(t_xoffset+(t_width*2)+(kerning*2), t_yoffset, t_spacer, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, digit_three, FONT_DIGITS_MED, GRect(t_xoffset+(t_width*2)+t_spacer+(kerning*3), t_yoffset, t_width, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, digit_four, FONT_DIGITS_MED, GRect(t_xoffset+(t_width*3)+t_spacer+(kerning*4), t_yoffset, t_width, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, digit_five, FONT_DIGITS_TNY, GRect(t_xoffset+(t_width*4)+t_spacer+(kerning*5)+1, t_yoffset_sm, t_width_sm, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, digit_six, FONT_DIGITS_TNY, GRect(t_xoffset+(t_width*4)+t_spacer+(t_width_sm)+(kerning*5)+1, t_yoffset_sm, t_width_sm, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  }

  if (stop_type == 4) {
  
  // layout == 6 medium digits
  snprintf(digit_one, 8, "%01d", (hours / 10) % 10);  
  snprintf(digit_two, 8, "%01d", hours % 10);  
  snprintf(digit_three, 8, "%01d", (minutes / 10) % 10);  
  snprintf(digit_four, 8, "%01d", minutes % 10);  

  snprintf(digit_five, 8, "%01d", (seconds / 10) % 10);  
  snprintf(digit_six, 8, "%01d", seconds % 10);
  
  static int t_width = 17 * MULTIPLIER;
  static int t_width_sm = 12 * MULTIPLIER;
  static int kerning = -2 * MULTIPLIER;
  static int t_height = 26 * MULTIPLIER;
  static int t_spacer = 10 * MULTIPLIER;
  static int t_xoffset = 3 * MULTIPLIER;
  static int t_yoffset = 6 * MULTIPLIER;

  graphics_fill_rect(ctx, GRect(0, 0, 112, t_height), 0, GCornerNone);
  graphics_draw_text(ctx, digit_one, FONT_DIGITS_SML,   GRect(t_xoffset, t_yoffset, t_width, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, digit_two, FONT_DIGITS_SML,   GRect(t_xoffset+t_width+(kerning*1), t_yoffset, t_width, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, ":", FONT_DIGITS_SML,         GRect(t_xoffset+(t_width*2)+(kerning*2), t_yoffset, t_spacer, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, digit_three, FONT_DIGITS_SML, GRect(t_xoffset+(t_width*2)+t_spacer+(kerning*3), t_yoffset, t_width, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, digit_four, FONT_DIGITS_SML,  GRect(t_xoffset+(t_width*3)+t_spacer+(kerning*4), t_yoffset, t_width, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, ":", FONT_DIGITS_SML,         GRect(t_xoffset+(t_width*4)+(t_spacer)+(kerning*5), t_yoffset, t_spacer, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, digit_five, FONT_DIGITS_SML,  GRect(t_xoffset+(t_width*4)+(t_spacer*2)+(kerning*6), t_yoffset, t_width, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, digit_six, FONT_DIGITS_SML,   GRect(t_xoffset+(t_width*5)+(t_spacer*2)+(kerning*7), t_yoffset, t_width, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  }  
}

// -- render fields + digits --------------------------------------------------------

void render_fields() {

      #if defined(PBL_RECT)
        static int x_offset = 0;
        static int y_offset = 0;
      #elif defined(PBL_ROUND)
        static int x_offset = 26;
        static int y_offset = 9;
      #endif
  
      // big seconds
      GRect layer_small_bounds = GRect(32+x_offset,38+y_offset,75,20);
      layer_small_field = text_layer_create(layer_small_bounds);
      text_layer_set_text(layer_small_field, "");
      text_layer_set_font(layer_small_field, LECO_REG_13);
      text_layer_set_text_alignment(layer_small_field, GTextAlignmentRight);
      text_layer_set_text_color(layer_small_field, GColorBlack);
      text_layer_set_background_color(layer_small_field, BG_COLOUR );
      layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(layer_small_field));    
  
      // init the distance box  
      GRect layer_big_bounds = GRect(32+x_offset,11+y_offset,75,26);
      layer_big_field = text_layer_create(layer_big_bounds);
      text_layer_set_text(layer_big_field, "" );
      text_layer_set_font(layer_big_field, fonts_get_system_font(FONT_KEY_LECO_26_BOLD_NUMBERS_AM_PM));  
      text_layer_set_text_alignment(layer_big_field, GTextAlignmentRight);
      text_layer_set_background_color(layer_big_field, BG_COLOUR );
      layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(layer_big_field));      
  
      // init the split boxes  
      GRect layer_split_bounds = GRect(0+x_offset,124+y_offset,108,20);
      layer_split_field = text_layer_create(layer_split_bounds);
      text_layer_set_text(layer_split_field, "" );
      text_layer_set_font(layer_split_field, LECO_REG_17);
      text_layer_set_text_alignment(layer_split_field, GTextAlignmentRight);
      text_layer_set_background_color(layer_split_field, BG_COLOUR );
      layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(layer_split_field));              

      // render field images
      field_bitmap_layer = bitmap_layer_create(GRect(5+x_offset, 13+y_offset, 30, 30));
      bitmap_layer_set_compositing_mode(field_bitmap_layer, GCompOpSet);
      layer_add_child(window_get_root_layer(s_window), bitmap_layer_get_layer(field_bitmap_layer));

      // init debug field
      if (DEBUG_FIELDS_ON) {
        GRect layer_debug_bounds = GRect(0,104,108,60);
        layer_debug_field = text_layer_create(layer_debug_bounds);
        text_layer_set_text(layer_debug_field, "" );
        text_layer_set_font(layer_debug_field, fonts_get_system_font(FONT_KEY_GOTHIC_14));
        text_layer_set_text_alignment(layer_debug_field, GTextAlignmentRight);
        text_layer_set_background_color(layer_debug_field, BG_COLOUR );
        layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(layer_debug_field));                      
      }


  
  
}

// callback to render individual digits for the split timer
static void update_gsplit_callback(Layer *layer, GContext *ctx) {
  
  // white background for non-colour devices, or yellow
  graphics_context_set_fill_color(ctx, BG_COLOUR);

  // text is black
  graphics_context_set_text_color(ctx, GColorBlack);

  static int t_width = 12;
  static int t_width_sm = 4;
  static int kerning = -2;
  static int t_height = 19;
  static int t_spacer = 6;
  int t_xoffset = 15;
  static int t_yoffset = 0;
  
  // iterate thru each lap, and render
  for(int i = 0; i < MAX_LAPS; ++i) {
      graphics_fill_rect(ctx, GRect(t_xoffset, t_yoffset+(t_height*i), t_width, t_height), 0, GCornerNone);

      // this is messy - we render each individual digit, just to center it!
      char digitbuff[5];
      memcpy( digitbuff, &lap_text[i][0], 1 );   // this basically is an inline substring, we grab the first char 
      digitbuff[1] = '\0';    
      graphics_draw_text(ctx, digitbuff, LECO_LIGHT_16,  GRect(t_xoffset, t_yoffset+(t_height*i), t_width, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
      memcpy( digitbuff, &lap_text[i][1], 1 );
      digitbuff[1] = '\0';    
      graphics_draw_text(ctx, digitbuff, LECO_LIGHT_16,  GRect(t_xoffset+t_width+(kerning*1), t_yoffset+(t_height*i), t_width, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
      memcpy( digitbuff, &lap_text[i][2], 1 );
      digitbuff[1] = '\0';    
      graphics_draw_text(ctx, digitbuff, LECO_LIGHT_16,  GRect(t_xoffset+(t_width*2)+(kerning*2), t_yoffset+(t_height*i), t_spacer, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
      memcpy( digitbuff, &lap_text[i][3], 1 );
      digitbuff[1] = '\0';    
      graphics_draw_text(ctx, digitbuff, LECO_LIGHT_16,  GRect(t_xoffset+(t_width*2)+t_spacer+(kerning*3), t_yoffset+(t_height*i), t_width, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
      memcpy( digitbuff, &lap_text[i][4], 1 );
      digitbuff[1] = '\0';    
      graphics_draw_text(ctx, digitbuff, LECO_LIGHT_16,  GRect(t_xoffset+(t_width*3)+t_spacer+(kerning*4), t_yoffset+(t_height*i), t_width, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
      memcpy( digitbuff, &lap_text[i][5], 1 );
      digitbuff[1] = '\0';    
      graphics_draw_text(ctx, digitbuff, LECO_LIGHT_16,  GRect(t_xoffset+(t_width*4)+(t_spacer)+(kerning*5), t_yoffset+(t_height*i), t_spacer, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
      memcpy( digitbuff, &lap_text[i][6], 1 );
      digitbuff[1] = '\0';    
      graphics_draw_text(ctx, digitbuff, LECO_LIGHT_16,  GRect(t_xoffset+(t_width*4)+(t_spacer*2)+(kerning*6), t_yoffset+(t_height*i), t_width, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
      memcpy( digitbuff, &lap_text[i][7], 1 );
      digitbuff[1] = '\0';    
      graphics_draw_text(ctx, digitbuff, LECO_LIGHT_16,  GRect(t_xoffset+(t_width*5)+(t_spacer*2)+(kerning*7), t_yoffset+(t_height*i), t_width, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
      memcpy( digitbuff, &lap_text[i][8], 1 );
      digitbuff[1] = '\0';    
      graphics_draw_text(ctx, digitbuff, LECO_LIGHT_16,  GRect(t_xoffset+(t_width*6)+(t_spacer*2)+(kerning*8), t_yoffset+(t_height*i), t_spacer, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
      memcpy( digitbuff, &lap_text[i][9], 1 );
      digitbuff[1] = '\0';    
      graphics_draw_text(ctx, digitbuff, LECO_LIGHT_16,  GRect(t_xoffset+(t_width*6)+(t_spacer*3)+(kerning*9), t_yoffset+(t_height*i), t_width, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
      memcpy( digitbuff, &lap_text[i][10], 1 );
      digitbuff[1] = '\0';    
      graphics_draw_text(ctx, digitbuff, LECO_LIGHT_16,  GRect(t_xoffset+(t_width*7)+(t_spacer*3)+(kerning*10), t_yoffset+(t_height*i), t_width, t_height), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

      #if defined(PBL_ROUND)
        t_xoffset = t_xoffset + 7;
      #endif
  }  
  
}

  
void render_splits() {

      #if defined(PBL_RECT)
        GRect gsplit_bounds = GRect(0,102,120,72);
      #elif defined(PBL_ROUND)
        GRect gsplit_bounds = GRect(16,102,120,72);
      #endif

      gsplit_layer = layer_create(gsplit_bounds);
      layer_set_update_proc(gsplit_layer, update_gsplit_callback);
  
      // Add the text layer to the window
      layer_add_child(window_get_root_layer(s_window), gsplit_layer);      
}

void render_digits() {

      #if defined(PBL_RECT)
        GRect gtext_bounds = GRect(0,64,120,60);
      #elif defined(PBL_ROUND)
        GRect gtext_bounds = GRect(16,64,120,60);
      #endif

      gtext_layer = layer_create(gtext_bounds);
      layer_set_update_proc(gtext_layer, update_gtext_callback);
  
      // Add the text layer to the window
      layer_add_child(window_get_root_layer(s_window), gtext_layer);      
}



void reset_stopwatch() {
  
  // reset all values
  start_time = 0;
  pause_time = 0;
  last_split = 0;
  split_time = 0;
  resume_time = 0;
  elapsed_time = 0;
  mode_is = MODE_DIST;
  state_is = STATE_STOP;
  dist_initial = 0;
  dist_previous = 0;
  dist_latest = 0;
  dist_resume = 0;
  dist_pause = 0;
  pace_previous = 0;
  tick_count_pace = 0;
  tick_count_dist = 0;
  tick_count_hr = 0;

  for(int i = 0; i < MAX_LAPS; ++i) {
    strncpy(lap_text[i],"\0", sizeof(lap_text[i]));
  }
  
  // re-render the stopwatch
  update_stopwatch();
  update_fields();

  // reset the text
  text_layer_set_text(layer_small_field, "");  
  text_layer_set_text(layer_big_field, "" );
  text_layer_set_text(layer_split_field, "" );

  // reset the bitmap
  bitmap_layer_set_bitmap(field_bitmap_layer, NULL);

  // reset the actionbar
  update_actionbar();

  //APP_LOG(APP_LOG_LEVEL_INFO, "Reset stopwatch... %d %d %d %d", dist_initial, dist_previous, dist_latest, pace_previous);    

  
}

// -- update the sub-fields (if required) -----------------------------------------

void update_fields() {
  if (state_is == STATE_RUN || state_is == STATE_PAUSE) {

    if (mode_is == MODE_DIST) {
      text_layer_set_text(layer_big_field, dist_value_buffer );
      text_layer_set_text(layer_small_field, dist_field_buffer);
      bitmap_layer_set_bitmap(field_bitmap_layer, icon_pace);
    } else if (mode_is == MODE_PACE) {
      text_layer_set_text(layer_big_field, pace_value_buffer );
      text_layer_set_text(layer_small_field, pace_field_buffer);
      bitmap_layer_set_bitmap(field_bitmap_layer, icon_running);
    } else if (mode_is == MODE_TIME) {
      text_layer_set_text(layer_big_field, time_buffer );
      if (clock_is_24h_style()) {
        text_layer_set_text(layer_small_field, "24H");        
      } else {
        text_layer_set_text(layer_small_field, AMorPM);                
      }
      bitmap_layer_set_bitmap(field_bitmap_layer, icon_time);      
    } else if (mode_is == MODE_HR) {
      snprintf(hr_placeholder, sizeof(hr_placeholder), "%lu", (uint32_t)field_hr_value);
      text_layer_set_text(layer_big_field, hr_placeholder );
      text_layer_set_text(layer_small_field, "BPM");
      bitmap_layer_set_bitmap(field_bitmap_layer, icon_hr);
    }  

  }
}

void start_stopwatch() {
  reset_stopwatch();
  state_is = STATE_RUN;
	start_time = float_time_ms();
  pause_time = 0;
  last_split = 0;
  split_time = 0;
  resume_time = 0;
  update_timer = app_timer_register(UPDATE_INTERVAL, handle_timer, NULL);
  update_fields();
  update_actionbar();
}

// -- time handler ---------------------------------

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  // Write the current hours and minutes into a buffer
  strftime(time_buffer, sizeof(time_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  strftime(AMorPM, sizeof(AMorPM), "%p", tick_time);
  
  // update field data (if needed)
  update_fields();

}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}


// -- stopwatch control ----------------------------

void pause_stopwatch() {
  state_is = STATE_PAUSE;
  pause_time = float_time_ms();
  
  const HealthMetric dist_metric = HealthMetricWalkedDistanceMeters;
  const HealthValue dist_when_paused = health_service_sum_today(dist_metric);  
  dist_pause = (int)dist_when_paused;
  
  update_actionbar();
}

void resume_stopwatch() {
  state_is = STATE_RUN;  
  resume_time = (float_time_ms() - pause_time) + resume_time;

  const HealthMetric dist_metric = HealthMetricWalkedDistanceMeters;
  const HealthValue distance_now = health_service_sum_today(dist_metric);  
  dist_resume = (int)(distance_now - dist_pause);

  update_timer = app_timer_register(100, handle_timer, NULL);
  update_actionbar();
}


// -- handle the split time ------------------------
void do_split() {
  
  // debugging
  //next_field_type();
  
  double now = float_time_ms();
  
  if (last_split == 0) {
    split_time = elapsed_time;    
  } else {
    split_time = elapsed_time - last_split;
  }   
  
  // now convert split to hours/minutes/seconds/hundreds
  int split_hundreds = (int)(split_time * 60) % 60;
  int split_tenths = (int)(split_time * 10) % 10;
  int split_seconds = (int)split_time % 60;
  int split_minutes = (int)split_time / 60 % 60;
  int split_hours = (int)split_time / 3600;
  
  // build the split layout
  static char text_split[12];
  snprintf(text_split, 14, "%02d:%02d:%02d.%02d", split_hours, split_minutes, split_seconds, split_hundreds);
  
  // transfer previous splits down the list
  for(int i = (MAX_LAPS-1); i > 0; --i) {
    strncpy(lap_text[i], lap_text[i-1], sizeof(lap_text[i]));
  }
  
  // add the latest split
  strncpy(lap_text[0],text_split, sizeof(lap_text[0]));

  // store last split time
  last_split = elapsed_time;
  
  // render the layer
  layer_mark_dirty(gsplit_layer);

}


void debug_digit_type() {
    
  stop_type = stop_type + 1;
  if (stop_type > 4) {
    stop_type = 1;
  }
  
}


void next_field_type() {
    
  stop_type = stop_type + 1;
  if (stop_type > 5) {
    stop_type = 1;
  }
    
}


// -- click handlers ---------------------------------------------------------

void my_up_click_handler() {

  if (state_is == STATE_RUN || state_is == STATE_PAUSE) {

      if (mode_is == MODE_DIST) {
        mode_is = MODE_PACE;
      } else if (mode_is == MODE_PACE) {
        mode_is = MODE_TIME;
      } else if (mode_is == MODE_TIME) {
        if (pebble_has_hr_available && ((uint32_t)field_hr_value > 0)) {
          mode_is = MODE_HR;          
        } else {
          mode_is = MODE_DIST;
        }
      } else if (mode_is == MODE_HR) {
        mode_is = MODE_DIST;
      } 

    // we've changed state, so update fields
    update_fields();

  }
}

void my_select_click_handler() {

  if (state_is == STATE_STOP) {
    start_stopwatch();
  } else if (state_is == STATE_PAUSE) {
    resume_stopwatch();    
  } else if (state_is == STATE_RUN) {
    pause_stopwatch();    
  }

  
}

void my_down_click_handler() {

  if (state_is == STATE_STOP) {
    // do nothing
  } else if (state_is == STATE_PAUSE) {
    reset_stopwatch();
  } else if (state_is == STATE_RUN) {
    do_split();
  }
  
  
}

void update_actionbar() {  
  
  if (state_is == STATE_RUN) {
    action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_UP, icon_fields, true);
    action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_SELECT, icon_pause, true);
    action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_DOWN, icon_split, true);
  }
  if (state_is == STATE_PAUSE) {
    action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_UP, icon_fields, true);
    action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_SELECT, icon_play, true);
    action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_DOWN, icon_trash, true);
  }
  if (state_is == STATE_STOP) {
    action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_UP, NULL, true);
    action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_SELECT, icon_play, true);
    action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_DOWN, NULL, true);
  }
  
}

void click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) my_up_click_handler);    
    window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) my_select_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) my_down_click_handler);    
}


void init_actionbar() {
  
  // Initialize the action bar + load images
  action_bar = action_bar_layer_create();
  load_actionbar_images();
  
  update_actionbar();

  // associate the action bar with the window:
  action_bar_layer_add_to_window(action_bar, s_window);

  // set the click config provider:
  action_bar_layer_set_click_config_provider(action_bar, click_config_provider);

}

// -- main initialisation ----------------------------------------------------------

void init_system() {

  const HealthMetric metric = HealthMetricWalkedDistanceMeters;

  // get the preferred measurement system
  MeasurementSystem unit_type = health_service_get_measurement_system_for_display(metric);
  
  // display to user in correct units
  switch(unit_type) {
  case MeasurementSystemMetric:
    sys_unit_type = SYSTEM_METRIC;
    APP_LOG(APP_LOG_LEVEL_INFO, "Unit is Metric");    
    break;
  case MeasurementSystemImperial: 
    sys_unit_type = SYSTEM_IMPERIAL;
    APP_LOG(APP_LOG_LEVEL_INFO, "Unit is Imperial");    
    break;
  case MeasurementSystemUnknown:
    APP_LOG(APP_LOG_LEVEL_INFO, "MeasurementSystem unknown or does not apply");    
    break;
  }

}

static void init(void) {

  // create a window and get information about the window
	s_window = window_create();
  
  window_set_background_color(s_window, BG_COLOUR);
  
  Layer *window_layer = window_get_root_layer(s_window);

  // initialise the actionbar
  init_actionbar();

  // initialise the time subscriber
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // render the initial fields
  render_fields();
  
  // render the initial large digits
  render_digits();
  
  // and the splits
  render_splits();

//  if (state_is == STATE_RUN) {
      update_timer = app_timer_register(UPDATE_INTERVAL, handle_timer, NULL);
//  }
  
	// Push the window, setting the window animation to 'true'
	window_stack_push(s_window, true);
	
	// app logging..
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Init stopwatch");
  

  
}

static void deinit(void) {

  struct StopwatchState state = (struct StopwatchState){
	.elapsed_time = elapsed_time,
	.start_time = start_time,
	.resume_time = resume_time,
  .temp_time = temp_time,
  .split_time = split_time,
  .last_split = last_split,
  .pause_time = pause_time,
  .dist_initial = dist_initial,
  .dist_latest = dist_latest,
  .dist_previous = dist_previous,
  .dist_resume = dist_resume,
  .dist_pause = dist_pause,
  .pace_previous = pace_previous,
  .state_is = state_is,
  .mode_is = mode_is,
  .stop_type = stop_type
};
  
	status_t status = persist_write_data(PERSIST_STATE, &state, sizeof(state));

  if (status == S_SUCCESS ) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Stored state: %ld", status);
  } else  {
		APP_LOG(APP_LOG_LEVEL_WARNING, "Failed to persist state: %ld", status);
	}
  
  // destroy all layers
  text_layer_destroy(layer_small_field);
  text_layer_destroy(layer_big_field);
  text_layer_destroy(layer_split_field);
  bitmap_layer_destroy(field_bitmap_layer);
  layer_destroy(gtext_layer);
  layer_destroy(gsplit_layer);
  
	// Destroy the window
	window_destroy(s_window);
}

// -- check, store, format number of steps -----------------------------------------------

void checkSteps() {
  const HealthMetric metric = HealthMetricWalkedDistanceMeters;
  const HealthValue distance = health_service_sum_today(metric);

  // get the preferred measurement system
  //MeasurementSystem system = health_service_get_measurement_system_for_display(metric);

  if (dist_initial < 1) {
    dist_initial = distance;
  }
  
  if (DEBUG_FIELDS_ON) {
    snprintf(text_debug_placeholder, sizeof(text_debug_placeholder), "now:%d\ninit:%d\nresume:%d", (int)distance, (int)dist_initial, (int)dist_resume);
    text_layer_set_text(layer_debug_field, text_debug_placeholder );
  }

  
  if (DEBUG_VALUES_ON) {
    // debug distance
    dist_latest = (distance - dist_initial - dist_resume) + dist_latest + 100 + (rand() % 100);
  } else {
    dist_latest = distance - dist_initial - dist_resume;
  }  

  if (dist_latest < 0) {
    dist_initial = distance;
    dist_latest = 0;   
  }
  
  //APP_LOG(APP_LOG_LEVEL_INFO, "Checking distance");    

  if (sys_unit_type == SYSTEM_METRIC) {

    if (dist_latest > 1000) {
      static float dist_km;
      dist_km = ((float)dist_latest)/1000;
      if (dist_km > 99) {
        snprintf(dist_value_buffer, sizeof(dist_value_buffer), "%d.%01d", (int)dist_km, (int)(dist_km*10)%10);
      } else {
        snprintf(dist_value_buffer, sizeof(dist_value_buffer), "%d.%02d", (int)dist_km, (int)(dist_km*100)%100);
      }
      strncpy(dist_field_buffer, "DIST / KM", 40);
    } else {
      snprintf(dist_value_buffer, sizeof(dist_value_buffer), "%d", (int)dist_latest);    
      strncpy(dist_field_buffer, "DIST / M", 40);
    }    
    
  } else if (sys_unit_type == SYSTEM_IMPERIAL) {

    int feet = (int)((float)dist_latest * 3.28F);
    if (dist_latest > 5280) {
      static float dist_mi;
      dist_mi = ((float)dist_latest)/5280;
      if (dist_mi > 99) {
        snprintf(dist_value_buffer, sizeof(dist_value_buffer), "%d.%01d", (int)dist_mi, (int)(dist_mi*10)%10);
      } else {
        snprintf(dist_value_buffer, sizeof(dist_value_buffer), "%d.%02d", (int)dist_mi, (int)(dist_mi*100)%100);
      }
      strncpy(dist_field_buffer, "DIST / Mi", 40);
    } else {
      snprintf(dist_value_buffer, sizeof(dist_value_buffer), "%d", (int)dist_latest);    
      strncpy(dist_field_buffer, "DIST / ft", 40);
    }
    
    
  }
  
  
  // update the fields
  if (mode_is == MODE_DIST) {
    text_layer_set_text(layer_big_field, dist_value_buffer );
    text_layer_set_text(layer_small_field, dist_field_buffer);
  }
}


// -- main event loop -----------------------------------------------------------------

void event_loop() {

  if (state_is == STATE_RUN) {
		double now = float_time_ms();
		elapsed_time = now - start_time - resume_time;
		update_timer = app_timer_register(UPDATE_INTERVAL, handle_timer, NULL);
	} 

	update_stopwatch();

}

void handle_timer(void* data) {

  event_loop();

}


// -- init and handle health data ------------------------------------------

static void handler_health_hr(HealthEventType type, void *context) {
  // if the update was from the Heart Rate Monitor, query it
  if (type == HealthEventHeartRateUpdate) {
    if (DEBUG_VALUES_ON) {
      field_hr_value = (uint32_t) 0x0000008F; // debug HR      
    } else {
      field_hr_value = health_service_peek_current_value(HealthMetricHeartRateBPM);
    }
  }
  
  if (state_is == STATE_RUN || state_is == STATE_PAUSE) {
    if (mode_is == MODE_HR) {
      snprintf(hr_placeholder, sizeof(hr_placeholder), "%lu", (uint32_t)field_hr_value);
      text_layer_set_text(layer_big_field, hr_placeholder );
    }
  }
  
}

static void init_health_hr(void) {
  
  bool success = health_service_set_heart_rate_sample_period(2);

  // Subscribe to health event handler
  health_service_events_subscribe(handler_health_hr, NULL);
  
}

void check_health_avail() {

  #if defined(PBL_HEALTH)
  // Use the step count metric
  static HealthMetric metric_steps = HealthMetricStepCount;
  static HealthMetric metric_hr = HealthMetricHeartRateBPM;

  // Create timestamps for midnight (the start time) and now (the end time)
  time_t start = time_start_of_today();
  time_t end = time(NULL);

  // Check step data is available
  HealthServiceAccessibilityMask mask_steps = health_service_metric_accessible(metric_steps,start, end);
  pebble_has_health_available = mask_steps & HealthServiceAccessibilityMaskAvailable;

  // this is what pebble use in their example , but it doesn't work...
  HealthServiceAccessibilityMask mask_hr = health_service_metric_accessible(HealthMetricHeartRateBPM, time(NULL), time(NULL));
  // this works...
  //HealthServiceAccessibilityMask mask_hr = health_service_metric_accessible(metric_hr,start, end);
  pebble_has_hr_available = mask_hr & HealthServiceAccessibilityMaskAvailable;

  #else
  // Health data is not available here
  pebble_has_health_available = false;
  pebble_has_hr_available = false;
  #endif
  
  if (pebble_has_hr_available) {
    init_health_hr();
  }
}



// -- update all timing data for the stopwatch -------------------------------------

void update_stopwatch() {
  
  // Now convert "elapsed_time" to hours/minutes/seconds/tenths/hundreds
  hundreds = (int)(elapsed_time * 60) % 60;
  tenths = (int)(elapsed_time * 10) % 10;
  seconds = (int)elapsed_time % 60;
  minutes = (int)elapsed_time / 60 % 60;
  hours = (int)elapsed_time / 3600;

  //APP_LOG(APP_LOG_LEVEL_DEBUG, "got an update!");

  // let's check distance every 10s
  if (tick_count_dist < elapsed_time && pebble_has_health_available) {
    //APP_LOG(APP_LOG_LEVEL_INFO, "tick_count_dist triggered....");    
    tick_count_dist = elapsed_time + TICK_INTERVAL;  

    checkSteps();      
  }
  
  if (tick_count_pace < elapsed_time && pebble_has_health_available) {
    //APP_LOG(APP_LOG_LEVEL_INFO, "tick_count_pace triggered....");    
    tick_count_pace = elapsed_time + TICK_INTERVAL_PACE;
    
    if (dist_previous < 1) {
      
      dist_previous = dist_latest;

    } else { 
      
      float meters_per_sec = (float)(dist_latest - dist_previous)/TICK_INTERVAL_PACE;
      float meters_per_min = (float)meters_per_sec*60;
      float minutes_per_km = 0;
      
      if (sys_unit_type == SYSTEM_METRIC) {
        minutes_per_km = (float)1000/meters_per_min;
        strcpy(pace_field_buffer, "PaCE / KM");
      } else if (sys_unit_type == SYSTEM_IMPERIAL) {
        int feet_per_min = (int)((float)meters_per_min * 3.28F);
        minutes_per_km = (float)5280/feet_per_min;
        strcpy(pace_field_buffer, "PaCE / Mi");
      }

      if (pace_previous == 0) {
        pace_previous = minutes_per_km;
      }
      
      // average out the pace
      float avg_pace = (pace_previous + minutes_per_km) / 2;

      if (avg_pace < 59) {
        snprintf(pace_value_buffer, sizeof(pace_value_buffer), "%02d:%02d", (int)avg_pace, (int)(avg_pace*60)%60);
      } else {
        minutes_per_km = 0;
        strcpy(pace_value_buffer, "--:--");
      }
      dist_previous = dist_latest;
      pace_previous = minutes_per_km;
    }

}
  
  
  // set the digit layout based on time
  if (elapsed_time < 59) { // 59s - 4 big digits (00.00)
    stop_type = 1;
  }

  if (elapsed_time > 60) { // 60s - 3 large, 2 small digits (0:00.00)
    stop_type = 2;
  }

  if (elapsed_time > 599) { // 599 - 6 med, 2 small digits (00:00.00)
    stop_type = 3;
  }

  if (elapsed_time > 3599) { // 3599 - 6 medium digits (00:00:00)
    stop_type = 4;
  }  
  
  // render digits
  layer_mark_dirty(gtext_layer);

}

void restore_data() {

  struct StopwatchState state;

  if(persist_read_data(PERSIST_STATE, &state, sizeof(state)) != E_DOES_NOT_EXIST) {
    elapsed_time = state.elapsed_time;
    start_time = state.start_time;
    resume_time = state.resume_time;
    temp_time = state.temp_time;
    split_time = state.split_time;
    last_split = state.last_split;
    pause_time = state.pause_time;
    dist_initial = state.dist_initial;
    dist_latest = state.dist_latest;
    dist_previous = state.dist_previous;
    dist_resume = state.dist_resume;
    dist_pause = state.dist_pause;
    pace_previous = state.pace_previous;
    state_is = state.state_is;
    mode_is = state.mode_is;
    stop_type = state.stop_type;  
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Loaded persisted state.");
	}


}

// -- main function ---------------------------------

int main(void) {
  init_fonts();
  init_system();
  load_field_images();
  check_health_avail();
  restore_data();
	init();
	app_event_loop();
	deinit();
}
