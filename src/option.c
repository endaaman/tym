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
  const unsigned offset_option = 3;
  GOptionEntry* ee = (GOptionEntry*)g_malloc0_n(sizeof(GOptionEntry), get_config_fields_count() + offset_option + 1);

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
  ee[1].description = "<path> to config file. Set '" TYM_SYMBOL_NONE "' to start without loading config.";
  ee[1].arg_description = "<path>";

  ee[2].long_name = "signal";
  ee[2].short_name = 's';
  ee[2].flags = G_OPTION_FLAG_NONE;
  ee[2].arg = G_OPTION_ARG_STRING;
  ee[2].arg_data = &option->signal;
  ee[2].description = "Signal to send via D-Bus.";
  ee[2].arg_description = "<signal>";

  unsigned i = offset_option;

  GHashTableIter iter;
  char* key = NULL;
  ConfigField* field = NULL;
  g_hash_table_iter_init(&iter, get_config_fields());
  while (g_hash_table_iter_next(&iter, (void*)&key, (void*)&field)) {
    // TODO: sort by index
    GOptionEntry* e = &ee[i];
    i += 1;
    e->long_name = field->name;
    e->short_name = field->short_name;
    e->flags = field->option_flag;
    e->arg_description = field->arg_desc;
    e->description = field->desc;
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
  if (option->values) {
    g_variant_dict_unref(option->values);
  }
  g_free(option->entries);
  g_free(option);
}

void option_set_values(Option* option, GVariantDict* values) {
  if (option->values) {
    g_variant_dict_unref(option->values);
  }
  option->values = g_variant_dict_ref(values);
}

bool option_get_str_value(Option* option, const char* key, char** value) {
  char* v;
  bool has_value = g_variant_dict_lookup(option->values, key, "s", &v);
  if (has_value) {
    *value = v;
  }
  return has_value;
}

bool option_get_int_value(Option* option, const char* key, int* value) {
  int v;
  bool has_value = g_variant_dict_lookup(option->values, key, "i", &v);
  if (has_value) {
    *value = v;
  }
  return has_value;
}

bool option_get_bool_value(Option* option, const char* key, bool* value) {
  gboolean v;
  bool has_value = g_variant_dict_lookup(option->values, key, "b", &v);
  if (has_value) {
    *value = v;
  }
  return has_value;
}

bool option_get_version(Option* option) {
  return option->version;
}

char* option_get_config_path(Option* option) {
  return option->config_path;
}

char* option_get_signal(Option* option) {
  return option->signal;
}
