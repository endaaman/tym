/**
 * option.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "option.h"
#include "app.h"


Option* option_init()
{
  Option* option = g_malloc0(sizeof(Option));
  /* option->entries = entries; */
  return option;
}

void option_close(Option* option)
{
  if (option->values) {
    g_variant_dict_unref(option->values);
  }
  /* g_free(option->entries); */
  g_free(option);
}

/* bool option_parse(Option* option, int* argc, char*** argv) */
/* { */
/*   if (option->option_context) { */
/*     g_message("option context is already registerd."); */
/*     return false; */
/*   } */
/*   GError* error = NULL; */
/*   option->option_context = g_option_context_new(NULL); */
/*   #<{(| g_option_context_set_help_enabled(option_context, FALSE); |)}># */
/*   g_option_context_add_main_entries(option->option_context, option->entries, NULL); */
/*   g_option_context_parse(option->option_context, argc, argv, &error); */
/*   if (error) { */
/*     g_error("%s", error->message); */
/*     g_error_free(error); */
/*     return false; */
/*   } */
/*   return true; */
/* } */

void option_set_values(Option* option, GVariantDict* values)
{
  option->values = g_variant_dict_ref(values);
}

/* void* option_get(Option* option, const char* key) */
/* { */
/*   int i = 0; */
/*  */
/*   GOptionEntry* e = NULL; */
/*   while (&option->entries[i]) { */
/*     e = &option->entries[i]; */
/*     if (is_equal(key, e->long_name)) { */
/*       return e->arg_data; */
/*     } */
/*     i += 1; */
/*   } */
/*   return NULL; */
/* } */

bool option_get_str_value(Option* option, const char* key, const char** value)
{
  if (!option->values) {
    return false;
  }
  char* v;
  bool has_value = g_variant_dict_lookup(option->values, key, "s", &v);
  if (has_value) {
    *value = v;
  }
  return has_value;
}

bool option_get_int_value(Option* option, const char* key, int* value)
{
  if (!option->values) {
    return false;
  }
  int v;
  bool has_value = g_variant_dict_lookup(option->values, key, "i", &v);
  if (has_value) {
    *value = v;
  }
  return has_value;
}

bool option_get_bool_value(Option* option, const char* key, bool* value)
{
  if (!option->values) {
    return false;
  }
  gboolean v;
  bool has_value = g_variant_dict_lookup(option->values, key, "b", &v);
  if (has_value) {
    *value = v;
  }
  return has_value;
}

/* bool option_get_str_value(Option* option, const char* key, const char** value) */
/* { */
/*   void* p = option_get(option, key); */
/*   if (!p) { */
/*     return false; */
/*   } */
/*   if (!(char**)p) { */
/*     return false; */
/*   } */
/*   *value = *(char**)p; */
/*   return true; */
/* } */
/*  */
/* bool option_get_int_value(Option* option, const char* key, int* value) */
/* { */
/*   void* p = option_get(option, key); */
/*   if (!p) { */
/*     return false; */
/*   } */
/*   *value = *(int*)p; */
/*   return true; */
/* } */
/*  */
/* bool option_get_bool_value(Option* option, const char* key, bool* value) */
/* { */
/*   void* p = option_get(option, key); */
/*   if (!p) { */
/*     return false; */
/*   } */
/*   *value = *(bool*)p; */
/*   return true; */
/* } */

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
