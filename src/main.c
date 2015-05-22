#include <pebble.h>
#include "effect_layer.h"
  
#ifdef PBL_COLOR
  #define color_screen 1
#else
  #define color_screen 0
#endif

#define screen_width 144
#define screen_height 168
  
#define current_time_color GColorBlack
#define not_connected_background_color GColorChromeYellow
#define printer_error_background_color GColorRed
#define progress_background_color GColorMediumSpringGreen
#define settings_not_set_background_color GColorVividCerulean

#define operational_state 1
#define settings_not_defined_state 2

// UI Elements variables
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_print_time_left_layer;
static TextLayer *s_info_layer;
static EffectLayer *progress_percent_layer;

static void update_progress(int percent_complete){
  APP_LOG(APP_LOG_LEVEL_INFO, "%d %% complete", percent_complete);
  Layer* layer = effect_layer_get_layer(progress_percent_layer);
  float progress_percent_height = (screen_height*(percent_complete*0.01));
  APP_LOG(APP_LOG_LEVEL_INFO, "%fpx height", progress_percent_height);
  layer_set_frame(layer, GRect(0, screen_height-progress_percent_height, screen_width, progress_percent_height));
  if(percent_complete > 0){
    static char percent_text[] = "100%";
    snprintf(percent_text, sizeof(percent_text), "%d%%", percent_complete);
    text_layer_set_text(s_info_layer, percent_text); 
  }
}

void process_tuple(Tuple *t){
  int key = t->key;
  int value = t->value->int32;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Got key %d, value %d", key, value);
  switch(key){
    // Print progress percent
    case 0:
      update_progress(value);
      break;
    // Connected state (true/false)
    case 1:
      // not connected
      if(value == 0){
        update_progress(0);
        text_layer_set_text(s_info_layer, "Connection error");
      }else {
        text_layer_set_text(s_info_layer, "Connected");
      }
      break;
    // Print time left
    case 2:
//       char tempvar[] = t->key;
//       text_layer_set_text(s_print_time_left_layer, tempvar);
//       text_layer_set_text(s_print_time_left_layer, t->key->);
      break;
    // Printer state
    case 3:
      switch(value){
        // Operational state
        case operational_state:
          update_progress(99);
          text_layer_set_text(s_print_time_left_layer, "00:00");
          text_layer_set_text(s_info_layer, "Printer Ready");
          break;
        case settings_not_defined_state:
          text_layer_set_text(s_print_time_left_layer, "00:00");
          text_layer_set_text(s_info_layer, "Settings not defined");
          break;
      }
      break;
  }
}
 
void inbox(DictionaryIterator *iter, void *context){
  Tuple *t = dict_read_first(iter);
  if(t){
    process_tuple(t);
  }
  while(t != NULL){
    t = dict_read_next(iter);
    if(t){
      process_tuple(t);
    }
  }
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a buffer for the time
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use the 24h format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // Use the 12hr format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }

  // send message to js to update the progress
  
  // Display the time
  text_layer_set_text(s_time_layer, buffer);
}

static void main_window_load(Window *window) {
  // ------------------------ Time Text Layer ------------------------
  s_time_layer = text_layer_create(GRect(0, 25, 144, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, current_time_color);
  text_layer_set_text(s_time_layer, "00:00");

  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  // ------------------------ Time Text Layer ------------------------
  
  // ------------------------ Print Time Left Text Layer ------------------------
  s_print_time_left_layer = text_layer_create(GRect(0, 65, 144, 50));
  text_layer_set_background_color(s_print_time_left_layer, GColorClear);
  text_layer_set_text_color(s_print_time_left_layer, current_time_color);
  text_layer_set_text(s_print_time_left_layer, "00:00");

  text_layer_set_font(s_print_time_left_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_text_alignment(s_print_time_left_layer, GTextAlignmentCenter);

  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_print_time_left_layer));
  // ------------------------ Print Time Left Text Layer ------------------------
  
  // ------------------------ Info Text Layer ------------------------
  s_info_layer = text_layer_create(GRect(3, 130, screen_width, 30));
  text_layer_set_background_color(s_info_layer, GColorClear);
  text_layer_set_text_color(s_info_layer, current_time_color);
  text_layer_set_text(s_info_layer, "Loading...");

  text_layer_set_font(s_info_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(s_info_layer, GTextAlignmentLeft);

  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_info_layer));
  // ------------------------ Info Text Layer ------------------------
  
  // ------------------------ Progress Percent Layer ------------------------
  progress_percent_layer = effect_layer_create(GRect(0, 0, screen_width, screen_height));
  effect_layer_add_effect(progress_percent_layer, effect_invert, NULL);
  
  layer_add_child(window_get_root_layer(window), effect_layer_get_layer(progress_percent_layer));
  // ------------------------ Progress Percent Layer ------------------------
  
  // Make sure the time is displayed from the start
  update_time();
  
}

static void main_window_unload(Window *window) {
  // Destroy the time text layer
  text_layer_destroy(s_time_layer);
  
  // Destroy the progress percent layer
  effect_layer_destroy(progress_percent_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}
  
static void init() {
  // Create main window element
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  // Set the window background color
  window_set_background_color(s_main_window, COLOR_FALLBACK(settings_not_set_background_color, GColorWhite));
  
  // Display the window
  window_stack_push(s_main_window, true);
  
  app_message_register_inbox_received(inbox);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  
  // Subscribe to the time ticker on the minute unit
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  // Destroy the window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
