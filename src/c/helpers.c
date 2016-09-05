#include "helpers.h"

 TextLayer *init_text_layer(GRect location, GColor color, GColor background,
  const char * font_key, GTextAlignment alignment) {
    TextLayer *layer = text_layer_create(location);
    text_layer_set_text_color(layer, color);
    text_layer_set_background_color(layer, background);
    text_layer_set_font(layer, fonts_get_system_font(font_key));
    text_layer_set_text_alignment(layer, alignment);

    return layer;
  }
