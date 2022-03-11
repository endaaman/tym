/**
 * option.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "option.h"


Option* option_init(GOptionEntry* entries)
{
  df();
  Option* option = g_malloc0(sizeof(Option));

  option->entries = entries;
  option->option_context = g_option_context_new("tym command line");
  g_option_context_add_main_entries(option->option_context, option->entries, NULL);
  /* g_option_context_set_help_enabled(option->option_context, false); */
  /* g_option_context_add_group(option->option_context, gtk_get_option_group(TRUE)); */

  option->entries_as_table = g_hash_table_new_full(
    g_str_hash,
    g_str_equal,
    NULL,
    NULL
  );
  return option;
}

void option_close(Option* option)
{
  g_option_context_free(option->option_context);
  if (option->entries) {
    GOptionEntry* e = &option->entries[0];
    while (e->long_name) {
      if (e->arg_data) {
        g_free(e->arg_data);
      }
      e++;
    };
    g_free(option->entries);
  }
  if (option->entries_as_table) {
    g_hash_table_destroy(option->entries_as_table);
  }
  g_free(option);
}

bool option_parse(Option* option, int argc, char** argv)
{
  df();
  g_assert(option->entries);
  g_assert(option->entries_as_table);

  GError* error = NULL;

  char** argv_strv = g_new0(char*, argc + 1);
  int i = 0;
  while (i < argc) {
    argv_strv[i] = g_strdup(argv[i]);
    i++;
  }
  g_option_context_parse_strv(option->option_context, &argv_strv, &error);
  char** a = &argv_strv[0];
  while (*a) {
    g_free(*a);
    a++;
  }
  g_free(argv_strv);

  if (error) {
    g_warning("%s", error->message);
    return false;
  }

  GOptionEntry* e = &option->entries[0];
  while (e->long_name) {
    g_hash_table_insert(option->entries_as_table, (void*)e->long_name, e);
    e++;
  };

  return true;
}

void* option_get_pointer(Option* option, const char* key)
{
  g_assert(option->entries);
  GOptionEntry* e = (GOptionEntry*)g_hash_table_lookup(option->entries_as_table, key);
  g_assert(e);
  return e->arg_data;
}

char* option_get_str(Option* option, const char* key)
{
  char** p = (char**)option_get_pointer(option, key);
  if (!p) {
    return false;
  }
  return *p;
}

int option_get_int(Option* option, const char* key)
{
  int* p = (int*)option_get_pointer(option, key);
  if (!p) {
    return false;
  }
  return *p;
}

bool option_get_bool(Option* option, const char* key)
{
  gboolean* p = (gboolean*)option_get_pointer(option, key);
  if (!p) {
    return false;
  }
  return *p;
}
