#include <pebble.h>
#include "helpers.h"

static const char APPNAME[] = "Transponder";

static Window *s_main_window;
static TextLayer *s_logo_text, *s_info_text;
static char server_url[256];
static char server_secret[256];

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Layout logo and info text
  s_logo_text = init_text_layer(GRect(0, 0, bounds.size.w, 35)
  , GColorWhite, GColorVividCerulean, FONT_KEY_GOTHIC_24_BOLD, GTextAlignmentCenter);
  text_layer_set_text(s_logo_text, APPNAME);

  s_info_text = init_text_layer(GRect(5, 35, bounds.size.w - 5, bounds.size.h - 35)
  , GColorBlack, GColorWhite, FONT_KEY_GOTHIC_18, GTextAlignmentLeft);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_logo_text));
  layer_add_child(window_layer, text_layer_get_layer(s_info_text));
}

static void main_window_unload(Window *window) {

}

static void prv_user_setup() {
  /*
   * If we are missing the server url and server secret we will instruct
   * the user to open the configuration on their phone.
   */
   if (persist_read_string(MESSAGE_KEY_SERVERURL, server_url, 256) == E_DOES_NOT_EXIST ||
   persist_read_string(MESSAGE_KEY_SERVERSECRET, server_secret, 256) == E_DOES_NOT_EXIST) {
     APP_LOG(APP_LOG_LEVEL_DEBUG, "Server url or key not in Storage");
     text_layer_set_text(s_info_text, "To continue setup please go to the configuration"
                                      " page inside of the pebble app on your phone.");
   }
   //TODO Tell the user everythings already setup!
}
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
  // Read color preferences
  Tuple *serverUrl = dict_find(iter, MESSAGE_KEY_SERVERURL);
  if(serverUrl) {
    strcpy(server_url, serverUrl->value->cstring);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Server url now set to %s", server_url);
  }
}

static void prv_init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Open AppMessage connection
  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
}

static void prv_deinit() {
  //Destroy the text layer(s)
  text_layer_destroy(s_logo_text);
  text_layer_destroy(s_info_text);
  // Destroy Window
  window_destroy(s_main_window);
}

int main() {
  prv_init();
  prv_user_setup();
  app_event_loop();

  prv_deinit();
}
