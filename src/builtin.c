/**
 * context.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "builtin.h"


void builtin_reload(Context* context)
{
  dd("builtin_reload");
  context_load_config(context, false);
}

void builtin_copy_clipboard(Context* context)
{
  dd("builtin_copy_clipboard");
  vte_terminal_copy_clipboard(context->vte);
  vte_terminal_unselect_all(context->vte);
}

void builtin_paste_clipboard(Context* context)
{
  dd("builtin_paste_clipboard");
  vte_terminal_paste_clipboard(context->vte);
}

void builtin_increase_font_scale(Context* context)
{
  dd("builtin_increase_font_scale");
  VteTerminal* vte = context->vte;

  double scale = vte_terminal_get_font_scale(vte) + 0.1;
  vte_terminal_set_font_scale(vte, scale);
}

void builtin_decrease_font_scale(Context* context)
{
  dd("builtin_decrease_font_scale");
  VteTerminal* vte = context->vte;

  double scale = vte_terminal_get_font_scale(vte) - 0.1;
  vte_terminal_set_font_scale(vte, scale);
}

void builtin_reset_font_scale(Context* context)
{
  dd("builtin_reset_font_scale");
  vte_terminal_set_font_scale(context->vte, 1.0);
}
