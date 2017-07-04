/**
 * config.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "config.h"

typedef void (*VteSetColorFunc)(VteTerminal*, const GdkRGBA*);

const unsigned VTE_CJK_WIDTH_NARROW = 1;
const unsigned VTE_CJK_WIDTH_WIDE = 2;

const char* APP_NAME = "tym";
const char* CONFIG_FILE_NAME = "config.lua";
const char* FALL_BACK_SHELL = "/bin/sh";
const int DEFAULT_WIDTH = 80;
const int DEFAULT_HEIGHT = 22;

const char* CURSOR_BLINK_MODE_SYSTEM = "system";
const char* CURSOR_BLINK_MODE_ON = "on";
const char* CURSOR_BLINK_MODE_OFF = "off";

const char* CJK_WIDTH_NARROW = "narrow";
const char* CJK_WIDTH_WIDE = "wide";

GList* config_fields = NULL;
GList* config_fields_int = NULL;

void init_config_fields() {
  const char* base_config_fields[] = {
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

  const char* base_config_fields_int[] = {
    "width",
    "height",
  };

  // initialize string fields
  for (unsigned i = 0; i < sizeof(base_config_fields) / sizeof(char*); i++) {
    config_fields = g_list_append(config_fields, g_strdup(base_config_fields[i]));
  }
  // append `color_123` field
  char numbered_color_key[10];
  for (unsigned i = 0; i < 256; i++) {
    sprintf(numbered_color_key, "color_%d", i);
    config_fields = g_list_append(config_fields, g_strdup(numbered_color_key));
  }

  // initialize integer fields
  for (unsigned i = 0; i < sizeof(base_config_fields_int) / sizeof(char*); i++) {
    config_fields_int = g_list_append(config_fields_int, g_strdup(base_config_fields_int[i]));
  }
}

void close_config_fields() {
  g_list_foreach(config_fields, (GFunc)g_free, NULL);
  g_list_foreach(config_fields_int, (GFunc)g_free, NULL);
  g_list_free(config_fields);
}

char* get_default_shell() {
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

VteCursorBlinkMode match_cursor_blink_mode(const char* str) {
  if (0 == g_strcmp0(str, CURSOR_BLINK_MODE_ON)) {
    return VTE_CURSOR_BLINK_ON;
  }
  if (0 == g_strcmp0(str, CURSOR_BLINK_MODE_OFF)) {
    return VTE_CURSOR_BLINK_OFF;
  }
  return VTE_CURSOR_BLINK_SYSTEM;
}

unsigned match_cjk_width(const char* str) {
  if (0 == g_strcmp0(str, CJK_WIDTH_WIDE)) {
    return VTE_CJK_WIDTH_WIDE;
  }
  return VTE_CJK_WIDTH_NARROW;
}

char* config_get_str(GHashTable* c, const char* key) {
  char* ptr = (char*)g_hash_table_lookup(c, key);
  if (!ptr) {
    g_printerr("warining: tried to refer null string field: `%s`.\n", key);
  }
  return ptr;
}

int config_get_int(GHashTable* c, const char* key) {
  int* ptr = (int*)g_hash_table_lookup(c, key);
  if (!ptr) {
    g_printerr("warining: tried to refer null integer field: `%s`.\n", key);
    return 0;
  }
  return *ptr;
}

bool config_has(GHashTable* c, const char* key) {
  char* value = config_get_str(c, key);
  if (!value) {
    return false;
  }
  if (0 == g_strcmp0(value, "")) {
    return false;
  }
  return true;
}

void config_set_str(GHashTable* c, const char* key, const char* value) {
  char* old_key = NULL;
  bool has_value = g_hash_table_lookup_extended(c, key, (gpointer)&old_key, NULL);
  if (has_value) {
    g_hash_table_remove(c, old_key);
  }
  g_hash_table_insert(c, g_strdup(key), g_strdup(value));
}


void config_set_int(GHashTable* c, const char* key, int value) {
  char* old_key = NULL;
  bool has_value = g_hash_table_lookup_extended(c, key, (gpointer)&old_key, NULL);
  if (has_value) {
    g_hash_table_remove(c, old_key);
  }
  g_hash_table_insert(c, g_strdup(key), g_memdup((gpointer)&value, sizeof(int)));
}

GHashTable* config_init() {
  GHashTable* config = g_hash_table_new_full(
    g_str_hash,
    g_str_equal,
    (GDestroyNotify)g_free,
    (GDestroyNotify)g_free
  );
  return config;
}

void config_close(GHashTable* config) {
  g_hash_table_destroy(config);
}

void config_reset(GHashTable* c) {
  char* default_shell = get_default_shell();
  config_set_str(c, "shell", default_shell);
  g_free(default_shell);
  config_set_str(c, "title", APP_NAME);
  config_set_str(c, "font", "");
  config_set_str(c, "cjk_width", CJK_WIDTH_NARROW);
  config_set_str(c, "cursor_blink_mode", CURSOR_BLINK_MODE_SYSTEM);
  for (GList* li = config_fields; li != NULL; li = li->next) {
    const char* key = (char *)li->data;
    // set empty value if start with "color_"
    if (0 == g_ascii_strncasecmp(key, "color_", 6)) {
      config_set_str(c, key, "");
    }
  }

  config_set_int(c, "width", DEFAULT_WIDTH);
  config_set_int(c, "height", DEFAULT_HEIGHT);
}


void config_load(GHashTable* c) {
  char* config_file_path = g_build_path(
    G_DIR_SEPARATOR_S,
    g_get_user_config_dir(),
    APP_NAME,
    CONFIG_FILE_NAME,
    NULL
  );
  config_reset(c);

  // early return if config file does not exist
  if (!g_file_test(config_file_path, G_FILE_TEST_EXISTS)) {
    g_free(config_file_path);
    return;
  }

  lua_State* l = luaL_newstate();
  luaL_openlibs(l);

  lua_newtable(l);
  // push string fields
  for (GList* li = config_fields; li != NULL; li = li->next) {
    const char* key = (char*)li->data;
    const char* value = config_get_str(c, key);
    lua_pushstring(l, key);
    lua_pushstring(l, value ? value : "");
    lua_settable(l, -3);
  }
  // push int fields
  for (GList* li = config_fields_int; li != NULL; li = li->next) {
    const char* key = (char*)li->data;
    int value = config_get_int(c, key);
    lua_pushstring(l, key);
    lua_pushinteger(l, value);
    lua_settable(l, -3);
  }
  lua_setglobal(l, "config");

  // run user config file
  luaL_loadfile(l, config_file_path);
  g_free(config_file_path);

  // if error
  if (lua_pcall(l, 0, 0, 0)) {
    g_printerr("warining: config error %s\n", lua_tostring(l, -1));
    g_printerr("warining: start with default configuration...\n");
    return;
  }

  lua_getglobal(l, "config");
  // store string fields
  for (GList* li = config_fields; li != NULL; li = li->next) {
    const char* key = (char *)li->data;
    lua_getfield(l, -1, key);
    config_set_str(c, key, lua_tostring(l, -1));
    lua_pop(l, 1);
  }
  // store int fields
  for (GList* li = config_fields_int; li != NULL; li = li->next) {
    const char* key = (char *)li->data;
    lua_getfield(l, -1, key);
    config_set_int(c, key, lua_tointeger(l, -1));
    lua_pop(l, 1);
  }
  lua_close(l);
}

void config_apply_color(
  GHashTable* c,
  VteTerminal* vte,
  VteSetColorFunc vte_set_color_func,
  const char* key
) {
  if (!config_has(c, key)) {
    return;
  }
  GdkRGBA color;
  const char* value = config_get_str(c, key);
  bool valid = gdk_rgba_parse(&color, value);
  if (valid) {
    vte_set_color_func(vte, &color);
  }
}

void config_apply_colors(GHashTable* c, VteTerminal* vte) {
  GdkRGBA* palette = g_new0(GdkRGBA, 256);
  char key[10];
  for (unsigned i = 0; i < 256; i++) {
    sprintf(key, "color_%d", i);
    if (config_has(c, key)) {
      bool valid = gdk_rgba_parse(&palette[i], config_get_str(c, key));
      if (valid) {
        continue;
      }
    }
    if (i < 16) {
      palette[i].blue = (((i & 4) ? 0xc000 : 0) + (i > 7 ? 0x3fff: 0)) / 65535.0;
      palette[i].green = (((i & 2) ? 0xc000 : 0) + (i > 7 ? 0x3fff : 0)) / 65535.0;
      palette[i].red = (((i & 1) ? 0xc000 : 0) + (i > 7 ? 0x3fff : 0)) / 65535.0;
      palette[i].alpha = 0;
      continue;
    }
    if (i < 232) {
      const unsigned j = i - 16;
      const unsigned r = j / 36, g = (j / 6) % 6, b = j % 6;
      const unsigned red =   (r == 0) ? 0 : r * 40 + 55;
      const unsigned green = (g == 0) ? 0 : g * 40 + 55;
      const unsigned blue =  (b == 0) ? 0 : b * 40 + 55;
      palette[i].red   = (red | red << 8) / 65535.0;
      palette[i].green = (green | green << 8) / 65535.0;
      palette[i].blue  = (blue | blue << 8) / 65535.0;
      palette[i].alpha = 0;
      continue;
    }
    const unsigned shade = 8 + (i - 232) * 10;
    palette[i].red = palette[i].green = palette[i].blue = (shade | shade << 8) / 65535.0;
    palette[i].alpha = 0;
  }
  vte_terminal_set_colors(vte, NULL, NULL, palette, 256);
}

void config_apply_all(GHashTable* c, VteTerminal* vte, bool is_startup) {
  GtkWidget* window = gtk_widget_get_toplevel(GTK_WIDGET(vte));

  gtk_window_set_title(GTK_WINDOW(window), config_get_str(c, "title"));

  vte_terminal_set_cursor_blink_mode(vte, match_cursor_blink_mode(config_get_str(c, "cursor_blink_mode")));
  vte_terminal_set_cjk_ambiguous_width(vte, match_cjk_width(config_get_str(c, "cjk_width")));

  int width = config_get_int(c, "width"); // Number of horizontal char
  int height = config_get_int(c, "height"); // Number of vertical char
  if (is_startup){
    vte_terminal_set_size(vte, width, height);
  } else {
    GtkBorder border;
    gtk_style_context_get_padding(
      gtk_widget_get_style_context(GTK_WIDGET(vte)),
      gtk_widget_get_state_flags(GTK_WIDGET(vte)),
      &border
    );
    const int char_width = vte_terminal_get_char_width(vte);
    const int char_height = vte_terminal_get_char_height(vte);
    gtk_window_resize(
      GTK_WINDOW(window),
      width * char_width + border.left + border.right,
      height * char_height + border.top + border.bottom
    );
  }

  if (config_has(c, "font")) {
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
