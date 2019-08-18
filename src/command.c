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
}

void command_reload_theme(Context* context)
{
  context_load_theme(context);
}

void command_copy_clipboard(Context* context)
{
#ifdef TYM_USE_VTE_COPY_CLIPBOARD_FORMAT
  vte_terminal_copy_clipboard_format(context->layout->vte, VTE_FORMAT_TEXT);
#else
  vte_terminal_copy_clipboard(context->layout->vte);
#endif
}

void command_paste_clipboard(Context* context)
{
  vte_terminal_paste_clipboard(context->layout->vte);
}
