#include <gtk/gtk.h>
#include <vte/vte.h>

static void exit_with_status(VteTerminal *, int status) {
  gtk_main_quit();
}

static char *get_user_shell_with_fallback() {
  if (const char *env = g_getenv("SHELL")) {
    return g_strdup(env);
  }

  if (char *command = vte_get_user_shell()) {
    return command;
  }

  return g_strdup("/bin/sh");
}

static void activate (GtkApplication *app, gpointer user_data)
{
  GtkWidget *window;

  window = gtk_application_window_new (app);
  gtk_window_set_title(GTK_WINDOW (window), "tym");
  gtk_container_set_border_width(GTK_CONTAINER(window), 0);

  GtkWidget *vte_widget = vte_terminal_new();
  VteTerminal *vte = VTE_TERMINAL(vte_widget);

  char **command_argv;
  char *default_argv[2] = {nullptr, nullptr};
  default_argv[0] = get_user_shell_with_fallback();
  command_argv = default_argv;

  char **env = g_get_environ();
  GError *error = nullptr;

  g_signal_connect(vte, "child-exited", G_CALLBACK(exit_with_status), nullptr);

  GPid child_pid;
  if (vte_terminal_spawn_sync(
        vte,
        VTE_PTY_DEFAULT,
        nullptr,
        command_argv,
        env,
        G_SPAWN_SEARCH_PATH,
        nullptr,
        nullptr,
        &child_pid,
        nullptr,
        &error))
  {
    vte_terminal_watch_child(vte, child_pid);
  } else {
    g_printerr("%s\n", error->message);
  }

  gtk_container_add (GTK_CONTAINER (window), vte_widget);
  gtk_widget_grab_focus(vte_widget);

  gtk_widget_show_all (window);
}

int main (int argc, char **argv)
{
  GtkApplication *app = gtk_application_new ("me.endaaman.tym", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  int status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}
