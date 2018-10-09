/**
 * config.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "tym.h"


typedef void (*VteSetColorFunc)(VteTerminal*, const GdkRGBA*);
typedef enum {
  VTE_CJK_WIDTH_NARROW = 1,
  VTE_CJK_WIDTH_WIDE = 2
} VteCjkWidth;

#define TYM_FALL_BACK_SHELL "/bin/sh"
#define TYM_CURSOR_SHAPE_BLOCK "block"
#define TYM_CURSOR_SHAPE_IBEAM "ibeam"
#define TYM_CURSOR_SHAPE_UNDERLINE "underline"
#define TYM_CURSOR_BLINK_MODE_SYSTEM "system"
#define TYM_CURSOR_BLINK_MODE_ON "on"
#define TYM_CURSOR_BLINK_MODE_OFF "off"
#define TYM_CJK_WIDTH_NARROW "narrow"
#define TYM_CJK_WIDTH_WIDE "wide"

static const char* TYM_DEFAULT_TITLE = "tym";
static const char* TYM_DEFAULT_ICON = "terminal";
static const char* TYM_DEFAULT_TERM = "xterm-256color";
static const char* TYM_DEFAULT_CURSOR_SHAPE = TYM_CURSOR_SHAPE_BLOCK;
static const char* TYM_DEFAULT_CURSOR_BLINK_MODE = TYM_CURSOR_BLINK_MODE_SYSTEM;
static const char* TYM_DEFAULT_CJK = TYM_CJK_WIDTH_NARROW;
static const int TYM_DEFAULT_WIDTH = 80;
static const int TYM_DEFAULT_HEIGHT = 22;


static char* get_default_shell()
{
  const char* shell_env = g_getenv("SHELL");
  if (shell_env) {
    return g_strdup(shell_env);
  }
  char* user_shell = vte_get_user_shell();
  if (user_shell) {
    return user_shell;
  }
  return g_strdup(TYM_FALL_BACK_SHELL);
}

char* default_theme_path;
GList* config_fields = NULL;
unsigned config_fields_len = 0;


__attribute__((constructor))
static void initialize() {
  default_theme_path = g_build_path(
    G_DIR_SEPARATOR_S,
    g_get_user_config_dir(),
    TYM_CONFIG_DIR_NAME,
    TYM_THEME_FILE_NAME,
    NULL
  );
#define color_special(name) { ("color_" name), 0, T_STR, F_NONE, dup(""), "", ("value of color_" name ), NULL, }
#define color_normal(name) { ("color_" name), 0, T_STR, F_HIDDEN, dup(""), NULL, NULL, NULL, }
  const ConfigType T_STR = CONFIG_TYPE_STRING;
  const ConfigType T_INT = CONFIG_TYPE_INTEGER;
  const ConfigType T_BOOL = CONFIG_TYPE_BOOLEAN;
  const ConfigType T_NONE = CONFIG_TYPE_NONE;
  const GOptionFlags F_NONE = G_OPTION_FLAG_NONE;
  const GOptionFlags F_HIDDEN = G_OPTION_FLAG_HIDDEN;

  char* (*dup)(const char*) = g_strdup;
  const bool default_false = false;

  // name, short, type, group, flag, default, desc
  ConfigField c[] = {
    { "theme",               't',  T_STR, F_NONE, dup(default_theme_path), "<path>", "<path> to theme file. Set '" TYM_SYMBOL_NONE "' to start without loading theme.", NULL, },
    { "shell",               'e',  T_STR, F_NONE, get_default_shell(), "<shell path>", "Shell to be used", NULL, },
    { "title",                 0,  T_STR, F_NONE, dup(TYM_DEFAULT_TITLE), "", "Window title", NULL, },
    { "font",                  0,  T_STR, F_NONE, dup(""), "", "Font to render(e.g. 'Ubuntu Mono 12')", NULL, },
    { "icon",                  0,  T_STR, F_NONE, dup(TYM_DEFAULT_ICON), "", "Name of icon", NULL, },
    { "cursor_shape",          0,  T_STR, F_NONE, dup(TYM_DEFAULT_CURSOR_SHAPE), "", "'block', 'ibeam' or 'underline'", NULL, },
    { "cursor_blink_mode",     0,  T_STR, F_NONE, dup(TYM_DEFAULT_CURSOR_BLINK_MODE), "", "'system', 'on' or 'off'", NULL, },
    { "term",                  0,  T_STR, F_NONE, dup(TYM_DEFAULT_TERM), "$TERM", "Value to override $TERM", NULL, },
    { "role",                  0,  T_STR, F_NONE, dup(""), "", "Unique identifier for the window", NULL, },
    { "cjk_width",             0,  T_STR, F_NONE, dup(TYM_DEFAULT_CJK), "", "'narrow' or 'wide'", NULL, },
    { "width",                 0,  T_INT, F_NONE, g_memdup(&TYM_DEFAULT_WIDTH, sizeof(int)), "<int>", "Initial columns", NULL, },
    { "height",                0,  T_INT, F_NONE, g_memdup(&TYM_DEFAULT_HEIGHT, sizeof(int)), "<int>", "Initial rows", NULL, },
    { "ignore_default_keymap", 0, T_BOOL, F_NONE, g_memdup(&default_false, sizeof(bool)), NULL, "Whether to use default keymap", NULL, },
    { "ignore_bold"          , 0, T_BOOL, F_NONE, g_memdup(&default_false, sizeof(bool)), NULL, "Whether to attempt to draw bold text", NULL, },
    { "autohide"             , 0, T_BOOL, F_NONE, g_memdup(&default_false, sizeof(bool)), NULL, "Whether to hide mouse cursor when the user presses a key", NULL, },
    color_special("foreground"),
    color_special("background"),
    color_special("cursor"),
    color_special("cursor_foreground"),
    color_special("highlight"),
    color_special("highlight_foreground"),
    color_special("bold"),
    color_normal("0"),  color_normal("1"),  color_normal("2"),  color_normal("3"),
    color_normal("4"),  color_normal("5"),  color_normal("6"),  color_normal("7"),
    color_normal("8"),  color_normal("9"),  color_normal("10"), color_normal("11"),
    color_normal("12"), color_normal("13"), color_normal("14"), color_normal("15"),
    { "color_1..15", 0, T_NONE, F_NONE, NULL, "", "value of color from color_1 to color_15", NULL, },
  };
#undef color_special
#undef color_normal


  unsigned i = 0;
  config_fields_len = sizeof(c) / sizeof(ConfigField);
  while (i < config_fields_len) {
    config_fields = g_list_append(config_fields, g_memdup(&c[i], sizeof(c[i])));
    i++;
  }
}

static void free_data(void* data, void* user_data) {
  UNUSED(user_data);
  g_free(data);
}

__attribute__((destructor))
static void finalize() {
  g_free(default_theme_path);
  g_list_foreach(config_fields, free_data, NULL);
  g_list_free(config_fields);
}

ConfigField* get_config_field(const char* key) {
  for (GList* li = config_fields; li != NULL; li = li->next) {
    ConfigField* field = (ConfigField*)li->data;
    if (0 == g_strcmp0(field->name, key)) {
      ConfigType t= field->type;
      if (t == CONFIG_TYPE_STRING || t == CONFIG_TYPE_INTEGER || t == CONFIG_TYPE_BOOLEAN) {
        return field;
      }
    }
  }
  return NULL;
}

static VteCursorShape match_cursor_shape(const char* str)
{
  if (0 == g_strcmp0(str, TYM_CURSOR_SHAPE_IBEAM)) {
    return VTE_CURSOR_SHAPE_IBEAM;
  }
  if (0 == g_strcmp0(str, TYM_CURSOR_SHAPE_UNDERLINE)) {
    return VTE_CURSOR_SHAPE_UNDERLINE;
  }
  return VTE_CURSOR_SHAPE_BLOCK;
}

static VteCursorBlinkMode match_cursor_blink_mode(const char* str)
{
  if (0 == g_strcmp0(str, TYM_CURSOR_BLINK_MODE_ON)) {
    return VTE_CURSOR_BLINK_ON;
  }
  if (0 == g_strcmp0(str, TYM_CURSOR_BLINK_MODE_OFF)) {
    return VTE_CURSOR_BLINK_OFF;
  }
  return VTE_CURSOR_BLINK_SYSTEM;
}

static unsigned match_cjk_width(const char* str)
{
  if (0 == g_strcmp0(str, TYM_CJK_WIDTH_WIDE)) {
    return VTE_CJK_WIDTH_WIDE;
  }
  return VTE_CJK_WIDTH_NARROW;
}

Config* config_init()
{
  Config* config = g_malloc0(sizeof(Config));
  config->data = g_hash_table_new_full(
    g_str_hash,
    g_str_equal,
    (GDestroyNotify)g_free,
    (GDestroyNotify)g_free
  );
  config_reset(config);
  return config;
}

void config_close(Config* config)
{
  g_hash_table_destroy(config->data);
  g_free(config);
}

static void* config_get_raw(Config* config, const char* key)
{
  void* ptr = g_hash_table_lookup(config->data, key);
  if (!ptr) {
    dd("tried to refer null field: '%s'", key);
  }
  return ptr;
}

static void config_add_raw(Config* config, const char* key, void* value)
{
  void* ptr = g_hash_table_lookup(config->data, key);
  if (ptr) {
    dd("tried to overwrite field: '%s'", key);
    return;
  }
  g_hash_table_insert(config->data, g_strdup(key), value);
}

static bool config_set_raw(Config* config, const char* key, void* value)
{
  if (!value) {
    dd("tried to set null field: '%s'", key);
    return false;
  }
  void* old_key = NULL;
  bool has_value = g_hash_table_lookup_extended(config->data, key, &old_key, NULL);
  if (has_value) {
    g_hash_table_remove(config->data, old_key);
  } else {
    dd("tried to add new field: '%s'", key);
    return false;
  }
  g_hash_table_insert(config->data, g_strdup(key), value);
  return true;
}

bool config_has_str(Config* config, const char* key)
{
  char* value = (char*)config_get_raw(config, key);
  if (!value) {
    return false;
  }
  if (0 == g_strcmp0(value, "")) {
    return false;
  }
  return true;
}

static void config_add_str(Config* config, const char* key, const char* value)
{
  config_add_raw(config, key, g_strdup(value));
}

bool config_set_str(Config* config, const char* key, const char* value)
{
  return config_set_raw(config, key, g_strdup(value));
}

char* config_get_str(Config* config, const char* key)
{
  char* v = (char*)config_get_raw(config, key);
  if (v) {
    return v;
  }
  dd("string config of '%s' is null. falling back to \"\"", key);
  return "";
}

int config_get_int(Config* config, const char* key)
{
  int* v = (int*)config_get_raw(config, key);
  if (v) {
    return *v;
  }
  dd("int config of '%s' is null. falling back to 0", key);
  return 0;
}

static void config_add_int(Config* config, const char* key, int value)
{
  config_add_raw(config, key, g_memdup((gpointer)&value, sizeof(int)));
}

bool config_set_int(Config* config, const char* key, int value)
{
  return config_set_raw(config, key, g_memdup((gpointer)&value, sizeof(int)));
}

bool config_get_bool(Config* config, const char* key)
{
  bool* v = (bool*)config_get_raw(config, key);
  if (v) {
    return *v;
  }
  dd("bool config of '%s' is null. falling back to null", key);
  return false;
}

static void config_add_bool(Config* config, const char* key, bool value)
{
  config_add_raw(config, key, g_memdup((gpointer)&value, sizeof(bool)));
}

bool config_set_bool(Config* config, const char* key, bool value)
{
  return config_set_raw(config, key, g_memdup((gpointer)&value, sizeof(bool)));
}

void config_reset(Config* config)
{
  dd("reset");
  g_hash_table_remove_all(config->data);

  for (GList* li = config_fields; li != NULL; li = li->next) {
    ConfigField* field = (ConfigField*)li->data;
    switch (field->type) {
      case CONFIG_TYPE_STRING:
        config_add_str(config, field->name, field->default_value);
        break;
      case CONFIG_TYPE_INTEGER:
        config_add_int(config, field->name, *(int*)(field->default_value));
        break;
      case CONFIG_TYPE_BOOLEAN:
        config_add_bool(config, field->name, *(bool*)(field->default_value));
        break;
      case CONFIG_TYPE_NONE:
        break;
    }
  }
}

void config_load_option_values(Config* config, Option* option)
{
  UNUSED(option);

  for (GList* li = config_fields; li != NULL; li = li->next) {
    ConfigField* field = (ConfigField*)li->data;
    char* key = field->name;
    if (!field->option_data) {
      continue;
    }
    switch (field->type) {
      case CONFIG_TYPE_STRING:
        config_set_str(config, key, (char*)field->option_data);
        break;
      case CONFIG_TYPE_INTEGER:
        config_set_int(config, key, (uintptr_t)field->option_data);
        break;
      case CONFIG_TYPE_BOOLEAN: {
        config_set_bool(config, key, (bool)field->option_data);
        break;
      }
      case CONFIG_TYPE_NONE:
        break;
    }
  }
}

static void config_apply_color(
  Config* config,
  VteTerminal* vte,
  VteSetColorFunc vte_set_color_func,
  const char* key
) {
  if (!config_has_str(config, key)) {
    return;
  }
  GdkRGBA color;
  const char* value = config_get_str(config, key);
  bool valid = gdk_rgba_parse(&color, value);
  if (valid) {
    vte_set_color_func(vte, &color);
  } else {
    g_message( "Invalid color string for '%s': %s", key, value);
  }
}

static void config_apply_colors(Config* config, VteTerminal* vte)
{
  GdkRGBA* palette = g_new0(GdkRGBA, 16);
  char key[10];
  for (unsigned i = 0; i < 16; i++) {
    // read color_0 .. color_15
    g_snprintf(key, 10, "color_%d", i);
    char* value = config_get_str(config, key);
    if (config_has_str(config, key)) {
      bool valid = gdk_rgba_parse(&palette[i], value);
      if (valid) {
        continue;
      } else {
        g_message( "Invalid color string for '%s': %s", key, value);
      }
    }
    // calc default color
    palette[i].blue  = (((i & 5) ? 0xc000 : 0) + (i > 7 ? 0x3fff : 0)) / 65535.0;
    palette[i].green = (((i & 2) ? 0xc000 : 0) + (i > 7 ? 0x3fff : 0)) / 65535.0;
    palette[i].red   = (((i & 1) ? 0xc000 : 0) + (i > 7 ? 0x3fff : 0)) / 65535.0;
    palette[i].alpha = 0;
  }
  vte_terminal_set_colors(vte, NULL, NULL, palette, 16);
}

void config_apply_theme(Config* config, VteTerminal* vte)
{
  config_apply_colors(config, vte);
  config_apply_color(config, vte, vte_terminal_set_color_bold, "color_bold");
  config_apply_color(config, vte, vte_terminal_set_color_background, "color_background");
  config_apply_color(config, vte, vte_terminal_set_color_foreground, "color_foreground");
  config_apply_color(config, vte, vte_terminal_set_color_cursor, "color_cursor");
  config_apply_color(config, vte, vte_terminal_set_color_highlight, "color_highlight");
  config_apply_color(config, vte, vte_terminal_set_color_highlight_foreground, "color_highlight_foreground");

#ifdef TYM_USE_VTE_COLOR_CURSOR_FOREGROUND
  config_apply_color(config, vte, vte_terminal_set_color_cursor_foreground, "color_cursor_foreground");
#endif
}

void config_apply(Config* config, VteTerminal* vte)
{
  GtkWindow* window = GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(vte)));

  gtk_window_set_title(window, config_get_str(config, "title"));
  char* role = config_has_str(config, "role")
    ? config_get_str(config, "role")
    : NULL;
  gtk_window_set_role(window, role);
  gtk_window_set_icon_name(window, config_get_str(config, "icon"));

  vte_terminal_set_cursor_shape(vte, match_cursor_shape(config_get_str(config, "cursor_shape")));
  vte_terminal_set_cursor_blink_mode(vte, match_cursor_blink_mode(config_get_str(config, "cursor_blink_mode")));
  vte_terminal_set_cjk_ambiguous_width(vte, match_cjk_width(config_get_str(config, "cjk_width")));
  vte_terminal_set_allow_bold(vte, !config_get_bool(config, "ignore_bold"));
  vte_terminal_set_mouse_autohide(vte, config_get_bool(config, "autohide"));

  int width = config_get_int(config, "width");
  int height = config_get_int(config, "height");
  if (0 < width && 0 < height) {
    if (gtk_window_is_active(window)){
      GtkBorder border;
      gtk_style_context_get_padding(
        gtk_widget_get_style_context(GTK_WIDGET(vte)),
        gtk_widget_get_state_flags(GTK_WIDGET(vte)),
        &border
      );
      const int char_width = vte_terminal_get_char_width(vte);
      const int char_height = vte_terminal_get_char_height(vte);
      gtk_window_resize(
        window,
        width * char_width + border.left + border.right,
        height * char_height + border.top + border.bottom
      );
    } else {
      vte_terminal_set_size(vte, width, height);
    }
  }

  if (config_has_str(config, "font")) {
    PangoFontDescription* font_desc = pango_font_description_from_string(config_get_str(config, "font"));
    vte_terminal_set_font(vte, font_desc);
    pango_font_description_free(font_desc);
  }
}

char* config_acquire_theme_path(Config* config)
{
  if (!config_has_str(config, "theme")) {
    return g_strdup(default_theme_path);
  }

  char* path = config_get_str(config, "theme");

  if (0 == g_strcmp0(path, TYM_SYMBOL_NONE)) {
    return NULL;
  }

  if (g_path_is_absolute(path)) {
    return g_strdup(path);
  }

  char* cwd = g_get_current_dir();
  char* p = g_build_path(G_DIR_SEPARATOR_S, cwd, path, NULL);
  g_free(cwd);
  return p;
}
