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


const char* fields_str[31] = {
  "title",
  "shell",
  "font",
  "icon",
  "cursor_blink_mode",
  "cjk_width",
  "role",
  "term",
  "color_bold",
  "color_foreground",
  "color_background",
  "color_cursor",
  "color_cursor_foreground",
  "color_highlight",
  "color_highlight_foreground",
  "color_0",
  "color_1",
  "color_2",
  "color_3",
  "color_4",
  "color_5",
  "color_6",
  "color_7",
  "color_8",
  "color_9",
  "color_10",
  "color_11",
  "color_12",
  "color_13",
  "color_14",
  "color_15",
};

const char* fields_int[2] = {
  "width",
  "height",
};

const char* fields_bool[3] = {
  "use_default_keymap",
  "allow_bold_font",
  "no_quit",
};

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
static const char* DEFAULT_ICON = "terminal";
static const char* DEFAULT_TERM = "xterm-256color";
static const char* DEFAULT_BLINK_MODE = CURSOR_BLINK_MODE_SYSTEM;
static const char* DEFAULT_CJK = CJK_WIDTH_NARROW;


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
    g_warning("Tried to refer null field: `%s`", key);
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
  config_set_str(c, "icon", DEFAULT_ICON);
  config_set_str(c, "role", "");
  config_set_str(c, "term", DEFAULT_TERM);
  config_set_str(c, "cjk_width", DEFAULT_CJK);
  config_set_str(c, "cursor_blink_mode", DEFAULT_BLINK_MODE);
  unsigned i = 0;
  unsigned size = sizeof(fields_str) / sizeof(char*);
  while (i < size) {
    const char* key = fields_str[i];
    // Set empty value if start with "color_"
    if (0 == g_ascii_strncasecmp(key, "color_", 6)) {
      config_set_str(c, key, "");
    }
    i++;
  }
  config_set_int(c, "width", DEFAULT_WIDTH);
  config_set_int(c, "height", DEFAULT_HEIGHT);

  config_set_bool(c, "use_default_keymap", true);
  config_set_bool(c, "allow_bold_font", true);
  config_set_bool(c, "no_quit", false);
}

void config_prepare(Config* c)
{
  lua_State* l = c->lua;
  lua_newtable(l);

  unsigned i;
  unsigned size;
  i = 0;
  size = sizeof(fields_str) / sizeof(char*);
  while (i < size) {
    const char* key = fields_str[i];
    const char* value = config_get_str(c, key);
    lua_pushstring(l, key);
    lua_pushstring(l, value ? value : "");
    lua_settable(l, -3);
    i++;
  }

  i = 0;
  size = sizeof(fields_int) / sizeof(char*);
  while (i < size) {
    const char* key = fields_int[i];
    int value = config_get_int(c, key);
    lua_pushstring(l, key);
    lua_pushinteger(l, value);
    lua_settable(l, -3);
    i++;
  }

  i = 0;
  size = sizeof(fields_bool) / sizeof(char*);
  while (i < size) {
    const char* key = fields_bool[i];
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

  unsigned i;
  unsigned size;
  i = 0;
  size = sizeof(fields_str) / sizeof(char*);
  while (i < size) {
    const char* key = fields_str[i];
    lua_getfield(l, -1, key);
    config_set_str(c, key, lua_tostring(l, -1));
    lua_pop(l, 1);
    i++;
  }

  i = 0;
  size = sizeof(fields_int) / sizeof(char*);
  while (i < size) {
    const char* key = fields_int[i];
    lua_getfield(l, -1, key);
    config_set_int(c, key, lua_tointeger(l, -1));
    lua_pop(l, 1);
    i++;
  }

  i = 0;
  size = sizeof(fields_bool) / sizeof(char*);
  while (i < size) {
    const char* key = fields_bool[i];
    lua_getfield(l, -1, key);
    config_set_bool(c, key, lua_toboolean(l, -1));
    lua_pop(l, 1);
    i++;
  }

  lua_pop(l, 1);
}


void config_load_option(Config* c, Option* option)
{
  const unsigned offset_long_name = strlen(OPTION_CONFIG_PREFIX);
  unsigned i, size;
  GOptionEntry* entries;

  i = 0;
  size = sizeof(fields_str) / sizeof(char*);
  entries = option_get_str_entries(option);
  while (i < size) {
    GOptionEntry* entry = &entries[i];
    const char* key = &entry->long_name[offset_long_name];
    if (*(char**)entry->arg_data) {
      config_set_str(c, key, *(char**)entry->arg_data);
    }
    i++;
  }

  i = 0;
  size = sizeof(fields_int) / sizeof(char*);
  entries = option_get_int_entries(option);
  while (i < size) {
    GOptionEntry* entry = &entries[i];
    const char* key = &entry->long_name[offset_long_name];
    if (*(int*)entry->arg_data) { // if not zero
      config_set_int(c, key, *(int*)entry->arg_data);
    }
    i++;
  }

  i = 0;
  size = sizeof(fields_bool) / sizeof(char*);
  entries = option_get_bool_entries(option);
  while (i < size) {
    GOptionEntry* entry = &entries[i];
    const char* key = &entry->long_name[offset_long_name];
    char* data = *(char**)entry->arg_data;
    if (data) {
      bool flag;
      if (0 == g_strcmp0(data, "true")) {
        flag = true;
      } else if (0 == g_strcmp0(data, "false")) {
        flag = false;
      } else {
        continue;
      }
      config_set_bool(c, key, flag);
    }
    i++;
  }
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
  char* role = config_has_str(c, "role")
    ? config_get_str(c, "role")
    : NULL;
  gtk_window_set_role(window, role);
  gtk_window_set_icon_name(window, config_get_icon(c));

  vte_terminal_set_cursor_blink_mode(vte, match_cursor_blink_mode(config_get_str(c, "cursor_blink_mode")));
  vte_terminal_set_cjk_ambiguous_width(vte, match_cjk_width(config_get_str(c, "cjk_width")));
  vte_terminal_set_allow_bold(vte, config_get_bool(c, "allow_bold_font"));

  int width = config_get_int(c, "width");
  int height = config_get_int(c, "height");
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

char* config_get_title(Config* c)
{
  return config_get_str(c, "title");
}

char* config_get_icon(Config* c)
{
  return config_get_str(c, "icon");
}

char* config_get_term(Config* c)
{
  return config_get_str(c, "term");
}

bool config_get_use_default_keymap(Config* c)
{
  return config_get_bool(c, "use_default_keymap");
}

bool config_get_no_quit(Config* c)
{
  return config_get_bool(c, "no_quit");
}
