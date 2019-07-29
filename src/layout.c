/**
 * layout.c
 *
 * Copyright (c) 2019 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "tym.h"


typedef void (*VteSetColorFunc)(VteTerminal*, const GdkRGBA*);

typedef struct {
  Config* config;
  Layout* layout;
} ContextForDraw;


static void free_context_for_draw(void* data, GClosure* closure) {
  UNUSED(closure);
  g_free(data);
}

static gboolean on_draw(GtkWidget* widget, cairo_t* cr, void* user_data)
{
  ContextForDraw* ctx = (ContextForDraw*)user_data;
  if (config_is_none(ctx->config, "color_window_background")) {
    return false;
  }
  GdkRGBA color;
  if (config_acquire_color(ctx->config, "color_window_background", &color)) {
    if (ctx->layout->alpha_supported) {
      cairo_set_source_rgba(cr, color.red, color.green, color.blue, color.alpha);
    } else {
      cairo_set_source_rgb(cr, color.red, color.green, color.blue);
    }
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);
   }
  return false;
}

Layout* layout_init()
{
  Layout* layout = g_malloc0(sizeof(Layout));
  return layout;
}

void layout_close(Layout* layout)
{
  if (layout->uri_tag) {
    g_free(layout->uri_tag);
  }
  g_free(layout);
}

void layout_build(Layout* layout, GApplication* app, Config* config)
{
  GtkWindow* window = layout->window = GTK_WINDOW(gtk_application_window_new(GTK_APPLICATION(app)));
  VteTerminal* vte = layout->vte = VTE_TERMINAL(vte_terminal_new());
  GtkBox* hbox = layout->hbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
  GtkBox* vbox = layout->vbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));

  int width = config_get_int(config, "width");
  int height = config_get_int(config, "height");
  vte_terminal_set_size(vte, width, height);

  gtk_container_add(GTK_CONTAINER(hbox), GTK_WIDGET(vte));
  gtk_container_add(GTK_CONTAINER(vbox), GTK_WIDGET(hbox));
  gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(vbox));
  gtk_container_set_border_width(GTK_CONTAINER(window), 0);

  GError* error = NULL;
  VteRegex* regex = vte_regex_new_for_match(IRI, -1, PCRE2_UTF | PCRE2_MULTILINE | PCRE2_CASELESS, &error);
  if (error) {
    g_warning("Error when parsing css: %s", error->message);
    g_error_free(error);
  } else {
    int tag = vte_terminal_match_add_regex(vte, regex, 0);
    layout->uri_tag = g_malloc0(sizeof(int));
    *layout->uri_tag = tag;
    vte_terminal_match_set_cursor_name(vte, tag, "hand");
    vte_regex_unref(regex);
  }

  GdkScreen* screen = gtk_widget_get_screen(GTK_WIDGET(window));
  GdkVisual* visual = gdk_screen_get_rgba_visual(screen);
  layout->alpha_supported = visual;
  if (!layout->alpha_supported) {
    g_message("Your screen does not support alpha channel.");
    visual = gdk_screen_get_system_visual(screen);
  }
  gtk_widget_set_visual(GTK_WIDGET(window), visual);
  ContextForDraw* ctx = g_malloc0(sizeof(ContextForDraw));
  ctx->layout = layout;
  ctx->config = config;
  g_signal_connect_data(
      G_OBJECT(window),
      "draw",
      G_CALLBACK(on_draw),
      ctx,
      (GClosureNotify)free_context_for_draw,
      0);
}

static void layout_apply_color(
  Layout* layout,
  Config* config,
  VteSetColorFunc vte_set_color_func,
  const char* key
) {
  GdkRGBA color;
  bool has_color = config_acquire_color(config, key, &color);
  if (has_color) {
    vte_set_color_func(layout->vte, &color);
  }
}

static void layout_apply_colors(Layout* layout, Config* config)
{
  GdkRGBA* palette = g_new0(GdkRGBA, 16);
  char key[10];
  for (unsigned i = 0; i < 16; i++) {
    // read color_0 .. color_15
    g_snprintf(key, 10, "color_%d", i);
    bool has_color = config_acquire_color(config, key, &palette[i]);
    if (has_color) {
      continue;
    }
    // calc default color
    palette[i].blue  = (((i & 5) ? 0xc000 : 0) + (i > 7 ? 0x3fff : 0)) / 65535.0;
    palette[i].green = (((i & 2) ? 0xc000 : 0) + (i > 7 ? 0x3fff : 0)) / 65535.0;
    palette[i].red   = (((i & 1) ? 0xc000 : 0) + (i > 7 ? 0x3fff : 0)) / 65535.0;
    palette[i].alpha = 0;
  }
  vte_terminal_set_colors(layout->vte, NULL, NULL, palette, 16);
  g_free(palette);
}

void layout_apply_theme(Layout* layout, Config* config)
{
  layout_apply_colors(layout, config);
  layout_apply_color(layout, config, vte_terminal_set_color_bold, "color_bold");
  layout_apply_color(layout, config, vte_terminal_set_color_background, "color_background");
  layout_apply_color(layout, config, vte_terminal_set_color_foreground, "color_foreground");
  layout_apply_color(layout, config, vte_terminal_set_color_cursor, "color_cursor");
  layout_apply_color(layout, config, vte_terminal_set_color_highlight, "color_highlight");
  layout_apply_color(layout, config, vte_terminal_set_color_highlight_foreground, "color_highlight_foreground");
#ifdef TYM_USE_VTE_COLOR_CURSOR_FOREGROUND
  layout_apply_color(layout, config, vte_terminal_set_color_cursor_foreground, "color_cursor_foreground");
#endif
}

void layout_apply_config(Layout* layout, Config* config)
{
  GtkWindow* window = layout->window;
  VteTerminal* vte = layout->vte;
  GtkBox* hbox = layout->hbox;
  GtkBox* vbox = layout->vbox;

  gtk_window_set_title(window, config_get_str(config, "title"));
  char* role = config_has_str(config, "role")
    ? config_get_str(config, "role")
    : NULL;
  gtk_window_set_role(window, role);
  gtk_window_set_icon_name(window, config_get_str(config, "icon"));

  int hpad = config_get_int(config, "padding_horizontal");
  int vpad = config_get_int(config, "padding_vertical");
  gtk_box_set_child_packing(hbox, GTK_WIDGET(vte), true, true, hpad, GTK_PACK_START);
  gtk_box_set_child_packing(vbox, GTK_WIDGET(hbox), true, true, vpad, GTK_PACK_START);

  vte_terminal_set_cursor_shape(vte, config_get_cursor_shape(config));
  vte_terminal_set_cursor_blink_mode(vte, config_get_cursor_blink_mode(config));
  vte_terminal_set_cjk_ambiguous_width(vte, config_get_cjk_width(config));
  vte_terminal_set_allow_bold(vte, !config_get_bool(config, "ignore_bold"));
  vte_terminal_set_mouse_autohide(vte, config_get_bool(config, "autohide"));
  vte_terminal_set_scrollback_lines(vte, config_get_int(config, "scrollback_length"));
  vte_terminal_set_audible_bell(vte, !config_get_bool(config, "silent"));
#ifdef TYM_USE_TRANSPARENT
  vte_terminal_set_clear_background(vte, !config_is_none(config, "color_background"));
#else
  if (!config_is_none(config, "color_background")) {
    g_message("`NONE` for `color_background` is supported on VTE version>=0.52 (your VTE version is %d.%d.%d)",
      VTE_MAJOR_VERSION, VTE_MINOR_VERSION, VTE_MICRO_VERSION);
  }
#endif

  if (config_has_str(config, "font")) {
    PangoFontDescription* font_desc = pango_font_description_from_string(config_get_str(config, "font"));
    vte_terminal_set_font(vte, font_desc);
    pango_font_description_free(font_desc);
  }
  gtk_widget_set_app_paintable(GTK_WIDGET(window), config_has_str(config, "color_window_background"));
}
