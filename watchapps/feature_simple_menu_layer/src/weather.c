#include "pebble.h"

static Window *window;
static TextLayer *text_layer;


static TextLayer *temperature_layer;
static TextLayer *city_layer;
static BitmapLayer *icon_layer;
static GBitmap *icon_bitmap = NULL;


/* MENU */
#define NUM_MENU_SECTIONS 2
#define NUM_FIRST_MENU_ITEMS 3
#define NUM_SECOND_MENU_ITEMS 1
// This is a simple menu layer
static SimpleMenuLayer *simple_menu_layer;

// A simple menu layer can have multiple sections
static SimpleMenuSection menu_sections[NUM_MENU_SECTIONS];

// Each section is composed of a number of menu items
static SimpleMenuItem first_menu_items[NUM_FIRST_MENU_ITEMS];

static SimpleMenuItem second_menu_items[NUM_SECOND_MENU_ITEMS];

// Menu items can optionally have icons
static GBitmap *menu_icon_image;

static bool special_flag = false;

static int hit_count = 0;


/* MENU */

// You can capture when the user selects a menu icon with a menu item select callback
static void menu_select_callback(int index, void *ctx) {
  // Here we just change the subtitle to a literal string
  first_menu_items[index].subtitle = "You've hit select here!";
  // Mark the layer to be updated
  layer_mark_dirty(simple_menu_layer_get_layer(simple_menu_layer));
}




// You can specify special callbacks to differentiate functionality of a menu item
static void special_select_callback(int index, void *ctx) {
  // Of course, you can do more complicated things in a menu item select callback
  // Here, we have a simple toggle
  special_flag = !special_flag;

  SimpleMenuItem *menu_item = &second_menu_items[index];

  if (special_flag) {
    menu_item->subtitle = "Okay, it's not so special.";
  } else {
    menu_item->subtitle = "Well, maybe a little.";
  }

  if (++hit_count > 5) {
    menu_item->title = "Very Special Item";
  }

  // Mark the layer to be updated
  layer_mark_dirty(simple_menu_layer_get_layer(simple_menu_layer));
}
/* MENU FUNCTIONS */



static AppSync sync;
static uint8_t sync_buffer[64];

enum WeatherKey {
  WEATHER_ICON_KEY = 0x0,         // TUPLE_INT
  WEATHER_TEMPERATURE_KEY = 0x1,  // TUPLE_CSTRING
  WEATHER_CITY_KEY = 0x2,         // TUPLE_CSTRING
};

enum SensorKeys {
  TEMP = 0x0,         // TUPLE_INT
  FLOWRATE = 0x1,  // TUPLE_CSTRING
  VALVE1P = 0x2,         // TUPLE_CSTRING
  VALVE2P = 0x3,         // TUPLE_INT
  BAYALVL = 0x4,  // TUPLE_CSTRING
  BAYBLVL = 0x5,         // TUPLE_CSTRING
  BAYCRIT = 0x6,         // TUPLE_INT
  TEMPCRIT = 0x7,  // TUPLE_CSTRING
  VALVPCRIT = 0x8,         // TUPLE_CSTRING
};
enum EQUIP {
  equipType = 0x0,
  equipID = 0x1,
  crit = 0x2,
  statys = 0x4,
};

static const uint32_t WEATHER_ICONS[] = {
  RESOURCE_ID_IMAGE_SUN, //0
  RESOURCE_ID_IMAGE_CLOUD, //1
  RESOURCE_ID_IMAGE_RAIN, //2
  RESOURCE_ID_IMAGE_SNOW //3
};


static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Select");
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Up");
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Down");
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}


static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}




static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
 	//printf("key: %d \n" , (int) key);
 	
       APP_LOG(APP_LOG_LEVEL_DEBUG, "We received: %d ", (int) key);
//     char str2[120];
// 	strcpy(str2, new_tuple->value->cstring);
// 	
//     char *options = strtok(str2,",");
//     int value = atoi(options);
// 
//   	APP_LOG(APP_LOG_LEVEL_DEBUG, "Value: %d ", value );
// 
//    	options =strtok(NULL,",");
// 
// 	value = atoi(options);
// 
//   	APP_LOG(APP_LOG_LEVEL_DEBUG, "Value2: %d ", value );

  switch (key) {
//     case 0x0:
//       if (icon_bitmap) {
//        // gbitmap_destroy(icon_bitmap);
//       }
//      // icon_bitmap = gbitmap_create_with_resource(WEATHER_ICONS[new_tuple->value->uint8]);
//      // bitmap_layer_set_bitmap(icon_layer, icon_bitmap);
//       break;
// 
    case 0x0:
     if(strlen(new_tuple->value->cstring) > 0){
      // App Sync keeps new_tuple in sync_buffer, so we may use it directly
      //text_layer_set_text(temperature_layer, new_tuple->value->cstring);
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Data: %s ", new_tuple->value->cstring);
    
      first_menu_items[0].title= new_tuple->value->cstring;
      //text_layer_set_text(city_layer, new_tuple->value->cstring);
	};
      break;

    case 0x1:
     APP_LOG(APP_LOG_LEVEL_DEBUG, "Data: %s ", new_tuple->value->cstring);
     if(strlen(new_tuple->value->cstring) > 0){
     
      first_menu_items[0].subtitle = new_tuple->value->cstring;
     }; 
      //text_layer_set_text(city_layer, new_tuple->value->cstring);
      break;
  }
}

