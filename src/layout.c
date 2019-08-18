/**
 * layout.c
 *
 * Copyright (c) 2019 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "layout.h"
#include "regex.h"


typedef void (*VteSetColorFunc)(VteTerminal*, const GdkRGBA*);


Layout* layout_init()
{
  Layout* layout = g_malloc0(sizeof(Layout));
  return layout;
}

void layout_close(Layout* layout)
{
  if (layout->uri_tag) {
    g_free(layout->uri_tag);
  }
  g_free(layout);
}

void layout_build(Layout* layout, GApplication* app)
{
  GtkWindow* window = layout->window = GTK_WINDOW(gtk_application_window_new(GTK_APPLICATION(app)));
  VteTerminal* vte = layout->vte = VTE_TERMINAL(vte_terminal_new());
  GtkBox* hbox = layout->hbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
  GtkBox* vbox = layout->vbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));

  gtk_container_add(GTK_CONTAINER(hbox), GTK_WIDGET(vte));
  gtk_container_add(GTK_CONTAINER(vbox), GTK_WIDGET(hbox));
  gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(vbox));
  gtk_container_set_border_width(GTK_CONTAINER(window), 0);

  GError* error = NULL;
  VteRegex* regex = vte_regex_new_for_match(IRI, -1, PCRE2_UTF | PCRE2_MULTILINE | PCRE2_CASELESS, &error);
  if (error) {
    g_warning("Error when parsing css: %s", error->message);
    g_error_free(error);
  } else {
    int tag = vte_terminal_match_add_regex(vte, regex, 0);
    layout->uri_tag = g_malloc0(sizeof(int));
    *layout->uri_tag = tag;
    vte_terminal_match_set_cursor_name(vte, tag, "hand");
    vte_regex_unref(regex);
  }

  GdkScreen* screen = gtk_widget_get_screen(GTK_WIDGET(window));
  GdkVisual* visual = gdk_screen_get_rgba_visual(screen);
  layout->alpha_supported = visual;
  if (!layout->alpha_supported) {
    g_message("Your screen does not support alpha channel.");
    visual = gdk_screen_get_system_visual(screen);
  }
  gtk_widget_set_visual(GTK_WIDGET(window), visual);
}
