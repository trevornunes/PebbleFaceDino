#include <pebble.h>
  
// Pebble 'dino' watchface.
// Animates 'dingbat' TTFs 
  
  
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_msg_layer;
static TextLayer *s_bottom_layer;

static char s_battery_buffer[16];
static GFont s_time_font;
static GFont  s_msg_font; 
static GFont s_bottom_font;

static char s_msg[] = "      "; 
static char s_ascTable[256];

static void update_time();
void battery_life();


//
// update_msg
//
static void update_msg() { 
  
  static int timeRotater;
  time_t temp;
  struct tm *tick_time;
    
  static char buffer[] = "Saturday, Jul 13 1:32.59  ";
  static int idx = 65;  
  static int charPos = 0;
  
  idx = 65 + (rand() % 25);
   
  
  if ( charPos++ >= 3) {  // position cycle
     charPos = 0;
  } 
  if ( timeRotater++ >= 10 ) {  // 1-10s tick
     timeRotater = 0;
  }

  switch( timeRotater ) {
           case 1:  // At 1 second interval ...
               update_time();
               break;
           case 3: 
           case 4:
               battery_life();
               text_layer_set_text(s_bottom_layer,s_battery_buffer);
               break;
           case 6:
               temp = time(NULL);
               tick_time = localtime(&temp);
               strftime(buffer, sizeof("Saturday, Jul 13        "), "%A, %b %d", tick_time);
               text_layer_set_text(s_bottom_layer,buffer);  
               break;
  }           
  
  memset( s_msg, 32, 6);
  s_msg[5] = '\0';
  memcpy( &s_msg[charPos], &s_ascTable[idx], 1);
  text_layer_set_text(s_msg_layer, s_msg );
  text_layer_set_text(s_msg_layer, s_msg );
}


//
//  battery_life
//
void battery_life() {
  
   BatteryChargeState charge_state = battery_state_service_peek();
   if (charge_state.is_charging) {
        snprintf(s_battery_buffer, sizeof(s_battery_buffer), "charging");
   } else {
        snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d%% battery", charge_state.charge_percent);
     }
}


static void update_time() {
  
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  static char buffer[] = "00:00";
    
  srand( tick_time->tm_sec + tick_time->tm_min );
  
  if(clock_is_24h_style() == true) {
      strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
      strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }
  text_layer_set_text(s_time_layer, buffer);  
}

static void main_window_load(Window *window) {
  // Create time TextLayer
  // 5,52,139,50
  s_time_layer = text_layer_create(GRect(0, 0, 145, 68));
  s_msg_layer = text_layer_create(GRect(10,100,145,56));      // 5,68,145,80
  s_bottom_layer = text_layer_create(GRect(0,69,145,16)); // 0,136,145,60
  
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text(s_time_layer, "00:00");
  
  text_layer_set_background_color(s_msg_layer, GColorWhite);
  text_layer_set_text_color(s_msg_layer, GColorBlack);
  text_layer_set_text(s_msg_layer,s_msg);
  
  text_layer_set_background_color(s_bottom_layer, GColorBlack);
  text_layer_set_text_color(s_bottom_layer,GColorWhite);
  text_layer_set_text(s_bottom_layer,"Mushy");
  
  //Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_CALC_60));
  s_msg_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DINO_48));
  

  //Apply to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  text_layer_set_font(s_msg_layer, s_msg_font);
  text_layer_set_text_alignment(s_msg_layer, GTextAlignmentCenter);

  // Use built in font for lower layers
  text_layer_set_font(s_bottom_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_bottom_layer,GTextAlignmentCenter);
  
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_msg_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_bottom_layer));

  // Make sure the time is displayed from the start
  update_msg();
  update_time();
}

static void main_window_unload(Window *window) {
  //Unload GFont
  fonts_unload_custom_font(s_time_font);
  
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_msg_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}
  
static void tick_secs_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_msg();
}

static void init() {
  
  int i = 0;
  for (i =0; i < 255; i++ ) {
      s_ascTable[i] = i;
  }
  
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  tick_timer_service_subscribe(SECOND_UNIT, tick_secs_handler);
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
