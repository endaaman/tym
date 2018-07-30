/**
 * tym.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "common.h"
#include "context.h"
#include "option.h"

static void do_quit(GtkApplication* app)
{
  g_application_quit(G_APPLICATION(app));
}

static void on_shutdown(GApplication* app, void* user_data)
{
  UNUSED(app);

  Context* context = (Context*) user_data;
  context_close(context);
}

static bool on_key_press(GtkWidget *widget, GdkEventKey *event, void* user_data)
{
  UNUSED(widget);
  Context* context = (Context*)user_data;

  unsigned mod = event->state & gtk_accelerator_get_default_mod_mask();
  unsigned key = gdk_keyval_to_lower(event->keyval);

  if (context_perform_keymap(context, key, mod)) {
    return true;
  }
  return false;
}

static void on_child_exited(VteTerminal *vte, int status, void* user_data)
{
  UNUSED(vte);
  UNUSED(status);
  Context* context = (Context*)user_data;

  if (!config_get_no_quit(context->config)) {
    do_quit(context->app);
  }
}

static void on_vte_title_changed(VteTerminal *vte, void* user_data)
{
  UNUSED(vte);

  Context* context = (Context*)user_data;
  context_on_change_vte_title(context);
}

#ifdef USE_ASYNC_SPAWN
static void on_spawn(VteTerminal *vte, GPid pid, GError *error, void* user_data)
{
  UNUSED(vte);
  UNUSED(pid);

  Context* context = (Context*)user_data;

  if (error) {
    g_printerr("error: %s\n", error->message);
    do_quit(context->app);
  }
}
#endif

static void on_activate(GtkApplication* app, void* user_data)
{
  dd("app activate");
  GList* list = gtk_application_get_windows(app);
  if (list) {
    gtk_window_present(GTK_WINDOW(list->data));
    return;
  }
  GError* error = NULL;
  Option* option = (Option*)user_data;

  GtkWindow *window = GTK_WINDOW(gtk_application_window_new(app));
  gtk_container_set_border_width(GTK_CONTAINER(window), 0);

  VteTerminal* vte = VTE_TERMINAL(vte_terminal_new());
  gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(vte));
  vte_terminal_set_rewrap_on_resize(vte, true);

  Context* context = context_init(option, app, vte);
  context_load(context);

  g_signal_connect(app, "shutdown", G_CALLBACK(on_shutdown), context);
  g_signal_connect(G_OBJECT(vte), "key-press-event", G_CALLBACK(on_key_press), context);
  g_signal_connect(G_OBJECT(vte), "child-exited", G_CALLBACK(on_child_exited), context);
  g_signal_connect(G_OBJECT(vte), "window-title-changed", G_CALLBACK(on_vte_title_changed), context);

  int argc;
  char** argv;
  char* line = config_get_shell(context->config);
  g_shell_parse_argv(line, &argc, &argv, &error);
  if (error) {
    g_printerr("error: %s\n", error->message);
    g_error_free(error);
    do_quit(app);
    return;
  }
  char** env = g_get_environ();
  env = g_environ_setenv(env, "TERM", config_get_term(context->config), true);

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
    on_spawn,            // callback
    context              // user_data
  );
#else
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
    g_strfreev(env);
    g_strfreev(argv);
    g_printerr("error: %s\n", error->message);
    g_error_free(error);
    do_quit(app);
    return;
  }
#endif

  g_strfreev(env);
  g_strfreev(argv);
  gtk_widget_grab_focus(GTK_WIDGET(vte));
  gtk_widget_show_all(GTK_WIDGET(window));
}

int main(int argc, char* argv[])
{
  dd("start");
  int exit_code = EXIT_SUCCESS;
  Option* option = option_init();

  GError* error = NULL;
  bool is_continuous = option_check(option, &argc, &argv, &error);
  if (error) {
    g_printerr("error: %s\n", error->message);
    exit_code = EXIT_FAILURE;
    g_error_free(error);
    goto CLEANUP;
  }

  if (!is_continuous) {
    goto CLEANUP;
  }

  GtkApplication* app = gtk_application_new("me.endaaman.tym", G_APPLICATION_NON_UNIQUE);
  g_signal_connect(app, "activate", G_CALLBACK(on_activate), option);
  exit_code = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

CLEANUP:
  dd("cleanup");
  option_close(option);
  return exit_code;
}
