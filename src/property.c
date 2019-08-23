/**
 * property.c
 *
 * Copyright (c) 2019 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "property.h"


typedef enum {
  VTE_CJK_WIDTH_NARROW = 1,
  VTE_CJK_WIDTH_WIDE = 2
} VteCjkWidth;

typedef void (*VteSetColorFunc)(VteTerminal*, const GdkRGBA*);


static void set_size(Context* context, int width, int height)
{
  GtkWindow* window = context->layout->window;
  VteTerminal* vte = context->layout->vte;
  bool visible = gtk_widget_is_visible(GTK_WIDGET(window));
  if (visible) {
    GtkBorder border;
    gtk_style_context_get_padding(
      gtk_widget_get_style_context(GTK_WIDGET(vte)),
      gtk_widget_get_state_flags(GTK_WIDGET(vte)),
      &border
    );
    const int char_width = vte_terminal_get_char_width(vte);
    const int char_height = vte_terminal_get_char_height(vte);
    const int hpad = context_get_int(context, "padding_horizontal");
    const int vpad = context_get_int(context, "padding_vertical");
    gtk_window_resize(
      context->layout->window,
      width * char_width + border.left + border.right + hpad * 2,
      height * char_height + border.top + border.bottom + vpad * 2
    );
  } else {
    vte_terminal_set_size(vte, width, height);
  }
}


// STR

void setter_shell(Context* context, const char* key, const char* value)
{
  if (!is_equal(context_get_str(context, key), value) && !context->state.initializing) {
    g_message("To override `%s`, you need to set value before terminal finish initialization.`", key);
    return;
  }
  config_set_str(context->config, key, value);
}

void setter_term(Context* context, const char* key, const char* value)
{
  if (!is_equal(context_get_str(context, key), value) && !context->state.initializing) {
    g_message("To override `%s`, you need to set value before the terminal finish initialization.`", key);
    return;
  }
  config_set_str(context->config, key, value);
}

const char* getter_title(Context* context, const char* key)
{
  return gtk_window_get_title(context->layout->window);
}

void setter_title(Context* context, const char* key, const char* value)
{
  gtk_window_set_title(context->layout->window, value);
}

void setter_font(Context* context, const char* key, const char* value)
{
  PangoFontDescription* font_desc = pango_font_description_from_string(value);
  vte_terminal_set_font(context->layout->vte, font_desc);
  pango_font_description_free(font_desc);
  config_set_str(context->config, key, value);
}

const char* getter_icon(Context* context, const char* key)
{
  return gtk_window_get_icon_name(context->layout->window);
}

void setter_icon(Context* context, const char* key, const char* value)
{
  gtk_window_set_icon_name(context->layout->window, value);
}

const char* getter_role(Context* context, const char* key)
{
  const char* role = gtk_window_get_role(context->layout->window);
  return role ? role : "";
}

void setter_role(Context* context, const char* key, const char* value)
{
  gtk_window_set_role(context->layout->window, is_none(value) ? NULL : value);
}

const char* getter_cursor_shape(Context* context, const char* key)
{
  VteCursorShape cursor_shape = vte_terminal_get_cursor_shape(context->layout->vte);
  switch (cursor_shape) {
    case VTE_CURSOR_SHAPE_IBEAM:
      return TYM_CURSOR_SHAPE_IBEAM;
    case VTE_CURSOR_SHAPE_UNDERLINE:
      return TYM_CURSOR_SHAPE_UNDERLINE;
    case VTE_CURSOR_SHAPE_BLOCK:
      return TYM_CURSOR_SHAPE_BLOCK;
    default:
      dw("Invalid cursor shape `%d` is detected.", cursor_shape);
      return TYM_CURSOR_SHAPE_BLOCK;
  }
}

void setter_cursor_shape(Context* context, const char* key, const char* value)
{
  VteCursorShape cursor_shape = VTE_CURSOR_SHAPE_BLOCK;
  if (g_strcmp0(value, TYM_CURSOR_SHAPE_BLOCK) == 0) {
  } else if (g_strcmp0(value, TYM_CURSOR_SHAPE_UNDERLINE) == 0) {
    cursor_shape = VTE_CURSOR_SHAPE_UNDERLINE;
  } else if (g_strcmp0(value, TYM_CURSOR_SHAPE_IBEAM) == 0)  {
    cursor_shape = VTE_CURSOR_SHAPE_IBEAM;
  } else {
    g_message("Invalid `cursor_shape` value. (`%s` is provided). '" \
        TYM_CURSOR_SHAPE_BLOCK "', '" TYM_CURSOR_SHAPE_IBEAM "' or '" \
        TYM_CURSOR_SHAPE_UNDERLINE "' is available.", value);
    return;
  }
  vte_terminal_set_cursor_shape(context->layout->vte, cursor_shape);
}

const char* getter_cursor_blink_mode(Context* context, const char* key)
{
  VteCursorBlinkMode mode = vte_terminal_get_cursor_blink_mode(context->layout->vte);
  switch (mode) {
    case VTE_CURSOR_BLINK_SYSTEM:
      return TYM_CURSOR_BLINK_MODE_SYSTEM;
    case VTE_CURSOR_BLINK_ON:
      return TYM_CURSOR_BLINK_MODE_ON;
    case VTE_CURSOR_BLINK_OFF:
      return TYM_CURSOR_BLINK_MODE_OFF;
    default:
      dw("Invalid cursor blink `%d` is detected.", mode);
      return TYM_CURSOR_BLINK_MODE_SYSTEM;
  }
}

void setter_cursor_blink_mode(Context* context, const char* key, const char* value)
{
  VteCursorBlinkMode mode = VTE_CURSOR_BLINK_SYSTEM;
  if (g_strcmp0(value, TYM_CURSOR_BLINK_MODE_SYSTEM) == 0) {
  } else if (g_strcmp0(value, TYM_CURSOR_BLINK_MODE_ON) == 0) {
    mode = VTE_CURSOR_BLINK_ON;
  } else if (g_strcmp0(value, TYM_CURSOR_BLINK_MODE_OFF) == 0)  {
    mode = VTE_CURSOR_BLINK_OFF;
  } else {
    g_message("Invalid `cursor_blink_mode` value. (`%s` is provided). '" \
        TYM_CURSOR_BLINK_MODE_SYSTEM "', '" TYM_CURSOR_BLINK_MODE_ON "' or '" \
        TYM_CURSOR_BLINK_MODE_OFF "' is available.", value);
    return;
  }
  vte_terminal_set_cursor_blink_mode(context->layout->vte, mode);
}

const char* getter_cjk_width(Context* context, const char* key)
{
  VteCjkWidth cjk = vte_terminal_get_cjk_ambiguous_width(context->layout->vte);
  switch (cjk) {
    case VTE_CJK_WIDTH_NARROW:
      return TYM_CJK_WIDTH_NARROW;
    case VTE_CJK_WIDTH_WIDE:
      return TYM_CJK_WIDTH_WIDE;
    default:
      dw("Invalid `cjk_width` `%d` is detected.", cjk);
      return TYM_CJK_WIDTH_NARROW;
  }
}

void setter_cjk_width(Context* context, const char* key, const char* value)
{
  VteCjkWidth cjk = VTE_CJK_WIDTH_NARROW;
  if (g_strcmp0(value, TYM_CJK_WIDTH_NARROW) == 0) {
  } else if (g_strcmp0(value, TYM_CURSOR_BLINK_MODE_ON) == 0) {
    cjk = VTE_CJK_WIDTH_WIDE;
  } else {
    g_message("Invalid `cjk_width` value. (`%s` is provided). '" \
        TYM_CJK_WIDTH_NARROW "' or '" TYM_CJK_WIDTH_WIDE "' is available.", value);
    return;
  }
  vte_terminal_set_cjk_ambiguous_width(context->layout->vte, cjk);
}

void setter_background_image(Context* context, const char* key, const char* value)
{
  char* css;
  if (is_empty(value)) {
    css = g_strdup("window { background-image: none; }");
  } else {
    char* path;
    if (g_path_is_absolute(value)) {
      path = g_strdup(value);
    } else {
      char* cwd = g_get_current_dir();
      path = g_build_path(G_DIR_SEPARATOR_S, cwd, value, NULL);
      g_free(cwd);
    }
    if (!g_file_test(path, G_FILE_TEST_EXISTS)) {
      g_message("`%s`: `%s` does not exist.", key, path);
      g_free(path);
      return;
    }
    css = g_strdup_printf("window { background-image: url('%s'); background-size: cover; background-position: center; }", path);
    g_free(path);
  }
  GtkCssProvider* css_provider = gtk_css_provider_new();
  GError* error = NULL;
  gtk_css_provider_load_from_data(css_provider, css, -1, &error);
  g_free(css);
  if (error) {
    g_message("`%s`: Error in css: %s", key, error->message);
    g_error_free(error);
    return;
  }
  GtkStyleContext* style_context = gtk_widget_get_style_context(GTK_WIDGET(context->layout->window));
  gtk_style_context_add_provider(style_context, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
  config_set_str(context->config, key, value);
}

// INT

int getter_width(Context* context, const char* key)
{
  return vte_terminal_get_column_count(context->layout->vte);
}

void setter_width(Context* context, const char* key, int value)
{
  set_size(context, value, context_get_int(context, "height"));
}

int getter_height(Context* context, const char* key)
{
  return vte_terminal_get_row_count(context->layout->vte);
}

void setter_height(Context* context, const char* key, int value)
{
  set_size(context, context_get_int(context, "width"), value);
}

int getter_scale(Context* context, const char* key)
{
  return roundup(vte_terminal_get_font_scale(context->layout->vte) * 100);
}

void setter_scale(Context* context, const char* key, int value)
{
  vte_terminal_set_font_scale(context->layout->vte, (double)value / 100);
}

void setter_padding_horizontal(Context* context, const char* key, int value)
{
  gtk_box_set_child_packing(context->layout->hbox, GTK_WIDGET(context->layout->vte), true, true, value, GTK_PACK_START);
  config_set_int(context->config, key, value);
}

void setter_padding_vertical(Context* context, const char* key, int value)
{
  gtk_box_set_child_packing(context->layout->vbox, GTK_WIDGET(context->layout->hbox), true, true, value, GTK_PACK_START);
  config_set_int(context->config, key, value);
}

int getter_scrollback_length(Context* context, const char* key)
{
  return vte_terminal_get_scrollback_lines(context->layout->vte);
}

void setter_scrollback_length(Context* context, const char* key, int value)
{
  vte_terminal_set_scrollback_lines(context->layout->vte, value);
}


// BOOL

bool getter_silent(Context* context, const char* key)
{
  return !vte_terminal_get_audible_bell(context->layout->vte);
}

void setter_silent(Context* context, const char* key, bool value)
{
  vte_terminal_set_audible_bell(context->layout->vte, !value);
}

bool getter_ignore_bold(Context* context, const char* key)
{
  return !vte_terminal_get_allow_bold(context->layout->vte);
}

void setter_ignore_bold(Context* context, const char* key, bool value)
{
  vte_terminal_set_allow_bold(context->layout->vte, !value);
}

bool getter_autohide(Context* context, const char* key)
{
  return vte_terminal_get_mouse_autohide(context->layout->vte);
}

void setter_autohide(Context* context, const char* key, bool value)
{
  vte_terminal_set_mouse_autohide(context->layout->vte, value);
}

// COLOR
static void setter_color_special(Context* context, const char* key, const char* value, VteSetColorFunc color_func)
{
  GdkRGBA color = {};
  bool valid = gdk_rgba_parse(&color, value);
  if (!valid) {
    g_message("Invalid color string for '%s': %s", key, value);
    return;
  }
  color_func(context->layout->vte, &color);
  config_set_str(context->config, key, value);
}

void setter_color_normal(Context* context, const char* key, const char* value)
{
  assert(value);
  if (is_equal(value, context_get_str(context, key))) {
    dd("skip set `%s`(value `%s` is not changed)", key, value);
    return;
  }
  char* target = NULL;
  int index = g_ascii_strtoull(&key[6], &target, 10);
  GdkRGBA* palette = g_new0(GdkRGBA, 16);
  assert(&key[6] != target || index < 0 || index > 15);
  char s[10] = {};
  unsigned i = 0;
  while (i < 16) {
    const char* v;
    if (i == index) {
      v = value;
    } else {
      g_snprintf(s, 10, "color_%d", i);
      v = context_get_str(context, s);
    }
    bool valid = gdk_rgba_parse(&palette[i], v);
    if (!valid) {
      g_message("Invalid color string for '%s': %s", key, value);
      return;
    }
    i += 1;
  }
  vte_terminal_set_colors(context->layout->vte, NULL, NULL, palette, 16);
  config_set_str(context->config, key, value);
  g_free(palette);
}

void setter_color_window_background(Context* context, const char* key, const char* value)
{
  if (is_empty(value)) {
    gtk_widget_set_app_paintable(GTK_WIDGET(context->layout->window), false);
    config_set_str(context->config, key, value);
    return;
  }

  if (!is_none(value)) {
    GdkRGBA color = {};
    bool valid = gdk_rgba_parse(&color, value);
    if (!valid) {
      g_message("Invalid color string for '%s': %s", key, value);
      return;
    }
  } else {
    gtk_widget_queue_draw(GTK_WIDGET(context->layout->window));
  }
  gtk_widget_set_app_paintable(GTK_WIDGET(context->layout->window), true);
  config_set_str(context->config, key, value);
}

void setter_color_background(Context* context, const char* key, const char* value)
{
  if (is_none(value)) {
#ifdef TYM_USE_TRANSPARENT
    vte_terminal_set_clear_background(context->layout->vte, false);
    config_set_str(context->config, key, value);
#else
    g_message("`NONE` for `color_background` is supported on VTE version>=0.52 (your VTE version is %s)", TYM_VTE_VERSION);
#endif
    return;
  }
  vte_terminal_set_clear_background(context->layout->vte, true);

  setter_color_special(context, key, value, vte_terminal_set_color_background);
}

void setter_color_foreground(Context* context, const char* key, const char* value)
{
  setter_color_special(context, key, value, vte_terminal_set_color_foreground);
}

void setter_color_bold(Context* context, const char* key, const char* value)
{
  setter_color_special(context, key, value, vte_terminal_set_color_bold);
}

void setter_color_cursor(Context* context, const char* key, const char* value)
{
  setter_color_special(context, key, value, vte_terminal_set_color_cursor);
}

void setter_color_cursor_foreground(Context* context, const char* key, const char* value)
{
#ifdef TYM_USE_VTE_COLOR_CURSOR_FOREGROUND
  setter_color_special(context, key, value, vte_terminal_set_color_cursor_foreground);
#else
  g_message("`%s` is supported on VTE version>=0.46 (your VTE version is %s)", key, TYM_VTE_VERSION);
#endif
}

void setter_color_highlight(Context* context, const char* key, const char* value)
{
  setter_color_special(context, key, value, vte_terminal_set_color_highlight);
}

void setter_color_highlight_foreground(Context* context, const char* key, const char* value)
{
  setter_color_special(context, key, value, vte_terminal_set_color_highlight_foreground);
}
