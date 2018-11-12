/**
 * context.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "tym.h"


typedef void (*TymCommandFunc)(Context* context);
typedef struct {
  unsigned key;
  GdkModifierType mod;
  TymCommandFunc func;
} KeyPair;

typedef void (*TymSinalFunc)(Context* context);
typedef struct {
  const char* name;
  TymSinalFunc func;
} SignalDefinition;

static const char* TYM_MODULE_NAME = "tym";
static const char* TYM_DEFAULT_NOTIFICATION_TITLE = "tym";

static KeyPair DEFAULT_KEY_PAIRS[] = {
  { GDK_KEY_plus , GDK_CONTROL_MASK                 , command_increase_font_scale },
  { GDK_KEY_minus, GDK_CONTROL_MASK                 , command_decrease_font_scale },
  { GDK_KEY_equal, GDK_CONTROL_MASK                 , command_reset_font_scale    },
  { GDK_KEY_c    , GDK_CONTROL_MASK | GDK_SHIFT_MASK, command_copy_clipboard      },
  { GDK_KEY_v    , GDK_CONTROL_MASK | GDK_SHIFT_MASK, command_paste_clipboard     },
  { GDK_KEY_r    , GDK_CONTROL_MASK | GDK_SHIFT_MASK, command_reload              },
  { 0            , 0                                , NULL                        },
};

static SignalDefinition SIGNALS[] = {
  { "ReloadTheme", signal_reload_theme },
  { NULL, NULL },
};

static void context_load_config_path(Context* context)
{
  char* path = context->option->config_path;
  if (0 == g_strcmp0(path, TYM_SYMBOL_NONE)) {
    context->config_path = NULL;
  } else {
    if (path) {
      if (g_path_is_absolute(path)) {
        context->config_path = g_strdup(path);
      } else {
        char* cwd = g_get_current_dir();
        context->config_path = g_build_path(G_DIR_SEPARATOR_S, cwd, path, NULL);
        g_free(cwd);
      }
    } else {
      context->config_path = g_build_path(
        G_DIR_SEPARATOR_S,
        g_get_user_config_dir(),
        TYM_CONFIG_DIR_NAME,
        TYM_CONFIG_FILE_NAME,
        NULL
      );
    }
    dd("config path: `%s`", context->config_path);
  }
}

void context_acquire_theme_path(Context* context, char** ppath)
{
  if (!config_has_str(context->config, "theme")) {
    *ppath = NULL;
    return;
  }

  char* path = config_get_str(context->config, "theme");

  if (0 == g_strcmp0(path, TYM_SYMBOL_NONE)) {
    *ppath = NULL;
    return;
  }

  if (g_path_is_absolute(path)) {
    *ppath = g_strdup(path);
    return;
  }

  char* cwd = g_get_current_dir();
  *ppath = g_build_path(G_DIR_SEPARATOR_S, cwd, path, NULL);
  g_free(cwd);
}

static void context_prepare_lua(Context* context)
{
  lua_State* L = luaL_newstate();
  luaL_openlibs(L);
  luaX_requirec(L, TYM_MODULE_NAME, builtin_register_module, true, context);
  lua_pop(L, 1);
  ///* BACKWARD COMPAT BEGIN
  lua_newtable(L);
  lua_setglobal(L, "config");
  lua_newtable(L);
  lua_setglobal(L, "keymap");
  ///* BACKWARD COMPAT END
  context->lua = L;
}

Context* context_init()
{
  dd("init");
  Context* context = g_malloc0(sizeof(Context));
  context->option = option_init();
  context->config = config_init();
  context->keymap = keymap_init();
  context->hook = hook_init();
  context->layout = layout_init();
  context->app = gtk_application_new(TYM_APP_ID, G_APPLICATION_NON_UNIQUE | G_APPLICATION_HANDLES_OPEN);
  context_prepare_lua(context);
  return context;
}

void context_close(Context* context)
{
  dd("close");
  option_close(context->option);
  config_close(context->config);
  keymap_close(context->keymap);
  hook_close(context->hook);
  layout_close(context->layout);
  g_object_unref(context->app);
  lua_close(context->lua);
  g_free(context->config_path);
  g_free(context);
}

int context_start(Context* context, int argc, char** argv) {
  GError* error = NULL;
  bool is_continuous = option_check(context->option, &argc, &argv, &error);
  if (error) {
    g_message("%s", error->message);
    g_error_free(error);
    return EXIT_FAILURE;
  }

  if (!is_continuous) {
    return EXIT_SUCCESS;
  }

  // read option after option parsed
  context_load_config_path(context);

  g_signal_connect(context->app, "activate", G_CALLBACK(on_activate), context);
  g_signal_connect(context->app, "open", G_CALLBACK(on_open), context);
  return g_application_run(G_APPLICATION(context->app), argc, argv);
}

void context_load_device(Context* context)
{
  GdkDisplay* display = gdk_display_get_default();
#ifdef TYM_USE_GDK_SEAT
  GdkSeat* seat = gdk_display_get_default_seat(display);
  context->device = gdk_seat_get_keyboard(seat);
#else
  GdkDeviceManager* manager = gdk_display_get_device_manager(display);
  GList* devices = gdk_device_manager_list_devices(manager, GDK_DEVICE_TYPE_MASTER);
  for (GList* li = devices; li != NULL; li = li->next) {
  GdkDevice* d = (GdkDevice*)li->data;
    if (gdk_device_get_source(d) == GDK_SOURCE_KEYBOARD) {
      context->device = d;
      break;
    }
  }
  g_list_free(devices);
#endif
}

static void context_on_lua_error(Context* context, const char* error)
{
  char* message = g_strdup_printf("%s", error);
  g_message("%s", message);
  context_notify(context, error, "tym: lua error");
  g_free(message);
}

void context_load_config(Context* context)
{
  dd("load config start");

  if (!context->config_path) {
    g_message("Skipped config loading.");
    return;
  }

  if (context->loading) {
    g_message("Tried to load config recursively. Ignoring loading.");
    return;
  }
  context->loading = true;

  if (!g_file_test(context->config_path, G_FILE_TEST_EXISTS)) {
    g_message("Config file (`%s`) does not exist. Skipped config loading.", context->config_path);
    goto EXIT;
  }

  lua_State* L = context->lua;
  int result = luaL_dofile(L, context->config_path);
  if (result != LUA_OK) {
    const char* error = lua_tostring(L, -1);
    lua_pop(L, 1);
    context_on_lua_error(context, error);
    goto EXIT;
  }

  dd("load option to config");
  config_load_option_values(context->config, context->option);

EXIT:
  context->loading = false;
  dd("load config end");
}

void context_load_theme(Context* context)
{
  dd("load theme start");

  char* theme_path;
  context_acquire_theme_path(context, &theme_path);

  if (!theme_path) {
    g_message("Skipped theme loading.");
    goto EXIT;
  }

  if (!g_file_test(theme_path, G_FILE_TEST_EXISTS)) {
    // do not warn
    g_message("Theme file (`%s`) does not exist. Skiped theme loading.", theme_path);
    goto EXIT;
  }

  lua_State* L = context->lua;
  int result = luaL_loadfile(L, theme_path);
  if (result != LUA_OK) {
    g_warning("Could not load `%s`.", context->config_path);
    goto EXIT;
  }

  if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
    const char* error = lua_tostring(L, -1);
    g_message("Got error excuting theme script. Stopped theme loading.");
    context_on_lua_error(context, error);
    goto EXIT;
  }

  if (!lua_istable(L, -1)) {
    g_message("Theme script must return a table (got %s). Skiped theme assignment.", lua_typename(L, lua_type(L, -1)));
    goto EXIT;
  }

  lua_pushnil(L);
  while (lua_next(L, -2)) {
    lua_pushvalue(L, -2);
    const char* key = lua_tostring(L, -1);
    const char* value = lua_tostring(L, -2);
    if (value) {
      bool ok = false;
      if (strncmp("color_", key, 6) == 0) {
        ok = config_set_str(context->config, key, value);
      }
      if (!ok) {
        g_warning("%s: Invalid color key in theme: `%s`", theme_path, key);
      }
    }
    lua_pop(L, 2);
  }
  lua_pop(L, 1);

EXIT:
  g_free(theme_path);
  dd("load theme end");
}

static bool context_perform_default(Context* context, unsigned key, GdkModifierType mod)
{
  unsigned i = 0;
  while (DEFAULT_KEY_PAIRS[i].func) {
    KeyPair* pair = &DEFAULT_KEY_PAIRS[i];
    if ((key == pair->key) && !(~mod & pair->mod)) {
      pair->func(context);
      return true;
    }
    i++;
  }
  return false;
}

bool context_perform_keymap(Context* context, unsigned key, GdkModifierType mod)
{
  char* error = NULL;
  if (keymap_perform(context->keymap, context->lua, key, mod, &error)) {
    if (error) {
      context_on_lua_error(context, error);
      g_free(error);
    }
    return true;
  }
  if (config_get_bool(context->config, "ignore_default_keymap")) {
    return false;
  }
  return context_perform_default(context, key, mod);
}

void context_handle_signal(Context* context, const char* signal_name, GVariant* parameters)
{
  UNUSED(parameters);
  dd("SIGNAL: %s", signal_name);
  unsigned i = 0;
  while (SIGNALS[i].func) {
    SignalDefinition* def = &SIGNALS[i];
    if (0 == g_strcmp0(def->name, signal_name)) {
      def->func(context);
      return;
    }
    i++;
  }
}

void context_apply_config(Context* context)
{
  layout_apply_config(context->layout, context->config);
}

void context_apply_theme(Context* context)
{
  layout_apply_theme(context->layout, context->config);
}

void context_build_layout(Context* context)
{
  layout_build(context->layout, context->app, context->config);
}

VteTerminal* context_get_vte(Context* context)
{
  return context->layout->vte;
}

GtkWindow* context_get_window(Context* context)
{
  return context->layout->window;
}

void context_notify(Context* context, const char* body, const char* title)
{
  GtkApplication* app = context->app;

  GNotification* notification = g_notification_new(title ? title : TYM_DEFAULT_NOTIFICATION_TITLE);
  GIcon* icon = g_themed_icon_new_with_default_fallbacks(config_get_str(context->config, "icon"));

  g_notification_set_icon(notification, G_ICON(icon));
  g_notification_set_body(notification, body);
  g_notification_set_priority(notification, G_NOTIFICATION_PRIORITY_URGENT);
  g_application_send_notification(G_APPLICATION(app), TYM_APP_ID, notification);

  g_object_unref(notification);
  g_object_unref(icon);
}
