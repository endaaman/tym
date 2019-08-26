/**
 * option.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "option.h"
#include "meta.h"


Option* option_init(Meta* meta)
{
  Option* option = g_malloc0(sizeof(Option));
  const unsigned offset_option = 5;
  GOptionEntry* ee = (GOptionEntry*)g_malloc0_n(sizeof(GOptionEntry), meta_size(meta) + offset_option + 1);

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

  ee[2].long_name = "theme";
  ee[2].short_name = 't';
  ee[2].flags = G_OPTION_FLAG_NONE;
  ee[2].arg = G_OPTION_ARG_STRING;
  ee[2].arg_data = &option->theme_path;
  ee[2].description = "<path> to theme file. Set '" TYM_SYMBOL_NONE "' to start without loading theme.";
  ee[2].arg_description = "<path>";

  ee[3].long_name = "signal";
  ee[3].short_name = 's';
  ee[3].flags = G_OPTION_FLAG_NONE;
  ee[3].arg = G_OPTION_ARG_STRING;
  ee[3].arg_data = &option->signal;
  ee[3].description = "Signal to send via D-Bus.";
  ee[3].arg_description = "<signal>";

  ee[4].long_name = "nolua";
  ee[4].flags = G_OPTION_FLAG_NONE;
  ee[4].arg = G_OPTION_ARG_NONE;
  ee[4].arg_data = &option->nolua;
  ee[4].description = "Disable to create Lua context.";

  unsigned i = offset_option;

  GList* mee = meta->list;
  for (GList* li = mee; li != NULL; li = li->next) {
    MetaEntry* me = (MetaEntry*)li->data;
    GOptionEntry* e = &ee[i];
    i += 1;
    e->long_name = me->name;
    e->short_name = me->short_name;
    e->flags = me->option_flag;
    e->arg_description = me->arg_desc;
    e->description = me->desc;
    switch (me->type) {
      case META_ENTRY_TYPE_STRING:
        e->arg = G_OPTION_ARG_STRING;
        break;
      case META_ENTRY_TYPE_INTEGER:
        e->arg = G_OPTION_ARG_INT;
        break;
      case META_ENTRY_TYPE_BOOLEAN:
        e->arg = G_OPTION_ARG_NONE;
        break;
      case META_ENTRY_TYPE_NONE:;
        break;
    }
  }

  option->entries = ee;
  return option;
}

void option_close(Option* option)
{
  if (option->values) {
    g_variant_dict_unref(option->values);
  }
  g_free(option->entries);
  g_free(option);
}

void option_set_values(Option* option, GVariantDict* values)
{
  if (option->values) {
    g_variant_dict_unref(option->values);
  }
  option->values = g_variant_dict_ref(values);
}

bool option_get_str_value(Option* option, const char* key, const char** value)
{
  char* v;
  bool has_value = g_variant_dict_lookup(option->values, key, "s", &v);
  if (has_value) {
    *value = v;
  }
  return has_value;
}

bool option_get_int_value(Option* option, const char* key, int* value)
{
  int v;
  bool has_value = g_variant_dict_lookup(option->values, key, "i", &v);
  if (has_value) {
    *value = v;
  }
  return has_value;
}

bool option_get_bool_value(Option* option, const char* key, bool* value)
{
  gboolean v;
  bool has_value = g_variant_dict_lookup(option->values, key, "b", &v);
  if (has_value) {
    *value = v;
  }
  return has_value;
}

bool option_get_version(Option* option)
{
  return option->version;
}

char* option_get_config_path(Option* option)
{
  return option->config_path;
}

char* option_get_theme_path(Option* option)
{
  return option->theme_path;
}

char* option_get_signal(Option* option)
{
  return option->signal;
}

bool option_get_nolua(Option* option)
{
  return option->nolua;
}
