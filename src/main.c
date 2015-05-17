#include <pebble.h>
  
#define screen_width 144
#define screen_height 168
  
#ifdef PBL_COLOR 
  #define color_screen 1
#else
  #define color_screen 0
#endif  


#define current_time_color GColorWhite
#define not_connected_background_color GColorChromeYellow
#define printer_error_background_color GColorRed
#define progress_background_color GColorMediumSpringGreen
#define settings_not_set_background_color GColorVividCerulean

// UI Elements variables
static Window *s_main_window;
static TextLayer *s_time_layer;
static Layer *progress_percent_layer;

static float progress_percent; 


void setConnected(Layer *layer, int connected){
  // Destroy the progress percent layer
  if(connected){
    APP_LOG(APP_LOG_LEVEL_INFO, "connected");
    if(layer_get_hidden(layer)){
      layer_set_hidden(layer, false);
    }
  }else {
    APP_LOG(APP_LOG_LEVEL_INFO, "not connected");
    if(!(layer_get_hidden(layer))){
      layer_set_hidden(layer, true);
    }
  }
}

void process_tuple(Tuple *t){
  int key = t->key;
  int value = t->value->int32;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Got key %d, value %d", key, value);
  switch(key){
    case 0:
      // Connected
      setConnected(progress_percent_layer, value);
      APP_LOG(APP_LOG_LEVEL_INFO, "%d", value);
      break;
    case 1:;
      DictionaryIterator *iter;
      app_message_outbox_begin(&iter);
 
      if (iter == NULL) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Iter is null! Returning");
        return;
      }
 
      dict_write_uint8(iter, 1, rand() % 3);
      dict_write_end(iter);
 
      app_message_outbox_send();
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

static void some_update_proc(Layer *this_layer, GContext *ctx){
  graphics_context_set_fill_color(ctx, COLOR_FALLBACK(progress_background_color, GColorBlack));
  float fill_height = screen_height-(screen_height*(progress_percent/100));
  graphics_fill_rect(ctx, GRect(0, screen_height-fill_height + 1, screen_width, fill_height), 0, GCornerNone);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "progress layer updated");
}

static void update_progress(int percent_complete){
  progress_percent = percent_complete;
  layer_mark_dirty(progress_percent_layer);
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

  update_progress(rand() % 100);
  
  // Display the time
  text_layer_set_text(s_time_layer, buffer);
}

static void main_window_load(Window *window) {
  // Create the progress percent layer
  progress_percent_layer = layer_create(GRect(0, 0, screen_width, screen_height));
  layer_set_update_proc(progress_percent_layer, some_update_proc);
  
  layer_add_child(window_get_root_layer(window), progress_percent_layer);
  
  setConnected(progress_percent_layer, 1);
  
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 55, 144, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, current_time_color);
  text_layer_set_text(s_time_layer, "00:00");

  // Improve the layout to be more like a watchface
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
  // Make sure the time is displayed from the start
  update_time();
  
}

static void main_window_unload(Window *window) {
  // Destroy the time text layer
  text_layer_destroy(s_time_layer);
  
  // Destroy the progress percent layer
  layer_destroy(progress_percent_layer);
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
