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
  return option;
}

void option_close(Option* option)
{
  if (option->values) {
    g_variant_dict_unref(option->values);
  }
  g_free(option);
}

void option_set_values(Option* option, GVariantDict* values)
{
  option->values = g_variant_dict_ref(values);
}

bool option_get_str_value(Option* option, const char* key, char** value)
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

char* option_get_str(Option* option, const char* key)
{
  char* v = NULL;
  if (option_get_str_value(option, key, &v)) {
    return NULL;
  }
  return v;
}

int option_get_int(Option* option, const char* key)
{
  int v = 0;
  if (option_get_int_value(option, key, &v)) {
    return 0;
  }
  return v;
}

bool option_get_bool(Option* option, const char* key)
{
  bool v = 0;
  if (option_get_bool_value(option, key, &v)) {
    return 0;
  }
  return v;
}