static void send_cmd(void) {
  Tuplet value = TupletInteger(1, 1);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    return;
  }

  dict_write_tuplet(iter, &value);
  dict_write_end(iter);

  app_message_outbox_send();
}

static void window_load(Window *window) {
 /*    MENU   INIT END */

  int num_a_items = 0;
  first_menu_items[num_a_items++] = (SimpleMenuItem){
    // You should give each menu item a title and callback
    .title = "First Item",
    .callback = menu_select_callback,
  };
  // The menu items appear in the order saved in the menu items array
  first_menu_items[num_a_items++] = (SimpleMenuItem){
    .title = "Second Item",
    // You can also give menu items a subtitle
    .subtitle = "Here's a subtitle",
    .callback = menu_select_callback,
  };
  first_menu_items[num_a_items++] = (SimpleMenuItem){
    .title = "Third Item",
    .subtitle = "This has an icon",
    .callback = menu_select_callback,
    // This is how you would give a menu item an icon
    .icon = menu_icon_image,
  };

  // This initializes the second section
  second_menu_items[0] = (SimpleMenuItem){
    .title = "Special Item",
    // You can use different callbacks for your menu items
    .callback = special_select_callback,
  };

  // Bind the menu items to the corresponding menu sections
  menu_sections[0] = (SimpleMenuSection){
    .num_items = NUM_FIRST_MENU_ITEMS,
    .items = first_menu_items,
  };
  menu_sections[1] = (SimpleMenuSection){
    // Menu sections can also have titles as well
    .title = "Yet Another Section",
    .num_items = NUM_SECOND_MENU_ITEMS,
    .items = second_menu_items,
  };

  // Now we prepare to initialize the simple menu layer
  // We need the bounds to specify the simple menu layer's viewport size
  // In this case, it'll be the same as the window's
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  // Initialize the simple menu layer
  simple_menu_layer = simple_menu_layer_create(bounds, window, menu_sections, NUM_MENU_SECTIONS, NULL);

  // Add it to the window for display
  layer_add_child(window_layer, simple_menu_layer_get_layer(simple_menu_layer));
  
  
  
  /*    MENU   INIT END */
  
//  Layer *window_layer = window_get_root_layer(window);
//  GRect bounds = layer_get_bounds(window_layer);
//  icon_layer = bitmap_layer_create(GRect(32, 10, 80, 80));
//  layer_add_child(window_layer, bitmap_layer_get_layer(icon_layer));

 /* temperature_layer = text_layer_create(GRect(0, 95, 144, 68));
  text_layer_set_text_color(temperature_layer, GColorWhite);
  text_layer_set_background_color(temperature_layer, GColorClear);
  text_layer_set_font(temperature_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(temperature_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(temperature_layer));
*/
 /* 
  city_layer = text_layer_create(GRect(0, 125, 144, 68));
  text_layer_set_text_color(city_layer, GColorWhite);
  text_layer_set_background_color(city_layer, GColorClear);
  text_layer_set_font(city_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(city_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(city_layer));
*/
/*
  text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(text_layer, "Press a button");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
*/

  Tuplet initial_values[] = {
   // TupletInteger(WEATHER_ICON_KEY, (uint8_t) 1),
    TupletCString(TEMP, "Test Equipment"),
    TupletCString(FLOWRATE, "-99.9"),
    TupletCString(VALVE1P, "-99.9"),
    TupletCString(VALVE2P, "-99.9"),
    TupletCString(BAYALVL, "-99.9"),
    TupletCString(BAYBLVL, "-99.9"),
//     TupletCString(BAYCRIT, "-99.9"),
//     TupletCString(TEMPCRIT, "-99.9"),
//     TupletCString(VALVPCRIT, "-99.9"),
  };

  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);

  send_cmd();
}

static void window_unload(Window *window) {
  app_sync_deinit(&sync);
  simple_menu_layer_destroy(simple_menu_layer);
  
  if (icon_bitmap) {
    gbitmap_destroy(menu_icon_image);
    gbitmap_destroy(icon_bitmap);
  }

  text_layer_destroy(text_layer);
//  text_layer_destroy(city_layer);
//  text_layer_destroy(temperature_layer);
  bitmap_layer_destroy(icon_layer);
}
// void tick_handler(struct tm *tick_time, TimeUnits units_changed)
// {
//     //Here we will update the watchface display
//     //Format the buffer string using tick_time as the time source
//     strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
//     //Change the TextLayer text to show the new time!
//     text_layer_set_text(text_layer, buffer);
// }


static void init(void) {
//   window = window_create();
//   window_set_background_color(window, GColorBlack);
//   window_set_fullscreen(window, true);
//   window_set_click_config_provider(window, click_config_provider);
//   window_set_window_handlers(window, (WindowHandlers) {
//     .load = window_load,
//     .unload = window_unload
//   });
// 

//tick_timer_service_subscribe(SECOND_UNIT, (TickHandler) tick_handler);
//char buffer[140];

 
  const int inbound_size = 64;
  const int outbound_size = 64;
  app_message_open(inbound_size, outbound_size);

  const bool animated = true;
//  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  //init();
  
 // app_event_loop();
 //deinit();

window = window_create();

  // Setup the window handlers
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  init();
  window_stack_push(window, true /* Animated */);

  app_event_loop();

  window_destroy(window);

}
