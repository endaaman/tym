/**
 * config.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "config.h"


Config* config_init()
{
  Config* config = g_new(Config, 1);
  config->data = g_hash_table_new_full(
    g_str_hash,
    g_str_equal,
    (GDestroyNotify)g_free,
    (GDestroyNotify)g_free
  );
  config->locked = true;
  return config;
}

void config_close(Config* config)
{
  g_hash_table_destroy(config->data);
  g_free(config);
}

static void* config_get_raw(Config* config, const char* key)
{
  void* ptr = g_hash_table_lookup(config->data, key);
  if (!ptr) {
    dd("tried to refer null field: '%s'", key);
  }
  return ptr;
}

static void config_set_raw(Config* config, const char* key, void* value)
{
  if (!value) {
    dd("tried to set null field: '%s'", key);
    return;
  }
  void* old_key = NULL;
  bool has_value = g_hash_table_lookup_extended(config->data, key, &old_key, NULL);
  // warn if: not reseting and attempt to insert value
  if (config->locked && !has_value) {
    dd("tried to add new field when locked: '%s'", key);
    return;
  }
  if (old_key) {
    g_hash_table_remove(config->data, old_key);
  }
  g_hash_table_insert(config->data, g_strdup(key), value);
  return;
}

void config_set_str(Config* config, const char* key, const char* value)
{
  config_set_raw(config, key, g_strdup(value));
}

const char* config_get_str(Config* config, const char* key)
{
  char* v = (char*)config_get_raw(config, key);
  if (v) {
    return v;
  }
  dd("string config of '%s' is null. falling back to \"\"", key);
  return "";
}

int config_get_int(Config* config, const char* key)
{
  int* v = (int*)config_get_raw(config, key);
  if (v) {
    return *v;
  }
  dd("int config of '%s' is null. falling back to 0", key);
  return 0;
}

void config_set_int(Config* config, const char* key, int value)
{
  config_set_raw(config, key, memdup((gpointer)&value, sizeof(int)));
}

bool config_get_bool(Config* config, const char* key)
{
  bool* v = (bool*)config_get_raw(config, key);
  if (v) {
    return *v;
  }
  dd("bool config of '%s' is null. falling back to null", key);
  return false;
}

void config_set_bool(Config* config, const char* key, bool value)
{
  config_set_raw(config, key, memdup((gpointer)&value, sizeof(bool)));
}

void config_restore_default(Config* config, Meta* meta)
{
  df();
  config->locked = false;
  g_hash_table_remove_all(config->data);

  for (GList* li = meta->list; li != NULL; li = li->next) {
    MetaEntry* e = (MetaEntry*)li->data;
    if (e->getter) {
      // if getter exists, do not save value in hash
      continue;
    }
    switch (e->type) {
      case META_ENTRY_TYPE_STRING:
        config_set_str(config, e->name, e->default_value);
        break;
      case META_ENTRY_TYPE_INTEGER:
        config_set_int(config, e->name, *(int*)(e->default_value));
        break;
      case META_ENTRY_TYPE_BOOLEAN:
        config_set_bool(config, e->name, *(bool*)(e->default_value));
        break;
      case META_ENTRY_TYPE_NONE:
        break;
    }
  }
  config->locked = true;
}
