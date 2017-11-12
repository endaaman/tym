/**
 * command.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "command.h"


static const char* DEFAULT_NOTIFICATION_TITLE = "tym";

void command_notify(Context* context, const char* body, const char* title)
{
  dd("command_notify");

  GtkApplication* app = context->app;

  GNotification* notification = g_notification_new(title ? title : DEFAULT_NOTIFICATION_TITLE);
  GIcon* icon = g_themed_icon_new("terminal");

  g_notification_set_icon (notification, G_ICON (icon));
  g_notification_set_body(notification, body);
  g_application_send_notification(G_APPLICATION(app), "lunch-is-ready", notification);

  g_object_unref(notification);
  g_object_unref(icon);
}

void command_reload(Context* context)
{
  dd("command_reload");
  context_load(context);
}

void command_copy_clipboard(Context* context)
{
  dd("command_copy_clipboard");
#ifdef USE_COPY_CLIPBOARD_FORMAT
  vte_terminal_copy_clipboard_format(context->vte, VTE_FORMAT_TEXT);
#else
  vte_terminal_copy_clipboard(context->vte);
#endif

  vte_terminal_unselect_all(context->vte);
}

void command_paste_clipboard(Context* context)
{
  dd("command_paste_clipboard");
  vte_terminal_paste_clipboard(context->vte);
}

void command_increase_font_scale(Context* context)
{
  dd("command_increase_font_scale");
  VteTerminal* vte = context->vte;

  double scale = vte_terminal_get_font_scale(vte) + 0.1;
  vte_terminal_set_font_scale(vte, scale);
}

void command_decrease_font_scale(Context* context)
{
  dd("command_decrease_font_scale");
  VteTerminal* vte = context->vte;

  double scale = vte_terminal_get_font_scale(vte) - 0.1;
  vte_terminal_set_font_scale(vte, scale);
}

void command_reset_font_scale(Context* context)
{
  dd("command_reset_font_scale");
  vte_terminal_set_font_scale(context->vte, 1.0);
}
