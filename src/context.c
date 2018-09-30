/**
 * context.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "context.h"
#include "command.h"
#include "builtin.h"


typedef void (*TymCommandFunc)(Context* context);

typedef struct {
  unsigned key;
  GdkModifierType mod;
  TymCommandFunc func;
} KeyPair;

static const char* CONFIG_FILE_NAME = "config.lua";
static const char* USE_DEFAULT_CONFIG_SYMBOL = "NONE";
static const char* CONFIG_DIR_NAME = "tym";
static const char* LIB_NAME = "tym";

static void context_embed_builtin_functions(Context* context); // declare forward


static KeyPair default_key_pairs[] = {
  { GDK_KEY_plus , GDK_CONTROL_MASK                 , command_increase_font_scale },
  { GDK_KEY_minus, GDK_CONTROL_MASK                 , command_decrease_font_scale },
  { GDK_KEY_equal, GDK_CONTROL_MASK                 , command_reset_font_scale    },
  { GDK_KEY_c    , GDK_CONTROL_MASK | GDK_SHIFT_MASK, command_copy_clipboard      },
  { GDK_KEY_v    , GDK_CONTROL_MASK | GDK_SHIFT_MASK, command_paste_clipboard     },
  { GDK_KEY_r    , GDK_CONTROL_MASK | GDK_SHIFT_MASK, command_reload              },
  { 0            , 0                                , NULL                        },
};

Context* context_init(Option* option, GtkApplication* app, VteTerminal* vte)
{
  dd("init");

  Context* context = g_malloc0(sizeof(Context));

  lua_State* l = luaL_newstate();
  luaL_openlibs(l);
  context->lua = l;

  char* path = option->config_file_path;
  if (0 == g_strcmp0(path, USE_DEFAULT_CONFIG_SYMBOL)) {
    // If symbol to start without config provived
    context->config_file_path = NULL;
    g_message("starting with default config");
  } else {
    if (path) {
      if (g_path_is_absolute(path)) {
        context->config_file_path = g_strdup(path);
      } else {
        char* cwd = g_get_current_dir();
        context->config_file_path = g_build_path(cwd, path, NULL);
        g_free(cwd);
      }
    } else {
      context->config_file_path = g_build_path(
        G_DIR_SEPARATOR_S,
        g_get_user_config_dir(),
        CONFIG_DIR_NAME,
        CONFIG_FILE_NAME,
        NULL
      );
    }
    dd("config path: `%s`", context->config_file_path);
  }

  context->option = option;
  context->app = app;
  context->vte = vte;

  context->config = config_init(l);
  context->keymap = keymap_init(l);

  context_embed_builtin_functions(context);
  return context;
}

void context_close(Context* context)
{
  dd("close");

  lua_close(context->lua);
  g_free(context->config_file_path);
  config_close(context->config);
  keymap_close(context->keymap);
  g_free(context);
}


static void context_on_lua_error(Context* context, const char* error)
{
  char* message = g_strdup_printf("Error in %s: %s", context->config_file_path, error);
  g_warning(message);
  command_notify(context, message, NULL);
  g_free(message);
}

void context_load(Context* context)
{
  dd("load start");

  if (!context->config_file_path) {
    // Running without config
    dd("load exit for running with out config");
    return;
  }

  if (context->loading) {
    dd("load exit for recursive loading");
    g_warning("Tried to load config recursively");
    return;
  }

  context->loading = true;

  config_reset(context->config);
  keymap_reset(context->keymap);

  if (!g_file_test(context->config_file_path, G_FILE_TEST_EXISTS)) {
    dd("load exit for the file(%s) does not exists", context->config_file_path);
    // Warn only if user config file provided
    g_warning("`%s` does not exist. skipping loading", context->config_file_path);
    goto EXIT;
  }

  config_prepare(context->config);
  keymap_prepare(context->keymap);

  lua_State* l = context->lua;

  int result = luaL_loadfile(l, context->config_file_path);
  if (result != LUA_OK) {
    g_warning("Could not load `%s`. skipping loading", context->config_file_path);
    goto EXIT;
  }

  if (lua_pcall(l, 0, 0, 0)) {
    const char* error = lua_tostring(l, -1);
    dd("load exit for lua error: %s", error);
    context_on_lua_error(context, error);
    g_message("starting with default config and keymap");
    goto EXIT;
  }

  char* error = NULL;

  config_load(context->config, &error);
  if (error) {
    dd("load exit for config load error: %s", error);
    context_on_lua_error(context, error);
    g_message("starting with default config");
    g_free(error);
    goto EXIT;
  }

  keymap_load(context->keymap, &error);
  if (error) {
    dd("load exit for keymap load error: %s", error);
    context_on_lua_error(context, error);
    g_message("starting without custom keymap");
    g_free(error);
    goto EXIT;
  }

  config_load_option(context->config, context->option);

  config_apply(context->config, context->vte);
  dd("load finished");

EXIT:
  context->loading = false;
  return;
}

static void context_embed_builtin_functions(Context* context)
{
  const luaL_Reg table[] = {
    { "get_version"         , builtin_get_version          },
    { "get_config_file_path", builtin_get_config_file_path },
    { "notify"              , builtin_notify               },
    { "put"                 , builtin_put                  },
    { "reload"              , builtin_reload               },
    { "increase_font_scale" , builtin_increase_font_scale  },
    { "decrease_font_scale" , builtin_decrease_font_scale  },
    { "reset_font_scale"    , builtin_reset_font_scale     },
    { "copy_clipboard"      , builtin_copy_clipboard       },
    { "copy"                , builtin_copy_clipboard       },
    { "paste_clipboard"     , builtin_paste_clipboard      },
    { "paste"               , builtin_paste_clipboard      },
    { NULL, NULL },
  };

  lua_State* l = context->lua;
  luaL_newlibtable(l, table);
  lua_pushlightuserdata(l, context);
  luaL_setfuncs(l, table, 1);
  lua_setglobal(l, LIB_NAME);
}

static bool context_perform_default(Context* context, unsigned key, GdkModifierType mod)
{
  unsigned i = 0;
  while (default_key_pairs[i].func) {
    if ((key == default_key_pairs[i].key) && !(~mod & default_key_pairs[i].mod)) {
      default_key_pairs[i].func(context);
      return true;
    }
    i++;
  }
  return false;
}

bool context_perform_keymap(Context* context, unsigned key, GdkModifierType mod)
{
  char* error = NULL;
  if (keymap_perform_custom(context->keymap, key, mod, &error)) {
    if (error) {
      context_on_lua_error(context, error);
      g_free(error);
    }
    return true;
  }

  if (
    config_get_use_default_keymap(context->config) &&
    context_perform_default(context, key, mod)
  ) {
    return true;
  }
  return false;
}

void context_on_change_vte_title(Context* context) {
  GtkWindow* window = GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(context->vte)));

  const char* terminal_title = vte_terminal_get_window_title(context->vte);
  const char* title = terminal_title
    ? terminal_title
    : config_get_title(context->config);

  gtk_window_set_title(window, title);
}
