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


static bool on_key_press(GtkWidget *widget, GdkEventKey *event, void* user_data)
{
  UNUSED(widget);
  Context* context = (Context*)user_data;

  unsigned mod = event->state & gtk_accelerator_get_default_mod_mask();
  unsigned key = gdk_keyval_to_lower(event->keyval);

  if (context_on_key(context, key, mod)) {
    return true;
  }
  return false;
}

static void quit(VteTerminal *vte, int status, void* user_data)
{
  UNUSED(vte);
  UNUSED(status);

  GApplication* app = G_APPLICATION(user_data);
  g_application_quit(app);
}

#ifdef USE_ASYNC_SPAWN
static void spawn_callback(VteTerminal *vte, GPid pid, GError *error, void* user_data)
{
  UNUSED(vte);
  UNUSED(pid);
  UNUSED(user_data);
  if (error) {
    g_printerr("error: %s\n", error->message);
  }
}
#endif

static void activate(GtkApplication* app, void* user_data)
{
  dd("activate");
  Context* context = (Context*)user_data;

  GList* list = gtk_application_get_windows(app);
  if (list) {
    gtk_window_present(GTK_WINDOW(list->data));
    return;
  }

  GtkWidget *window = gtk_application_window_new(app);
  gtk_window_set_icon_name(GTK_WINDOW(window), "terminal");
  gtk_container_set_border_width(GTK_CONTAINER(window), 0);

  VteTerminal* vte = VTE_TERMINAL(vte_terminal_new());
  gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(vte));
  vte_terminal_set_rewrap_on_resize(vte, true);
  g_signal_connect(G_OBJECT(vte), "child-exited", G_CALLBACK(quit), app);
  g_signal_connect(G_OBJECT(vte), "key-press-event", G_CALLBACK(on_key_press), context);
  context_set_app(context, app);
  context_set_vte(context, vte);
  context_load_config(context, true);

  char* argv[] = { config_get_shell(context->config), NULL };
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
    NULL                 // user_data */
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
    g_printerr("error: %s\n", error->message);
    g_error_free(error);
    return;
  }
#endif

  g_strfreev(env);
  gtk_widget_grab_focus(GTK_WIDGET(vte));
  gtk_widget_show_all(window);
  gtk_widget_show(window);
}

int main(int argc, char* argv[])
{
  int exit_code = EXIT_SUCCESS;

  bool version = false;
  char* config_file_path = NULL;
  GOptionEntry entries[] = {
    { "version", 'v', 0, G_OPTION_ARG_NONE, &version, "Show Version", NULL },
    { "use", 'u', 0, G_OPTION_ARG_STRING, &config_file_path,  "Use <path> instead of default config file", "<path>"},
    { NULL }
  };
  GOptionContext* option_context = g_option_context_new("");
  g_option_context_add_main_entries(option_context, entries, NULL);
  GError* error = NULL;
  if (!g_option_context_parse(option_context, &argc, &argv, &error)) {
    g_printerr("error: %s\n", error->message);
    exit_code = EXIT_FAILURE;
    g_error_free(error);
    goto CLEANUP;
  }
  if (version) {
    g_print("version %s\n", PACKAGE_VERSION);
    goto CLEANUP;
  }

  Context* context = context_init(config_file_path);

  GtkApplication* app = gtk_application_new("me.endaaman.tym", G_APPLICATION_NON_UNIQUE);
  g_signal_connect(app, "activate", G_CALLBACK(activate), context);
  exit_code = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  context_close(context);

CLEANUP:
  if (config_file_path) {
    g_free(config_file_path);
  }
  g_option_context_free(option_context);
  return exit_code;
}
