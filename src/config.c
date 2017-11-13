/**
 * config.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "config.h"

#define CURSOR_BLINK_MODE_SYSTEM "system"
#define CURSOR_BLINK_MODE_ON "on"
#define CURSOR_BLINK_MODE_OFF "off"
#define CJK_WIDTH_NARROW "narrow"
#define CJK_WIDTH_WIDE "wide"

typedef void (*VteSetColorFunc)(VteTerminal*, const GdkRGBA*);
typedef enum {
  VTE_CJK_WIDTH_NARROW = 1,
  VTE_CJK_WIDTH_WIDE = 2
} VteCjkWidth;

static const char* CONFIG_TABLE_NAME = "config";
static const char* FALL_BACK_SHELL = "/bin/sh";

static const int DEFAULT_WIDTH = 80;
static const int DEFAULT_HEIGHT = 22;
static const char* DEFAULT_TITLE = "tym";
static const char* DEFAULT_BLINK_MODE = CURSOR_BLINK_MODE_SYSTEM;
static const char* DEFAULT_CJK = CJK_WIDTH_NARROW;

char** str_fields; // dynamic

const char* int_fields[] = {
  "width",
  "height",
  NULL,
};

const char* bool_fields[] = {
  "use_default_keymap",
  NULL,
};


static void init_config_fields()
{
  const char* base_str_fields[] = {
    "title",
    "shell",
    "font",
    "cursor_blink_mode",
    "cjk_width",
    "color_bold",
    "color_foreground",
    "color_background",
    "color_cursor",
    "color_cursor_foreground",
    "color_highlight",
    "color_highlight_foreground",
  };
  unsigned base_count = sizeof(base_str_fields) / sizeof(char*);

  const unsigned color_count = 16;
  str_fields = (char**)g_malloc0_n(sizeof(char*), base_count + color_count + 1); // NULL terminated

  for (unsigned i = 0; i < base_count ; i++) {
    str_fields[i] = g_strdup(base_str_fields[i]);
  }

  // set `color_0` ~ `color_15` field
  char numbered_color_key[10];
  for (unsigned i = 0; i < color_count; i++) {
    g_snprintf(numbered_color_key, 10, "color_%d", i);
    str_fields[base_count + i] = g_strdup(numbered_color_key);
  }
}

static void close_config_fields()
{
  unsigned i = 0;
  while (str_fields[i]) {
    g_free(str_fields[i]);
    i++;
  }
  g_free(str_fields);
}

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
  return g_strdup(FALL_BACK_SHELL);
}


static VteCursorBlinkMode match_cursor_blink_mode(const char* str)
{
  if (0 == g_strcmp0(str, CURSOR_BLINK_MODE_ON)) {
    return VTE_CURSOR_BLINK_ON;
  }
  if (0 == g_strcmp0(str, CURSOR_BLINK_MODE_OFF)) {
    return VTE_CURSOR_BLINK_OFF;
  }
  return VTE_CURSOR_BLINK_SYSTEM;
}

static unsigned match_cjk_width(const char* str)
{
  if (0 == g_strcmp0(str, CJK_WIDTH_WIDE)) {
    return VTE_CJK_WIDTH_WIDE;
  }
  return VTE_CJK_WIDTH_NARROW;
}

Config* config_init(lua_State* lua)
{
  Config* c = g_malloc0(sizeof(Config));
  c->lua = lua;
  c->data = g_hash_table_new_full(
    g_str_hash,
    g_str_equal,
    (GDestroyNotify)g_free,
    (GDestroyNotify)g_free
  );
  config_reset(c);
  return c;
}

void config_close(Config* c)
{
  g_hash_table_destroy(c->data);
  g_free(c);
}

static void* config_get_raw(Config* c, const char* key)
{
  void* ptr = g_hash_table_lookup(c->data, key);
  if (!ptr) {
    g_warning("tried to refer null field: `%s`", key);
  }
  return g_hash_table_lookup(c->data, key);
}

static void config_set_raw(Config* c, const char* key, void* value)
{
  char* old_key = NULL;
  bool has_value = g_hash_table_lookup_extended(c->data, key, (gpointer)&old_key, NULL);
  if (has_value) {
    g_hash_table_remove(c->data, old_key);
  }
  g_hash_table_insert(c->data, g_strdup(key), value);
}

static char* config_get_str(Config* c, const char* key)
{
  return (char*)config_get_raw(c, key);
}

static void config_set_str(Config* c, const char* key, const char* value)
{
  config_set_raw(c, key, g_strdup(value));
}

static bool config_has_str(Config* c, const char* key)
{
  char* value = config_get_str(c, key);
  if (!value) {
    return false;
  }
  if (0 == g_strcmp0(value, "")) {
    return false;
  }
  return true;
}

static int config_get_int(Config* c, const char* key)
{
  return *(int*)config_get_raw(c, key);
}

static void config_set_int(Config* c, const char* key, int value)
{
  config_set_raw(c, key, g_memdup((gpointer)&value, sizeof(int)));
}

static bool config_get_bool(Config* c, const char* key)
{
  return *(bool*)config_get_raw(c, key);
}

static void config_set_bool(Config* c, const char* key, bool value)
{
  config_set_raw(c, key, g_memdup((gpointer)&value, sizeof(bool)));
}

void config_reset(Config* c)
{
  char* default_shell = get_default_shell();
  config_set_str(c, "shell", default_shell);
  g_free(default_shell);
  config_set_str(c, "title", DEFAULT_TITLE);
  config_set_str(c, "font", "");
  config_set_str(c, "cjk_width", DEFAULT_CJK);
  config_set_str(c, "cursor_blink_mode", DEFAULT_BLINK_MODE);
  unsigned i = 0;
  while (str_fields[i]) {
    const char* key = str_fields[i];
    // Set empty value if start with "color_"
    if (0 == g_ascii_strncasecmp(key, "color_", 6)) {
      config_set_str(c, key, "");
    }
    i++;
  }
  config_set_int(c, "width", DEFAULT_WIDTH);
  config_set_int(c, "height", DEFAULT_HEIGHT);

  config_set_bool(c, "use_default_keymap", true);
}

void config_prepare(Config* c)
{
  lua_State* l = c->lua;

  lua_newtable(l);

  unsigned i = 0;
  while (str_fields[i]) {
    const char* key = str_fields[i];
    const char* value = config_get_str(c, key);
    lua_pushstring(l, key);
    lua_pushstring(l, value ? value : "");
    lua_settable(l, -3);
    i++;
  }

  i = 0;
  while (int_fields[i]) {
    const char* key = int_fields[i];
    int value = config_get_int(c, key);
    lua_pushstring(l, key);
    lua_pushinteger(l, value);
    lua_settable(l, -3);
    i++;
  }

  i = 0;
  while (bool_fields[i]) {
    const char* key = bool_fields[i];
    bool value = config_get_bool(c, key);
    lua_pushstring(l, key);
    lua_pushboolean(l, value);
    lua_settable(l, -3);
    i++;
  }
  lua_setglobal(l, CONFIG_TABLE_NAME);
}

void config_load(Config* c, char** error)
{
  lua_State* l = c->lua;

  lua_getglobal(l, CONFIG_TABLE_NAME);

  if (lua_isnil(l, -1)) {
    // no error for nil
    lua_pop(l, 1);
    return;
  }

  if (!lua_istable(l, -1)) {
    *error = g_strdup_printf("`%s` is not table", CONFIG_TABLE_NAME);
    lua_pop(l, 1);
    return;
  }

  unsigned i = 0;
  while (str_fields[i]) {
    const char* key = str_fields[i];
    lua_getfield(l, -1, key);
    config_set_str(c, key, lua_tostring(l, -1));
    lua_pop(l, 1);
    i++;
  }

  i = 0;
  while (int_fields[i]) {
    const char* key = int_fields[i];
    lua_getfield(l, -1, key);
    config_set_int(c, key, lua_tointeger(l, -1));
    lua_pop(l, 1);
    i++;
  }

  i = 0;
  while (bool_fields[i]) {
    const char* key = bool_fields[i];
    lua_getfield(l, -1, key);
    config_set_bool(c, key, lua_toboolean(l, -1));
    lua_pop(l, 1);
    i++;
  }

  lua_pop(l, 1);
}

void config_apply_color(
  Config* c,
  VteTerminal* vte,
  VteSetColorFunc vte_set_color_func,
  const char* key
) {
  if (!config_has_str(c, key)) {
    return;
  }
  GdkRGBA color;
  const char* value = config_get_str(c, key);
  bool valid = gdk_rgba_parse(&color, value);
  if (valid) {
    vte_set_color_func(vte, &color);
  }
}

void config_apply_colors(Config* c, VteTerminal* vte)
{
  GdkRGBA* palette = g_new0(GdkRGBA, 16);
  char key[10];
  for (unsigned i = 0; i < 16; i++) {
    g_snprintf(key, 10, "color_%d", i);
    if (config_has_str(c, key)) {
      bool valid = gdk_rgba_parse(&palette[i], config_get_str(c, key));
      if (valid) {
        continue;
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

void config_apply(Config* c, VteTerminal* vte)
{
  GtkWindow* window = GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(vte)));

  gtk_window_set_title(window, config_get_str(c, "title"));

  vte_terminal_set_cursor_blink_mode(vte, match_cursor_blink_mode(config_get_str(c, "cursor_blink_mode")));
  vte_terminal_set_cjk_ambiguous_width(vte, match_cjk_width(config_get_str(c, "cjk_width")));

  int width = config_get_int(c, "width");
  int height = config_get_int(c, "height");
  if (0 < width && 0 < height) {
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
  }

  if (config_has_str(c, "font")) {
    PangoFontDescription* font_desc = pango_font_description_from_string(config_get_str(c, "font"));
    vte_terminal_set_font(vte, font_desc);
    pango_font_description_free(font_desc);
  }

  config_apply_colors(c, vte);
  config_apply_color(c, vte, vte_terminal_set_color_bold, "color_bold");
  config_apply_color(c, vte, vte_terminal_set_color_background, "color_background");
  config_apply_color(c, vte, vte_terminal_set_color_foreground, "color_foreground");
  config_apply_color(c, vte, vte_terminal_set_color_cursor, "color_cursor");
  config_apply_color(c, vte, vte_terminal_set_color_highlight, "color_highlight");
  config_apply_color(c, vte, vte_terminal_set_color_highlight_foreground, "color_highlight_foreground");
#if VTE_MAJOR_VERSION == 0
#if VTE_MINOR_VERSION >= 46
  /* vte_terminal_set_color_cursor_foreground is implemented since v0.46 */
  config_apply_color(c, vte, vte_terminal_set_color_cursor_foreground, "color_cursor_foreground");
#endif
#endif
}

char* config_get_shell(Config* c)
{
  return config_get_str(c, "shell");
}

bool config_get_use_default_keymap(Config* c)
{
  return config_get_bool(c, "use_default_keymap");
}

__attribute__((constructor))
static void initialization() {
  init_config_fields();
}

__attribute__((destructor))
static void finalization() {
  close_config_fields();
}
