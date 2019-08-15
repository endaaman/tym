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
  unsigned index;
} MetaEntry;

typedef struct {
  GHashTable* data;
} Meta;

typedef struct {
  GHashTableIter iter;
} MetaIter;


Meta* meta_init();
void meta_close(Meta* meta);
GList* meta_as_list(Meta* meta, bool sorted);
unsigned meta_size(Meta* meta);
MetaEntry* meta_get_entry(Meta* meta, const char* key);

void meta_iter_init(MetaIter* iter, Meta* meta);
bool meta_iter_next(MetaIter* iter, char** key, MetaEntry** entry);

#endif
