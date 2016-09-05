#include <pebble.h>
#include "helpers.h"

static const char APPNAME[] = "Transponder";

static Window *s_main_window;
static TextLayer *s_logo_text, *s_info_text;
static char server_url[PERSIST_DATA_MAX_LENGTH];
static char server_secret[PERSIST_DATA_MAX_LENGTH];
static bool timeline_setup = false;

//Make sure communicatoin from js is ready
static bool s_js_ready;
static AppTimer *s_jscomm_timer;
bool comm_is_js_ready() {
  return s_js_ready;
}

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

//Declare some prototypes
static void send_message();
static void test_message();
static void timout_timer_handler(void *context) {
  // Retry
  send_message();
}

static void send_message() {
  //Make sure js comm is ready
  if (!comm_is_js_ready()){
    APP_LOG(APP_LOG_LEVEL_DEBUG, "c <=> js comm not open yet.");
    const int interval_ms = 1000;
    s_jscomm_timer = app_timer_register(interval_ms, timout_timer_handler, NULL);
    return;
  }
  if (s_jscomm_timer)
    s_jscomm_timer = NULL;
  test_message();
}

static void prv_user_setup() {
  if (persist_read_bool(MESSAGE_KEY_ISSETUP)) {
    //The app is already setup update the info text and return
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Everything is setup.  Receiving pins");
    text_layer_set_text(s_info_text, "Your watch should now be receiving timeline pins from sonarr."
                                     " To stop receiving pins open the configuration page.");
    return;
  }
  /*
   * If we are missing the server url and server secret we will instruct
   * the user to open the configuration on their phone.
   */
   if (persist_read_string(MESSAGE_KEY_SERVERURL, server_url, sizeof(server_url)) == E_DOES_NOT_EXIST ||
   persist_read_string(MESSAGE_KEY_SERVERSECRET, server_secret, sizeof(server_secret)) == E_DOES_NOT_EXIST) {
     APP_LOG(APP_LOG_LEVEL_DEBUG, "Server url or key not in Storage");
     text_layer_set_text(s_info_text, "To continue setup please go to the configuration"
                                      " page inside of the pebble app on your phone.");
   }
}

static void test_message() {
  //TESTING
  // Declare the dictionary's iterator
  DictionaryIterator *out_iter;

  // Prepare the outbox buffer for this message
  AppMessageResult result = app_message_outbox_begin(&out_iter);
  if(result == APP_MSG_OK) {
    // Add an item to ask for weather data
    int value = 589;
    dict_write_int(out_iter, MESSAGE_KEY_RequestData, &value, sizeof(int), true);

    // Send this message
    result = app_message_outbox_send();
    if(result != APP_MSG_OK) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox: %d", (int)result);
    }
  } else {
    // The outbox cannot be used right now
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)result);
  }
}

static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *ready_tuple = dict_find(iter, MESSAGE_KEY_JSREADY);
  if(ready_tuple) {
    // PebbleKit JS is ready! Safe to send messages
    s_js_ready = true;
  }
  // Read user preferences
  Tuple *serverUrl = dict_find(iter, MESSAGE_KEY_SERVERURL);
  if(serverUrl) {
    strcpy(server_url, serverUrl->value->cstring);
    persist_write_string(MESSAGE_KEY_SERVERURL, server_url);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Server url now set to %s", server_url);
  }
  Tuple *serverSecret = dict_find(iter, MESSAGE_KEY_SERVERSECRET);
  if(serverSecret) {
    strcpy(server_secret, serverSecret->value->cstring);
    persist_write_string(MESSAGE_KEY_SERVERSECRET, server_secret);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Server secret now set to %s", server_secret);
  }

  //if we are not already setup attempt to register
  if (!persist_read_bool(MESSAGE_KEY_ISSETUP)) {
    //register
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
