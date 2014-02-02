#include "pebble.h"

static Window *window;
static TextLayer *amLayer;
static TextLayer *hourLayer;
static TextLayer *dotsLayer;
static TextLayer *minuteLayer;

static int hour = -1;

void display_time(struct tm *t)
{
  static char timeText[] = "00";
  static char hourText[] = "00";  
  static char amText[] = "am";
  static char pmText[] = "pm";

  // Always update the minute
  strftime(timeText, sizeof(timeText), "%M", t);
  text_layer_set_text(minuteLayer, timeText);

  // Update the hour only if it's changed
  if (hour != t->tm_hour) {
		hour = t->tm_hour;

		char *format = clock_is_24h_style() ? "%k" : "%l";
		strftime(hourText, sizeof(hourText), format, t);
		text_layer_set_text(hourLayer, hourText);	
  }

  // Update am/pm if it's not the 24h style set in settings
  if (!clock_is_24h_style()) {
		char *value = amText;
		if (t->tm_hour >= 12) {
			value = pmText;
		}
	
		const char *currentText = text_layer_get_text(amLayer);
		if (memcmp(currentText, value, 2*sizeof(char)) != 0) {
			// Update text layer
			text_layer_set_text(amLayer, value);
		}
  }
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
	display_time(tick_time);
}

static void init() {

  window = window_create();
  window_stack_push(window, true);
  window_set_background_color(window, GColorBlack);
  
  // Custom fonts
  GFont timeFont = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TEXT_TIME_30));
  GFont amFont = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TEXT_AM_18));
	
  //  AM / PM Layer
  amLayer = text_layer_create(GRect(10, 60, 144-20, 24));
  text_layer_set_text(amLayer, "");
  text_layer_set_font(amLayer, amFont);
  text_layer_set_text_color(amLayer, GColorWhite);
  text_layer_set_background_color(amLayer, GColorClear);
  text_layer_set_text_alignment(amLayer, GTextAlignmentLeft);

  // Hour layer
  hourLayer = text_layer_create(GRect(0, 75, 144-70, 50));
  text_layer_set_font(hourLayer, timeFont);
  text_layer_set_text_color(hourLayer, GColorWhite);
  text_layer_set_background_color(hourLayer, GColorClear);
  text_layer_set_text_alignment(hourLayer, GTextAlignmentRight);

  // Separator layer
  GRect rect = layer_get_frame(text_layer_get_layer(hourLayer));
  rect.origin.x = rect.size.w;
  rect.size.w = 20;
  dotsLayer = text_layer_create(rect);
  text_layer_set_text(dotsLayer, ":");
  text_layer_set_font(dotsLayer, timeFont);
  text_layer_set_text_color(dotsLayer, GColorWhite);
  text_layer_set_background_color(dotsLayer, GColorClear);
  text_layer_set_text_alignment(dotsLayer, GTextAlignmentCenter);
  
  // Minutes layer
  rect = layer_get_frame(text_layer_get_layer(dotsLayer));
  rect.origin.x += rect.size.w;
  rect.size.w = 144-rect.origin.x;
  minuteLayer = text_layer_create(rect);
  text_layer_set_font(minuteLayer, timeFont);
  text_layer_set_text_color(minuteLayer, GColorWhite);
  text_layer_set_background_color(minuteLayer, GColorClear);
  text_layer_set_text_alignment(minuteLayer, GTextAlignmentLeft);
  
  // Configure time on init
	// Configure time on init
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
	display_time(t);

	// Load layers
	Layer *window_layer = window_get_root_layer(window);
  layer_add_child(window_layer, text_layer_get_layer(amLayer));
  layer_add_child(window_layer, text_layer_get_layer(hourLayer));
  layer_add_child(window_layer, text_layer_get_layer(dotsLayer));
  layer_add_child(window_layer, text_layer_get_layer(minuteLayer));
	
	// Handle minute tick
	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

static void deinit() {
	tick_timer_service_unsubscribe();
	window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
