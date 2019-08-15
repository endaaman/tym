/**
 * command.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "command.h"


void command_reload(Context* context)
{
  context_load_config(context);
  context_load_theme(context);
  context_apply_config(context);
  context_apply_theme(context);
}

void command_reload_theme(Context* context)
{
  context_load_theme(context);
  context_apply_theme(context);
}

void command_copy_clipboard(Context* context)
{
#ifdef TYM_USE_VTE_COPY_CLIPBOARD_FORMAT
  vte_terminal_copy_clipboard_format(context_get_vte(context), VTE_FORMAT_TEXT);
#else
  vte_terminal_copy_clipboard(context_get_vte(context));
#endif
}

void command_paste_clipboard(Context* context)
{
  vte_terminal_paste_clipboard(context_get_vte(context));
}

void command_increase_font_scale(Context* context)
{
  VteTerminal* vte = context_get_vte(context);

  double scale = vte_terminal_get_font_scale(vte) + 0.1;
  vte_terminal_set_font_scale(vte, scale);
}

void command_decrease_font_scale(Context* context)
{
  VteTerminal* vte = context_get_vte(context);

  double scale = vte_terminal_get_font_scale(vte) - 0.1;
  vte_terminal_set_font_scale(vte, scale);
}

void command_reset_font_scale(Context* context)
{
  vte_terminal_set_font_scale(context_get_vte(context), 1.0);
}
