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

  GOptionEntry app_options[] = {
    {
      .long_name = "version",
      .short_name = 'v',
      .flags = G_OPTION_FLAG_NONE,
      .arg = G_OPTION_ARG_NONE,
      .arg_data = &option->version,
      .description = "Show version",
      .arg_description = NULL,
    }, {
      .long_name = "signal",
      .short_name = 's',
      .flags = G_OPTION_FLAG_NONE,
      .arg = G_OPTION_ARG_STRING,
      .arg_data = &option->signal,
      .description = "Signal to send via D-Bus",
      .arg_description = "<signal>",
    }, {
      .long_name = "use",
      .short_name = 'u',
      .flags = G_OPTION_FLAG_NONE,
      .arg = G_OPTION_ARG_STRING,
      .arg_data = &option->config_path,
      .description = "<path> to config file. Set '" TYM_SYMBOL_NONE "' to start without loading config",
      .arg_description = "<path>",
    }, {
      .long_name = "theme",
      .short_name = 't',
      .flags = G_OPTION_FLAG_NONE,
      .arg = G_OPTION_ARG_STRING,
      .arg_data = &option->theme_path,
      .description = "<path> to theme file. Set '" TYM_SYMBOL_NONE "' to start without loading theme",
      .arg_description = "<path>",
    }, {
      .long_name = "nolua",
      .flags = G_OPTION_FLAG_NONE,
      .arg = G_OPTION_ARG_NONE,
      .arg_data = &option->nolua,
      .description = "Launch without Lua context",
    }
  };

  GOptionEntry* options_entries = (GOptionEntry*)g_malloc0_n(
      sizeof(app_options) / sizeof(GOptionEntry) + meta_size(meta) + 1,
      sizeof(GOptionEntry));
  memmove(options_entries, app_options, sizeof(app_options));
  unsigned i = sizeof(app_options) / sizeof(GOptionEntry);

  for (GList* li = meta->list; li != NULL; li = li->next) {
    MetaEntry* me = (MetaEntry*)li->data;
    GOptionEntry* e = &options_entries[i];
    i += 1;
    e->long_name = me->name;
    e->short_name = me->short_name;
    e->flags = me->option_flag;
    e->arg_description = me->arg_desc;
    e->description = me->desc;
    switch (me->type) {
      case META_ENTRY_TYPE_STRING:
        e->arg = G_OPTION_ARG_STRING;
        char** ps = g_malloc0(sizeof(char*));
        *ps = me->default_value;
        e->arg_data = ps;
        break;
      case META_ENTRY_TYPE_INTEGER:
        e->arg = G_OPTION_ARG_INT;
        /* TODO: need to dispose */
        int* pi = g_malloc0(sizeof(int*));
        *pi = *(int*)me->default_value;
        e->arg_data = pi;
        break;
      case META_ENTRY_TYPE_BOOLEAN:
        e->arg = G_OPTION_ARG_NONE;
        /* TODO: need to dispose */
        bool* pb = g_malloc0(sizeof(bool*));
        *pb = *(bool*)me->default_value;
        e->arg_data = pb;
        break;
      case META_ENTRY_TYPE_NONE:;
        break;
    }
  }
  option->entries = options_entries;
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

bool option_parse(Option* option, int* argc, char*** argv)
{
  if (option->option_context) {
    g_message("option context is already registerd.");
    return false;
  }
  GError* error = NULL;
  option->option_context = g_option_context_new(NULL);
  /* g_option_context_set_help_enabled(option_context, FALSE); */
  g_option_context_add_main_entries(option->option_context, option->entries, NULL);
  g_option_context_parse(option->option_context, argc, argv, &error);
  if (error) {
    g_error("%s", error->message);
    g_error_free(error);
    return false;
  }
  return true;
}

void option_load_from_cli(Option* option, GApplicationCommandLine* cli)
{
  GVariantDict* values = g_application_command_line_get_options_dict(cli);
  if (option->values) {
    g_variant_dict_unref(option->values);
  }
  option->values = g_variant_dict_ref(values);
}


void* option_get(Option* option, const char* key)
{
  int i = 0;

  GOptionEntry* e = NULL;
  while (&option->entries[i]) {
    e = &option->entries[i];
    if (is_equal(key, e->long_name)) {
      return e->arg_data;
    }
    i += 1;
  }
  return NULL;
}

/* bool option_get_str_value(Option* option, const char* key, const char** value) */
/* { */
/*   if (!option->values) { */
/*     return false; */
/*   } */
/*   char* v; */
/*   bool has_value = g_variant_dict_lookup(option->values, key, "s", &v); */
/*   if (has_value) { */
/*     *value = v; */
/*   } */
/*   return has_value; */
/* } */
/*  */
/* bool option_get_int_value(Option* option, const char* key, int* value) */
/* { */
/*   if (!option->values) { */
/*     return false; */
/*   } */
/*   int v; */
/*   bool has_value = g_variant_dict_lookup(option->values, key, "i", &v); */
/*   if (has_value) { */
/*     *value = v; */
/*   } */
/*   return has_value; */
/* } */
/*  */
/* bool option_get_bool_value(Option* option, const char* key, bool* value) */
/* { */
/*   if (!option->values) { */
/*     return false; */
/*   } */
/*   gboolean v; */
/*   bool has_value = g_variant_dict_lookup(option->values, key, "b", &v); */
/*   if (has_value) { */
/*     *value = v; */
/*   } */
/*   return has_value; */
/* } */

bool option_get_str_value(Option* option, const char* key, const char** value)
{
  void* p = option_get(option, key);
  if (!p) {
    return false;
  }
  if (!(char**)p) {
    return false;
  }
  *value = *(char**)p;
  return true;
}

bool option_get_int_value(Option* option, const char* key, int* value)
{
  void* p = option_get(option, key);
  if (!p) {
    return false;
  }
  *value = *(int*)p;
  return true;
}

bool option_get_bool_value(Option* option, const char* key, bool* value)
{
  void* p = option_get(option, key);
  if (!p) {
    return false;
  }
  *value = *(bool*)p;
  return true;
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
