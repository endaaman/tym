/**
 * tym.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <stdlib.h>
#include <stdbool.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <vte/vte.h>

#include "config.h"
#include "version.h"


#define UNUSED(x) (void)(x)

#if VTE_MAJOR_VERSION == 0
#if VTE_MINOR_VERSION >= 48
#define USE_ASYNC_SPAWN
#endif
#endif


static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
  VteTerminal* vte = VTE_TERMINAL(widget);
  GHashTable* config = *((GHashTable **)user_data);

  const unsigned mod = event->state & gtk_accelerator_get_default_mod_mask();
  double scale;
  if (mod & GDK_CONTROL_MASK) {
    switch (gdk_keyval_to_lower(event->keyval)) {
      case GDK_KEY_minus:
        scale = vte_terminal_get_font_scale(vte) - 0.1;
        vte_terminal_set_font_scale(vte, scale);
        return true;
      case GDK_KEY_plus:
        scale = vte_terminal_get_font_scale(vte) + 0.1;
        vte_terminal_set_font_scale(vte, scale);
        return true;
      case GDK_KEY_equal:
        vte_terminal_set_font_scale(vte, 1.0);
        return true;
    }
  }

  if ((mod & GDK_CONTROL_MASK) && (mod & GDK_SHIFT_MASK)) {
    switch (gdk_keyval_to_lower(event->keyval)) {
      case GDK_KEY_c:
        vte_terminal_copy_clipboard(vte);
        vte_terminal_unselect_all(vte);
        return true;
      case GDK_KEY_v:
        vte_terminal_paste_clipboard(vte);
        return true;
      case GDK_KEY_r:
        config_load(config);
        config_apply_all(config, vte, false);
        return true;
    }
  }
  return false;
}

#ifdef USE_ASYNC_SPAWN
static void spawn_callback(VteTerminal *terminal, GPid pid, GError *error, gpointer user_data) {
  UNUSED(terminal);
  UNUSED(pid);
  UNUSED(user_data);

  if (!error) {
    return;
  }
  g_printerr("warining: %s\n", error->message);
}
#endif

static void start(GHashTable* c) {
  // setup window
  GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_icon_name(GTK_WINDOW(window), "terminal");
  gtk_container_set_border_width(GTK_CONTAINER(window), 0);

  g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

  // setup vte widget
  GtkWidget* vte_widget = vte_terminal_new();
  gtk_container_add(GTK_CONTAINER(window), vte_widget);

  VteTerminal* vte = VTE_TERMINAL(vte_widget);
  vte_terminal_set_rewrap_on_resize(vte, true);
  g_signal_connect(G_OBJECT(vte), "child-exited", G_CALLBACK(gtk_main_quit), NULL);
  g_signal_connect(G_OBJECT(vte), "key-press-event", G_CALLBACK(on_key_press), &c);

  config_apply_all(c, vte, true);

  char* argv[] = {config_get_str(c, "shell"), NULL};
  char** env = g_get_environ();

#ifdef USE_ASYNC_SPAWN
  vte_terminal_spawn_async(
    vte,                 // terminal
    VTE_PTY_DEFAULT,     // pty flag
    NULL,                // working directory
    argv,                // argv
    env,                 // envv
    G_SPAWN_SEARCH_PATH, // spawn_flags
    NULL,                // child_setup
    NULL,                // child_setup_data
    NULL,                // child_setup_data_destroy
    1000,                // timeout
    NULL,                // cancel callback
    spawn_callback,      // callback
    NULL                 // user_data
  );
#else
  GError* error = NULL;
  GPid child_pid;
  vte_terminal_spawn_sync(
    vte,
    VTE_PTY_DEFAULT,
    NULL,
    argv,
    env,
    G_SPAWN_SEARCH_PATH,
    NULL,
    NULL,
    &child_pid,
    NULL,
    &error
  );

  if (error) {
    g_printerr("%s\n", error->message);
    g_error_free(error);
    return;
  }
#endif
  g_strfreev(env);

  gtk_widget_grab_focus(vte_widget);
  gtk_widget_show_all(window);
  gtk_main();
}

int main(int argc, char* argv[])
{
  bool version = false;
  GOptionEntry entries[] = {
    { "version", 'v', 0, G_OPTION_ARG_NONE, &version, "Show Version", NULL },
    { NULL }
  };
  GOptionContext* context = g_option_context_new("");
  GError*error = NULL;
  g_option_context_add_main_entries(context, entries, NULL);
  if (!g_option_context_parse(context, &argc, &argv, &error)) {
    g_printerr ("option parsing failed: %s\n", error->message);
    return 1;
  }
  if (version) {
    g_print ("version %s\n", TYM_VERSION);
    return 0;
  }

  GHashTable *config = config_init();
  config_load(config);

  gtk_init(&argc, &argv);
  start(config);

  config_close(config);
  return EXIT_SUCCESS;
}
