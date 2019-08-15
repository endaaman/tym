/**
 * meta.c
 *
 * Copyright (c) 2019 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "meta.h"


#define TYM_FALL_BACK_SHELL "/bin/sh"

static const char* TYM_DEFAULT_TITLE = "tym";
static const char* TYM_DEFAULT_ICON = "utilities-terminal";
static const char* TYM_DEFAULT_TERM = "xterm-256color";
static const char* TYM_DEFAULT_CURSOR_SHAPE = TYM_CURSOR_SHAPE_BLOCK;
static const char* TYM_DEFAULT_CURSOR_BLINK_MODE = TYM_CURSOR_BLINK_MODE_SYSTEM;
static const char* TYM_DEFAULT_CJK = TYM_CJK_WIDTH_NARROW;
static const int TYM_DEFAULT_WIDTH = 80;
static const int TYM_DEFAULT_HEIGHT = 22;
static const int TYM_DEFAULT_SCROLLBACK = 512;

static char* get_default_shell()
{
  const char* shell_env = g_getenv("SHELL");
  if (shell_env) {
    return g_strdup(shell_env);
  }
  char* user_shell = vte_get_user_shell();
  if (user_shell) {
    return user_shell;
  }
  return g_strdup(TYM_FALL_BACK_SHELL);
}

static void free_entry(void* data) {
  g_free(((MetaEntry*)data)->default_value);
  g_free(data);
}

__attribute__((constructor))
Meta* meta_init() {
  Meta* meta = g_malloc0(sizeof(Meta));

  char* default_theme_path = g_build_path(
    G_DIR_SEPARATOR_S,
    g_get_user_config_dir(),
    TYM_CONFIG_DIR_NAME,
    TYM_THEME_FILE_NAME,
    NULL
  );
#define color_special(name) { ("color_" name), 0, T_STR, F_N, sdup(""), "", ("value of color_" name ), }
#define color_normal(name) { ("color_" name), 0, T_STR, F_HIDDEN, sdup(""), NULL, NULL, }
  const MetaEntryType T_STR = META_ENTRY_TYPE_STRING;
  const MetaEntryType T_INT = META_ENTRY_TYPE_INTEGER;
  const MetaEntryType T_BOOL = META_ENTRY_TYPE_BOOLEAN;
  const MetaEntryType T_NONE = META_ENTRY_TYPE_NONE;
  const GOptionFlags F_N = G_OPTION_FLAG_NONE;
  const GOptionFlags F_HIDDEN = G_OPTION_FLAG_HIDDEN;

  char* (*sdup)(const char*) = g_strdup;
  void* (*mdup)(const void*, unsigned) = g_memdup;
  const bool v_false = false;
  const int v_zero = 0;

  // name, short, type, group, flag, default, desc
  MetaEntry c[] = {
    { "theme",               't', T_STR,  F_N, default_theme_path, "<path>", "<path> to theme file. Set '" TYM_SYMBOL_NONE "' to start without loading theme.", },
    { "shell",               'e', T_STR,  F_N, get_default_shell(), "<shell path>", "Shell to be used", },
    { "title",                 0, T_STR,  F_N, sdup(TYM_DEFAULT_TITLE), "", "Window title", },
    { "font",                  0, T_STR,  F_N, sdup(""), "", "Font to render(e.g. 'Ubuntu Mono 12')", },
    { "icon",                  0, T_STR,  F_N, sdup(TYM_DEFAULT_ICON), "", "Name of icon", },
    { "cursor_shape",          0, T_STR,  F_N, sdup(TYM_DEFAULT_CURSOR_SHAPE), "", "'block', 'ibeam' or 'underline'", },
    { "cursor_blink_mode",     0, T_STR,  F_N, sdup(TYM_DEFAULT_CURSOR_BLINK_MODE), "", "'system', 'on' or 'off'", },
    { "term",                  0, T_STR,  F_N, sdup(TYM_DEFAULT_TERM), "$TERM", "Value to override $TERM", },
    { "role",                  0, T_STR,  F_N, sdup(""), "", "Unique identifier for the window", },
    { "cjk_width",             0, T_STR,  F_N, sdup(TYM_DEFAULT_CJK), "", "'narrow' or 'wide'", },
    { "width",                 0, T_INT,  F_N, mdup(&TYM_DEFAULT_WIDTH, sizeof(int)), "<int>", "Initial columns", },
    { "height",                0, T_INT,  F_N, mdup(&TYM_DEFAULT_HEIGHT, sizeof(int)), "<int>", "Initial rows", },
    { "padding_horizontal",    0, T_INT,  F_N, mdup(&v_zero, sizeof(int)), "<int>", "Horizontal padding", },
    { "padding_vertical",      0, T_INT,  F_N, mdup(&v_zero, sizeof(int)), "<int>", "Vertical padding", },
    { "scrollback_length",     0, T_INT,  F_N, mdup(&TYM_DEFAULT_SCROLLBACK, sizeof(int)), "<int>", "Scrollback buffer length", },
    { "ignore_default_keymap", 0, T_BOOL, F_N, mdup(&v_false, sizeof(bool)), NULL, "Whether to use default keymap", },
    { "ignore_bold",           0, T_BOOL, F_N, mdup(&v_false, sizeof(bool)), NULL, "Whether to ignore bold style", },
    { "autohide",              0, T_BOOL, F_N, mdup(&v_false, sizeof(bool)), NULL, "Whether to hide mouse cursor when key is pressed", },
    { "silent",                0, T_BOOL, F_N, mdup(&v_false, sizeof(bool)), NULL, "Whether to beep when bell sequence is sent", },
    color_special("window_background"),
    color_special("foreground"),
    color_special("background"),
    color_special("cursor"),
    color_special("cursor_foreground"),
    color_special("highlight"),
    color_special("highlight_foreground"),
    color_special("bold"),
    color_normal("0"),  color_normal("1"),  color_normal("2"),  color_normal("3"),
    color_normal("4"),  color_normal("5"),  color_normal("6"),  color_normal("7"),
    color_normal("8"),  color_normal("9"),  color_normal("10"), color_normal("11"),
    color_normal("12"), color_normal("13"), color_normal("14"), color_normal("15"),
    { "color_1..15", 0, T_NONE, F_N, NULL, "", "value of color from color_1 to color_15", },
  };
#undef color_special
#undef color_normal

  meta->data = g_hash_table_new_full(
    g_str_hash,
    g_str_equal,
    NULL,
    (GDestroyNotify)free_entry
  );
  unsigned i = 0;
  unsigned len = sizeof(c) / sizeof(MetaEntry);
  while (i < len) {
    MetaEntry* entry = (MetaEntry*)g_memdup(&c[i], sizeof(c[i]));
    entry->index = i;
    g_hash_table_insert(meta->data, entry->name, entry);
    i++;
  }
  return meta;
}

void meta_close(Meta* meta) {
  g_hash_table_destroy(meta->data);
  g_free(meta);
}

int entries_sort_func(const void* a, const void* b)
{
  return ((MetaEntry*)a)->index - ((MetaEntry*)b)->index;
}

GList* meta_as_list(Meta* meta, bool sorted)
{
  GHashTableIter iter;
  g_hash_table_iter_init(&iter, meta->data);
  GList* ee = NULL;
  char* key = NULL;
  MetaEntry* entry = NULL;
  while (g_hash_table_iter_next(&iter, (void*)&key, (void*)&entry)) {
    ee = sorted
      ? g_list_insert_sorted(ee, entry, entries_sort_func)
      : g_list_append(ee, entry);
  }
  return ee;
}

unsigned meta_size(Meta* meta)
{
  return g_hash_table_size(meta->data);
}

MetaEntry* meta_get_entry(Meta* meta, const char* key)
{
  MetaEntry* entry = (MetaEntry*)g_hash_table_lookup(meta->data, key);
  if (!entry) {
    return NULL;
  }
  MetaEntryType t= entry->type;
  if (t == META_ENTRY_TYPE_STRING || t == META_ENTRY_TYPE_INTEGER || t == META_ENTRY_TYPE_BOOLEAN) {
    return entry;
  }
  return NULL;
}

void meta_iter_init(MetaIter* iter, Meta* meta)
{
  g_hash_table_iter_init(&iter->iter, meta->data);
}

bool meta_iter_next(MetaIter* iter, char** key, MetaEntry** entry)
{
  return g_hash_table_iter_next(&iter->iter, (void*)key, (void*)entry);
}
