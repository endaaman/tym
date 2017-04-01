#include <stdlib.h>
#include <stdbool.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <vte/vte.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>


#define VTE_CJK_WIDTH_NARROW 1
#define VTE_CJK_WIDTH_WIDE 2

#define APP_NAME "tym"
#define CONFIG_FILE_NAME "config.lua"
#define FALL_BACK_SHELL "/bin/sh"

#define CURSOR_BLINK_MODE_SYSTEM "system"
#define CURSOR_BLINK_MODE_ON "on"
#define CURSOR_BLINK_MODE_OFF "off"

#define CJK_WIDTH_NARROW "narrow"
#define CJK_WIDTH_WIDE "wide"

GList* config_fields = NULL;

static void init_config_fields() {
  const char* base_config_fields[] = {
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
  for (unsigned i = 0; i < sizeof(base_config_fields) / sizeof(char*); i++) {
    config_fields = g_list_append(config_fields, g_strdup(base_config_fields[i]));
  }
  // append `color_123` field
  char numbered_color_key[10];
  for (unsigned i = 0; i < 256; i++) {
    sprintf(numbered_color_key, "color_%d", i);
    config_fields = g_list_append(config_fields, g_strdup(numbered_color_key));
  }
}

static void close_config_fields() {
  g_list_foreach(config_fields, (GFunc)g_free, NULL);
  g_list_free(config_fields);
}

static char* get_default_shell() {
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

static VteCursorBlinkMode match_cursor_blink_mode(const char* str) {
  if (0 == g_strcmp0(str, CURSOR_BLINK_MODE_ON)) {
    return VTE_CURSOR_BLINK_ON;
  }
  if (0 == g_strcmp0(str, CURSOR_BLINK_MODE_OFF)) {
    return VTE_CURSOR_BLINK_OFF;
  }
  return VTE_CURSOR_BLINK_SYSTEM;
}

static VteCursorBlinkMode match_cjk_width(const char* str) {
  if (0 == g_strcmp0(str, CJK_WIDTH_WIDE)) {
    return VTE_CJK_WIDTH_WIDE;
  }
  return VTE_CJK_WIDTH_NARROW;
}

static char* config_get(GHashTable* c, const char* key) {
  return (char*) g_hash_table_lookup(c, key);
}

static bool config_has(GHashTable* c, const char* key) {
  char* value = config_get(c, key);
  if (!value) {
    return false;
  }
  if (0 == g_strcmp0(value, "")) {
    return false;
  }
  return true;
}

static void config_set(GHashTable* c, const char* key, const char* value) {
  char* old_key = NULL;
  bool has_value = g_hash_table_lookup_extended(c, key, (gpointer)&old_key, NULL);
  if (has_value) {
    g_hash_table_remove(c, old_key);
  }
  g_hash_table_insert(c, g_strdup(key), g_strdup(value));
}

static GHashTable* config_init() {
  GHashTable* config = g_hash_table_new_full(
    g_str_hash,
    g_str_equal,
    (GDestroyNotify)g_free,
    (GDestroyNotify)g_free
  );
  return config;
}

static void config_close(GHashTable* config) {
  g_hash_table_destroy(config);
}

static void config_reset(GHashTable* c) {
  char* default_shell = get_default_shell();
  config_set(c, "shell", default_shell);
  g_free(default_shell);
  config_set(c, "font", "");
  config_set(c, "cjk_width", CJK_WIDTH_NARROW);
  config_set(c, "cursor_blink_mode", CURSOR_BLINK_MODE_SYSTEM);
  for(GList* li = config_fields; li != NULL; li = li->next) {
    const char* key = (char *)li->data;
    // set empty value if start with "color_"
    if (0 == g_ascii_strncasecmp(key, "color_", 6)) {
      config_set(c, key, "");
    }
  }
}

static void config_load(GHashTable* c) {
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

  // push config to lua env
  lua_newtable(l);
  for(GList* li = config_fields; li != NULL; li = li->next) {
    const char* key = (char *)li->data;
    const char* value = config_get(c, key);
    if (!value) {
      g_printerr("warining: key `%s` is not initailized.\n", key);
    }
    lua_pushstring(l, key); \
    lua_pushstring(l, value ? value : ""); \
    lua_settable(l, -3); \
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
  for(GList* li = config_fields; li != NULL; li = li->next) {
    const char* key = (char *)li->data;
    lua_getfield(l, -1, key);
    config_set(c, key, lua_tostring(l, -1));
    lua_pop(l, 1);
  }
  lua_close(l);
}

typedef void (*VteSetColorFunc)(VteTerminal*, const GdkRGBA*);

static void config_apply_color(
  GHashTable* c,
  VteTerminal* vte,
  VteSetColorFunc vte_set_color_func,
  const char* key
) {
  if (!config_has(c, key)) {
    return;
  }
  GdkRGBA color;
  const char* value = config_get(c, key);
  bool valid = gdk_rgba_parse(&color, value);
  if (valid) {
    vte_set_color_func(vte, &color);
  }
}

static void config_apply_colors(GHashTable* c, VteTerminal* vte) {
  GdkRGBA* palette = g_new0(GdkRGBA, 256);
  char key[10];
  for (unsigned i = 0; i < 256; i++) {
    sprintf(key, "color_%d", i);
    if (config_has(c, key)) {
      bool valid = gdk_rgba_parse(&palette[i], config_get(c, key));
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

static void config_apply_all(GHashTable* c, VteTerminal* vte) {
  vte_terminal_set_cursor_blink_mode(vte, match_cursor_blink_mode(config_get(c, "cursor_blink_mode")));
  vte_terminal_set_cjk_ambiguous_width(vte, match_cjk_width(config_get(c, "cjk_width")));

  if (config_has(c, "font")) {
    PangoFontDescription* font_desc = pango_font_description_from_string(config_get(c, "font"));
    vte_terminal_set_font(vte, font_desc);
    pango_font_description_free(font_desc);
  }

  config_apply_color(c, vte, vte_terminal_set_color_bold, "color_bold");
  config_apply_color(c, vte, vte_terminal_set_color_background, "color_background");
  config_apply_color(c, vte, vte_terminal_set_color_foreground, "color_foreground");
  config_apply_color(c, vte, vte_terminal_set_color_cursor, "color_cursor");
  config_apply_color(c, vte, vte_terminal_set_color_cursor_foreground, "color_cursor_foreground");
  config_apply_color(c, vte, vte_terminal_set_color_highlight, "color_highlight");
  config_apply_color(c, vte, vte_terminal_set_color_highlight_foreground, "color_highlight_foreground");
  config_apply_colors(c, vte);
}

static void quit(GtkWidget* widget, gpointer data) {
  gtk_main_quit();
}


static bool on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
  VteTerminal* vte = VTE_TERMINAL(widget);
  GHashTable* config = *((GHashTable **)user_data);

  const unsigned int mod = event->state & gtk_accelerator_get_default_mod_mask();

  if (mod == (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) {
    switch (gdk_keyval_to_lower(event->keyval)) {
      case GDK_KEY_c:
        vte_terminal_copy_clipboard(vte);
        vte_terminal_unselect_all(vte);
        return true;
      case GDK_KEY_v:
        vte_terminal_paste_clipboard(vte);
        return true;
      case GDK_KEY_r:
        config_load(config);
        config_apply_all(config, vte);
        return true;
    }
  }
  return false;
}

static void start(GHashTable* c) {
  GError* error = NULL;

  // setup window
  GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "tym");
  gtk_window_set_icon_name(GTK_WINDOW(window), "terminal");
  gtk_container_set_border_width(GTK_CONTAINER(window), 0);
  g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(quit), NULL);

  // setup vte widget
  GtkWidget* vte_widget = vte_terminal_new();
  VteTerminal* vte = VTE_TERMINAL(vte_widget);
  g_signal_connect(G_OBJECT(vte), "child-exited", G_CALLBACK(quit), NULL);
  g_signal_connect(G_OBJECT(vte), "key-press-event", G_CALLBACK(on_key_press), &c);

  config_apply_all(c, vte);

  char* argv[2] = {config_get(c, "shell"), NULL};
  char** env = g_get_environ();

  GPid child_pid;
  vte_terminal_spawn_sync(
    vte,
    VTE_PTY_DEFAULT,
    NULL,
    argv,
    env,
    G_SPAWN_SEARCH_PATH,
    NULL,
    NULL,
    &child_pid,
    NULL,
    &error
  );
  g_strfreev(env);

  if (error) {
    g_printerr("%s\n", error->message);
    g_error_free(error);
    return;
  }

  vte_terminal_watch_child(vte, child_pid);

  gtk_container_add(GTK_CONTAINER(window), vte_widget);
  gtk_widget_grab_focus(vte_widget);
  gtk_widget_show_all(window);
  gtk_main();
}

int main(int argc, char* argv[])
{
  init_config_fields();

  GHashTable *config = config_init();
  config_load(config);
  gtk_init(&argc, &argv);

  start(config);

  config_close(config);
  close_config_fields();
  return EXIT_SUCCESS;
}
