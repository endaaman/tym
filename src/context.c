/**
 * context.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "context.h"
#include "builtin.h"
#include "embedded.h"


static const char* CONFIG_FILE_NAME = "config.lua";
static const char* USE_DEFAULT_CONFIG_SYMBOL = "NONE";
static const char* CONFIG_DIR_NAME = "tym";

Context* context_init(const char* config_file_path)
{
  Context* context = g_malloc0(sizeof(Context));
  lua_State* l = luaL_newstate();
  luaL_openlibs(l);
  context->lua = l;

  if (0 == g_strcmp0(config_file_path, USE_DEFAULT_CONFIG_SYMBOL)) {
    // If symbol to start without config provived
    context->config_file_path = NULL;
    g_print("info: started with the default config\n");
  } else if (!config_file_path) {
    // If NULL
    context->use_default_config_file = true;
    context->config_file_path = g_build_path(
      G_DIR_SEPARATOR_S,
      g_get_user_config_dir(),
      CONFIG_DIR_NAME,
      CONFIG_FILE_NAME,
      NULL
    );
  } else {
    context->config_file_path = g_strdup(config_file_path);
  }

  context->config = config_init();
  context->keymap = keymap_init();

  context_embed_functions(context);

  return context;
}

void context_close(Context* context)
{
  lua_close(context->lua);
  g_free(context->config_file_path);
  config_close(context->config);
  keymap_close(context->keymap);
  g_free(context);
}

void context_set_app(Context* context, GtkApplication* app)
{
  context->app = app;
}

void context_set_vte(Context* context, VteTerminal* vte)
{
  context->vte = vte;
}

void context_load_config(Context* context, bool is_startup)
{
  lua_State* l = context->lua;

  config_reset(context->config);
  keymap_reset(context->keymap);

  if (!context->config_file_path) {
    // Running without config
    return;
  }

  if (!g_file_test(context->config_file_path, G_FILE_TEST_EXISTS)) {
    // Assert only if user config file provided
    if (!context->use_default_config_file) {
      g_print("warning: `%s` does not exist\n", context->config_file_path);
    }
    return;
  }

  config_prepare_lua(context->config, l);
  keymap_prepare_lua(context->keymap, l);

  luaL_loadfile(l, context->config_file_path);

  if (lua_pcall(l, 0, 0, 0)) {
    g_print("warning: config error %s\n", lua_tostring(l, -1));
    g_print("warning: start with default configuration...\n");
    return;
  }

  config_load_from_lua(context->config, l);
  keymap_load_from_lua(context->keymap, l);

  config_apply_all(context->config, context->vte, is_startup);
}

bool context_on_key(Context* context, unsigned key, GdkModifierType mod)
{
  if (keymap_perform_custom(context->keymap, context->lua, key, mod)) {
    return true;
  }
  if (config_get_use_default_keymap(context->config) && keymap_perform_default(context->keymap, context, key, mod)) {
    return true;
  }
  return false;
}
