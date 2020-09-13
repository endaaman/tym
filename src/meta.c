/**
 * meta.c
 *
 * Copyright (c) 2019 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "meta.h"
#include "property.h"


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

static void free_entry(void* data)
{
  g_free(((MetaEntry*)data)->default_value);
  g_free(data);
}

int entries_sort_func(const void* a, const void* b)
{
  return ((MetaEntry*)a)->index - ((MetaEntry*)b)->index;
}

Meta* meta_init()
{
  Meta* meta = g_malloc0(sizeof(Meta));
#define	CB(f) ((MetaCallback) (f))
#define color_special(key, default_color) \
  { \
    .name=("color_"#key ), .default_value=sdup(default_color), .arg_desc="", .desc=("value of color_"#key), \
     .is_theme=true, .setter=CB(setter_color_##key) \
  }
#define color_normal(i) \
  { \
    .name=("color_"#i ), .default_value=sdup(TYM_DEFAULT_COLOR_##i), .arg_desc="", .desc=("value of color_"#i), \
    .option_flag=G_OPTION_FLAG_HIDDEN, .is_theme=true, .setter=CB(setter_color_normal) \
  }
  const MetaEntryType T_INT = META_ENTRY_TYPE_INTEGER;
  const MetaEntryType T_BOOL = META_ENTRY_TYPE_BOOLEAN;
  const MetaEntryType T_NONE = META_ENTRY_TYPE_NONE;

  char* (*sdup)(const char*) = g_strdup;
  void* (*mdup)(const void*, unsigned) = g_memdup;
  const bool v_false = false;
  const int v_zero = 0;

  MetaEntry ee[] = {
    // STR
    {
      .name="shell",  .short_name='e', .default_value=get_default_shell(), .arg_desc="<shell>",
      .desc="Shell to use in the terminal",
      .setter=CB(setter_shell)
    },
    {
      .name="term", .default_value=sdup(TYM_DEFAULT_TERM), .arg_desc="", .desc="Value to override $TERM",
      .setter=CB(setter_term)
    },
    {
      .name="title", .default_value=sdup(TYM_DEFAULT_TITLE), .arg_desc="", .desc="Window title",
      .getter=CB(getter_title), .setter=CB(setter_title)
    },
    {
      .name="font", .default_value=sdup(""), .arg_desc="", .desc="Font to render(e.g. 'Ubuntu Mono 12')",
      .setter=CB(setter_font)
    },
    {
      .name="icon", .default_value=sdup(TYM_DEFAULT_ICON), .arg_desc="", .desc="Name of window icon",
      .setter=CB(setter_icon)
    },
    {
      .name="role", .default_value=sdup(""), .arg_desc="",
      .desc="Unique identifier for the window",
      .getter=CB(getter_role), .setter=CB(setter_role),
    },
    {
      .name="cursor_shape", .default_value=sdup(TYM_DEFAULT_CURSOR_SHAPE), .arg_desc="",
      .desc="'" TYM_CURSOR_SHAPE_BLOCK "', '" TYM_CURSOR_SHAPE_IBEAM "' or '" TYM_CURSOR_SHAPE_UNDERLINE "'",
      .getter=CB(getter_cursor_shape), .setter=CB(setter_cursor_shape),
    },
    {
      .name="cursor_blink_mode", .default_value=sdup(TYM_DEFAULT_CURSOR_BLINK_MODE), .arg_desc="",
      .desc="'" TYM_CURSOR_BLINK_MODE_SYSTEM "', '" TYM_CURSOR_BLINK_MODE_ON "' or '" TYM_CURSOR_BLINK_MODE_OFF "'",
      .getter=CB(getter_cursor_blink_mode), .setter=CB(setter_cursor_blink_mode),
    },
    {
      .name="cjk_width", .arg_desc="", .default_value=sdup(TYM_DEFAULT_CJK),
      .desc="'" TYM_CJK_WIDTH_NARROW "' or '" TYM_CJK_WIDTH_WIDE "'",
      .getter=CB(getter_cjk_width), .setter=CB(setter_cjk_width),
    },
    {
      .name="background_image", .arg_desc="", .default_value=sdup(""),
      .desc="path to background image",
      .setter=CB(setter_background_image),
    },
    {
      .name="uri_schemes", .arg_desc="", .default_value=sdup(TYM_DEFAULT_URI_SCHEMES),
      .desc="URI schemes to be highlighted and clickable",
      .setter=CB(setter_uri_schemes),
    },
    // INT
    {
      .name="width", .type=T_INT, .default_value=mdup(&TYM_DEFAULT_WIDTH, sizeof(int)),
      .arg_desc="<int>", .desc="Initial columns",
      .getter=CB(getter_width), .setter=CB(setter_width)
    },
    {
      .name="height", .type=T_INT, .default_value=mdup(&TYM_DEFAULT_HEIGHT, sizeof(int)),
      .arg_desc="<int>", .desc="Initial rows",
      .getter=CB(getter_height), .setter=CB(setter_height)
    },
    {
      .name="scale", .type=T_INT, .default_value=mdup(&TYM_DEFAULT_SCALE, sizeof(int)),
      .arg_desc="<int>", .desc="Font scale in percent",
      .getter=CB(getter_scale), .setter=CB(setter_scale)
    },
    {
      .name="padding_horizontal", .type=T_INT, .default_value=mdup(&v_zero, sizeof(int)),
      .arg_desc="<int>", .desc="Horizontal padding",
      .setter=CB(setter_padding_horizontal)
    },
    {
      .name="padding_vertical", .type=T_INT, .default_value=mdup(&v_zero, sizeof(int)),
      .arg_desc="<int>", .desc="Vertical padding",
      .setter=CB(setter_padding_vertical)
    },
    {
      .name="scrollback_length", .type=T_INT, .default_value=mdup(&TYM_DEFAULT_SCROLLBACK, sizeof(int)),
      .arg_desc="<int>", .desc="Scrollback buffer length",
      .getter=CB(getter_scrollback_length), .setter=CB(setter_scrollback_length)
    },
    // BOOL
    {
      .name="ignore_default_keymap", .type=T_BOOL, .default_value=mdup(&v_false, sizeof(bool)),
      .desc="Whether to use default keymap",
    },
    {
      .name="autohide", .type=T_BOOL, .default_value=mdup(&v_false, sizeof(bool)),
      .desc="Whether to hide mouse cursor when key is pressed",
      .getter=CB(getter_autohide), .setter=CB(setter_autohide)
    },
    {
      .name="silent", .type=T_BOOL, .default_value=mdup(&v_false, sizeof(bool)),
      .desc="Whether to beep when bell sequence is sent",
      .getter=CB(getter_silent), .setter=CB(setter_silent),
    },
    color_normal(0),  color_normal(1),  color_normal(2),  color_normal(3),
    color_normal(4),  color_normal(5),  color_normal(6),  color_normal(7),
    color_normal(8),  color_normal(9),  color_normal(10), color_normal(11),
    color_normal(12), color_normal(13), color_normal(14), color_normal(15),
    color_special(window_background, ""),
    color_special(background, TYM_DEFAULT_COLOR_BACKGROUND),
    color_special(foreground, TYM_DEFAULT_COLOR_FOREGROUND),
    color_special(bold, TYM_DEFAULT_COLOR_FOREGROUND),
    color_special(cursor, TYM_DEFAULT_COLOR_FOREGROUND),
    color_special(cursor_foreground, TYM_DEFAULT_COLOR_BACKGROUND),
    color_special(highlight, TYM_DEFAULT_COLOR_FOREGROUND),
    color_special(highlight_foreground, TYM_DEFAULT_COLOR_BACKGROUND),
    {
      .name="color_0..15", .type=T_NONE, .arg_desc="", .desc="value of color_0 .. color_15",
    },
  };
#undef CB
#undef get_default_color
#undef color_special
#undef color_normal

  meta->data = g_hash_table_new_full(
    g_str_hash,
    g_str_equal,
    NULL,
    (GDestroyNotify)free_entry
  );
  unsigned i = 0;
  unsigned len = sizeof(ee) / sizeof(MetaEntry);
  while (i < len) {
    MetaEntry* entry = (MetaEntry*)g_memdup(&ee[i], sizeof(ee[i]));
    if (entry->getter && !entry->setter) {
      dw("Invalid meta `%s`: setter is provided but getter is not provided.", entry->name);
    }
    entry->index = i;
    g_hash_table_insert(meta->data, entry->name, entry);
    meta->list = g_list_insert_sorted(meta->list, entry, entries_sort_func);
    i++;
  }
  return meta;
}

void meta_close(Meta* meta)
{
  g_hash_table_destroy(meta->data);
  g_list_free(meta->list);
  g_free(meta);
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
  MetaEntryType t = entry->type;
  if (t == META_ENTRY_TYPE_STRING || t == META_ENTRY_TYPE_INTEGER || t == META_ENTRY_TYPE_BOOLEAN) {
    return entry;
  }
  return NULL;
}
