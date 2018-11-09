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


Layout* layout_init()
{
  dd("init");
  Layout* layout = g_malloc0(sizeof(Layout));
  return layout;
}

void layout_close(Layout* layout)
{
  g_free(layout);
}

void layout_build(Layout* layout, GtkApplication* app, Config* config)
{
  GtkWindow* window = layout->window = GTK_WINDOW(gtk_application_window_new(app));
  VteTerminal* vte = layout->vte = VTE_TERMINAL(vte_terminal_new());

  int width = config_get_int(config, "width");
  int height = config_get_int(config, "height");
  vte_terminal_set_size(vte, width, height);

  gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(vte));
  gtk_container_set_border_width(GTK_CONTAINER(window), 0);
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

  gtk_window_set_title(window, config_get_str(config, "title"));
  char* role = config_has_str(config, "role")
    ? config_get_str(config, "role")
    : NULL;
  gtk_window_set_role(window, role);
  gtk_window_set_icon_name(window, config_get_str(config, "icon"));

  vte_terminal_set_cursor_shape(vte, config_get_cursor_shape(config));
  vte_terminal_set_cursor_blink_mode(vte, config_get_cursor_blink_mode(config));
  vte_terminal_set_cjk_ambiguous_width(vte, config_get_cjk_width(config));
  vte_terminal_set_allow_bold(vte, !config_get_bool(config, "ignore_bold"));
  vte_terminal_set_mouse_autohide(vte, config_get_bool(config, "autohide"));

  if (config_has_str(config, "font")) {
    PangoFontDescription* font_desc = pango_font_description_from_string(config_get_str(config, "font"));
    vte_terminal_set_font(vte, font_desc);
    pango_font_description_free(font_desc);
  }
}
