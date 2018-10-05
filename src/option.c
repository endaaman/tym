/**
 * option.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "tym.h"


Option* option_init() {
  Option* option = g_malloc0(sizeof(Option));
  // --version and --use
  const unsigned offset_option = 2;
  GOptionEntry* ee = (GOptionEntry*)g_malloc0_n(sizeof(GOptionEntry), config_fields_len + offset_option + 1);

  ee[0].long_name = "version";
  ee[0].short_name = 'v';
  ee[0].flags = G_OPTION_FLAG_NONE;
  ee[0].arg = G_OPTION_ARG_NONE;
  ee[0].arg_data = &option->version;
  ee[0].description = "Show version";
  ee[0].arg_description = NULL;

  ee[1].long_name = "use";
  ee[1].short_name = 'u';
  ee[1].flags = G_OPTION_FLAG_NONE;
  ee[1].arg = G_OPTION_ARG_STRING;
  ee[1].arg_data = &option->config_path;
  ee[1].description = "<path> to config file. Set " TYM_SYMBOL_NONE " to start without loading config.";
  ee[1].arg_description = "<path>";

  unsigned i = offset_option;

  for (GList* li = config_fields; li != NULL; li = li->next) {
    ConfigField* field = (ConfigField*)li->data;
    GOptionEntry* e = &ee[i];
    i += 1;
    e->long_name = field->name;
    e->short_name = field->short_name;
    e->flags = field->option_flag;
    e->arg_description = field->arg_desc;
    e->description = field->desc;
    e->arg_data = &field->option_data; // TODO: do not use &field->option_data, use GOptionArgFunc()
    switch (field->type) {
      case CONFIG_TYPE_STRING:
        e->arg = G_OPTION_ARG_STRING;
        break;
      case CONFIG_TYPE_INTEGER:
        e->arg = G_OPTION_ARG_INT;
        break;
      case CONFIG_TYPE_BOOLEAN:
        e->arg = G_OPTION_ARG_NONE;
        break;
      case CONFIG_TYPE_NONE:;
        break;
    }
  }

  option->entries = ee;
  return option;
}

void option_close(Option* option) {
  g_free(option->entries);
  g_free(option);
}

bool option_check(Option* option, int* argc, char*** argv, GError** error) {
  GOptionContext* option_context = g_option_context_new(NULL);
  g_option_context_add_main_entries(option_context, option->entries, NULL);

  if (!g_option_context_parse(option_context, argc, argv, error)) {
    g_option_context_free(option_context);
    return false;
  }

  if (option->version) {
    g_option_context_free(option_context);
    g_print("version %s\n", PACKAGE_VERSION);
    return false;
  }

  g_option_context_free(option_context);
  return true;
}
