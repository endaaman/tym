/**
 * meta.h
 *
 * Copyright (c) 2019 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef META_H
#define META_H

#include "common.h"

typedef void  (*MetaCallback) (void);

typedef enum {
  META_ENTRY_TYPE_STRING = 0,
  META_ENTRY_TYPE_INTEGER = 1,
  META_ENTRY_TYPE_BOOLEAN = 2,
  META_ENTRY_TYPE_NONE = 3, // not actual, only shown in help
} MetaEntryType;

typedef struct {
  char* name;
  char short_name;
  MetaEntryType type;
  GOptionFlags option_flag;
  void* default_value;
  char* arg_desc;
  char* desc;
  MetaCallback getter;
  MetaCallback setter;
  unsigned index;
} MetaEntry;

typedef struct {
  GHashTable* data;
  GList* list;
} Meta;

Meta* meta_init();
void meta_close(Meta* meta);
unsigned meta_size(Meta* meta);
MetaEntry* meta_get_entry(Meta* meta, const char* key);

#endif
