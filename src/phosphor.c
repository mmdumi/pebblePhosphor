#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#include "resource_ids.auto.h"

#define MY_UUID { 0xCE, 0xC0, 0x94, 0xD9, 0xE7, 0xC8, 0x4E, 0xFA, 0xB8, 0xAD, 0xA1, 0x41, 0xC8, 0x6A, 0x7D, 0xB0 }
PBL_APP_INFO(MY_UUID,
             "Phosphor", "Wip Interactive",
             1, 0, 0,
			 APP_INFO_WATCH_FACE);

Window window;
TextLayer amLayer;
TextLayer hourLayer;
TextLayer dotsLayer;
TextLayer minuteLayer;

int hour = -1;

void display_time(PblTm *t)
{
  static char timeText[] = "00";
  static char hourText[] = "00";  
  static char amText[] = "am";
  static char pmText[] = "pm";

  // Always update the minute
  string_format_time(timeText, sizeof(timeText), "%M", t);
  text_layer_set_text(&minuteLayer, timeText);

  // Update the hour only if it's changed
  if (hour != t->tm_hour) {
	hour = t->tm_hour;

	char *format = clock_is_24h_style() ? "%k" : "%l";
	string_format_time(hourText, sizeof(hourText), format, t);
	text_layer_set_text(&hourLayer, hourText);	
  }

  // Update am/pm if it's not the 24h style set in settings
  if (!clock_is_24h_style()) {
	char *value = amText;
	if (t->tm_hour >= 12) {
		value = pmText;
	}
	
	const char *currentText = text_layer_get_text(&amLayer);
	if (memcmp(currentText, value, 2*sizeof(char)) != 0) {
		// Update text layer
		text_layer_set_text(&amLayer, value);
	}
  }
}

void handle_init(AppContextRef ctx) {
  (void)ctx;

  window_init(&window, "Phosphor");
  window_stack_push(&window, true /* Animated */);
  window_set_background_color(&window, GColorBlack);
  
  resource_init_current_app(&APP_RESOURCES);
   	
  // Custom fonts
  GFont timeFont = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TEXT_TIME_30));
  GFont amFont = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TEXT_AM_18));

  //  AM / PM Layer
  text_layer_init(&amLayer, GRect(10, 60, 144-20, 24));
  text_layer_set_text(&amLayer, "");
  text_layer_set_font(&amLayer, amFont);
  text_layer_set_text_color(&amLayer, GColorWhite);
  text_layer_set_background_color(&amLayer, GColorClear);
  text_layer_set_text_alignment(&amLayer, GTextAlignmentLeft);

  // Hour layer
  text_layer_init(&hourLayer, GRect(0, 75, 144-70, 50));
  text_layer_set_font(&hourLayer, timeFont);
  text_layer_set_text_color(&hourLayer, GColorWhite);
  text_layer_set_background_color(&hourLayer, GColorClear);
  text_layer_set_text_alignment(&hourLayer, GTextAlignmentRight);

  // Separator layer
  GRect rect = layer_get_frame(&hourLayer.layer);
  rect.origin.x = rect.size.w;
  rect.size.w = 20;
  text_layer_init(&dotsLayer, rect);
  text_layer_set_text(&dotsLayer, ":");
  text_layer_set_font(&dotsLayer, timeFont);
  text_layer_set_text_color(&dotsLayer, GColorWhite);
  text_layer_set_background_color(&dotsLayer, GColorClear);
  text_layer_set_text_alignment(&dotsLayer, GTextAlignmentCenter);
  
  // Minutes layer
  rect = layer_get_frame(&dotsLayer.layer);
  rect.origin.x += rect.size.w;
  rect.size.w = 144-rect.origin.x;
  text_layer_init(&minuteLayer, rect);
  text_layer_set_font(&minuteLayer, timeFont);
  text_layer_set_text_color(&minuteLayer, GColorWhite);
  text_layer_set_background_color(&minuteLayer, GColorClear);
  text_layer_set_text_alignment(&minuteLayer, GTextAlignmentLeft);
  
  // Configure time on init
  PblTm t;
  get_time(&t);
  display_time(&t);

  layer_add_child(&window.layer, &amLayer.layer);
  layer_add_child(&window.layer, &hourLayer.layer);
  layer_add_child(&window.layer, &dotsLayer.layer);
  layer_add_child(&window.layer, &minuteLayer.layer);
}

void handle_deinit(AppContextRef ctx) {
  (void)ctx;
}

void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t) {
  (void)ctx;

  display_time(t->tick_time);
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
	.tick_info = {
      .tick_handler = &handle_minute_tick,
      .tick_units = MINUTE_UNIT
    }
  };
  app_event_loop(params, &handlers);
}
