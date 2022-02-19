/**
 * tym.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "app.h"

App* app = NULL;

void app_init()
{
  df();
  app = g_malloc0(sizeof(App));
  app->gapp = G_APPLICATION(gtk_application_new(
    TYM_APP_ID,
    /* G_APPLICATION_NON_UNIQUE | G_APPLICATION_HANDLES_COMMAND_LINE */
    G_APPLICATION_HANDLES_COMMAND_LINE | G_APPLICATION_SEND_ENVIRONMENT
  ));
  app->meta = meta_init();
}

void app_close()
{
  df();
  for (GList* li = app->contexts; li != NULL; li = li->next) {
    Context* c = (Context*)li->data;
    if (!context_is_disposed(c)) {
      context_dispose_only(c);
    }
    context_close(c);
  }
  g_application_quit(app->gapp);
  g_object_unref(app->gapp);
  meta_close(app->meta);
  g_free(app);
}

int app_start(int argc, char** argv)
{
  df();
  GError* error = NULL;

  g_application_register(app->gapp, NULL, &error);
  if (error) {
    g_error("%s", error->message);
    g_error_free(error);
  }

  Option* option = option_init(app->meta);
  option_parse(option, &argc, &argv);

  bool version = option_get_version(option);
  if (version) {
    g_print("version %s\n", PACKAGE_VERSION);
    return 0;
  }
  char* signal = option_get_signal(option);
  if (signal) {
    const char* path = g_application_get_dbus_object_path(app->gapp);
    dd("DBus is active: %s", path);
    GDBusConnection* conn = g_application_get_dbus_connection(app->gapp);
    g_dbus_connection_emit_signal(conn, NULL, path, TYM_APP_ID, signal, NULL, &error);
    g_message("Signal `%s` has been sent.\n", signal);
    if (error) {
      g_error("%s", error->message);
      g_error_free(error);
    }
    return 0;
  }

  g_signal_connect(app->gapp, "command-line", G_CALLBACK(on_command_line), option);
  /* g_signal_connect(app->gapp, "activate", G_CALLBACK(on_activate), app); */
  return g_application_run(app->gapp, argc, argv);
}

static int _contexts_sort_func(const void* a, const void* b)
{
  return ((Context*)a)->id - ((Context*)b)->id;
}

Context* app_start_context(Option* option)
{
  df();
  unsigned index = 0;
  /* create new context */
  for (GList* li = app->contexts; li != NULL; li = li->next) {
    Context* c = (Context*)li->data;
    /* Scanning from 0 and if find first ctx that is not continus from 0, the index is new index. */
    if (c->id != index) {
      break;
    }
    index += 1;
  }

  dd("new context id: %d", index);
  Context* context = context_init(index, app->meta, option);
  app->contexts = g_list_insert_sorted(app->contexts, context, _contexts_sort_func);
  g_application_hold(app->gapp);
  return context;
}

void app_quit_context(Context* context)
{
  df();
  g_application_release(app->gapp);
  context_dispose_only(context);
  /* app->contexts = g_list_remove(app->contexts, context); */
}

static void on_vte_drag_data_received(
  VteTerminal* vte,
  GdkDragContext* drag_context,
  int x,
  int y,
  GtkSelectionData* data,
  unsigned int info,
  unsigned int time,
  void* user_data)
{
  Context* context = (Context*)user_data;
  if (!data || gtk_selection_data_get_format(data) != 8) {
    return;
  }

  gchar** uris = g_uri_list_extract_uris(gtk_selection_data_get_data(data));
  if (!uris) {
    return;
  }

  GRegex* regex = g_regex_new("'", 0, 0, NULL);
  for (gchar** p = uris; *p; ++p) {
    gchar* file_path = g_filename_from_uri(*p, NULL, NULL);
    if (file_path) {
      bool result;
      if (!(hook_perform_drag(context->hook, context->lua, file_path, &result) && result)) {
        gchar* path_escaped = g_regex_replace(regex, file_path, -1, 0, "'\\\\''", 0, NULL);
        gchar* path_wrapped = g_strdup_printf("'%s' ", path_escaped);
        vte_terminal_feed_child(vte, path_wrapped, strlen(path_wrapped));
        g_free(path_escaped);
        g_free(path_wrapped);
      }
      g_free(file_path);
    }
  }
  g_regex_unref(regex);
}

static bool on_vte_key_press(GtkWidget* widget, GdkEventKey* event, void* user_data)
{
  Context* context = (Context*)user_data;

  unsigned mod = event->state & gtk_accelerator_get_default_mod_mask();
  unsigned key = gdk_keyval_to_lower(event->keyval);

  if (context_perform_keymap(context, key, mod)) {
    return true;
  }
  return false;
}

static bool on_vte_mouse_scroll(GtkWidget* widget, GdkEventScroll* e, void* user_data)
{
  Context* context = (Context*)user_data;
  bool result = false;
  if (hook_perform_scroll(context->hook, context->lua, e->delta_x, e->delta_y, e->x, e->y, &result) && result) {
    return true;
  }
  return false;
}

static void on_vte_child_exited(VteTerminal* vte, int status, void* user_data)
{
  df();
  Context* context = (Context*)user_data;
  gtk_window_close(context->layout.window);
  app_quit_context(context);
}

static void on_vte_title_changed(VteTerminal* vte, void* user_data)
{
  df();
  Context* context = (Context*)user_data;
  GtkWindow* window = context->layout.window;
  bool result = false;
  const char* title = vte_terminal_get_window_title(context->layout.vte);
  if (hook_perform_title(context->hook, context->lua, title, &result) && result) {
    return;
  }
  if (title) {
    gtk_window_set_title(window, title);
  }
}

static void on_vte_bell(VteTerminal* vte, void* user_data)
{
  df();
  Context* context = (Context*)user_data;
  bool result = false;
  if (hook_perform_bell(context->hook, context->lua, &result) && result) {
    return;
  }
  GtkWindow* window = context->layout.window;
  if (!gtk_window_is_active(window)) {
    gtk_window_set_urgency_hint(window, true);
  }
}

static bool on_vte_click(VteTerminal* vte, GdkEventButton* event, void* user_data)
{
  df();
  Context* context = (Context*)user_data;
  char* uri = NULL;
  if (context->layout.uri_tag >= 0) {
    uri = vte_terminal_match_check_event(vte, (GdkEvent*)event, NULL);
  }
  bool result = false;
  if (hook_perform_clicked(context->hook, context->lua, event->button, uri, &result) && result) {
    return true;
  }
  if (uri) {
    context_launch_uri(context, uri);
    return true;
  }
  return false;
}

static void on_vte_selection_changed(GtkWidget* widget, void* user_data)
{
  df();
  Context* context = (Context*)user_data;
  if (!vte_terminal_get_has_selection(context->layout.vte)) {
    hook_perform_unselected(context->hook, context->lua);
    return;
  }
  GtkClipboard* cb = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
  char* text = gtk_clipboard_wait_for_text(cb);
  hook_perform_selected(context->hook, context->lua, text);
}


#ifdef TYM_USE_VTE_SPAWN_ASYNC
static void on_vte_spawn(VteTerminal* vte, GPid pid, GError* error, void* user_data)
{
  df();
  Context* context = (Context*)user_data;
  context->initialized = false;
  if (error) {
    g_error("%s", error->message);
    /* TODO: impl */
    /* context_close(context); */
    return;
  }
}
#endif

static gboolean on_window_close(GtkWidget* widget, cairo_t* cr, void* user_data)
{
  df();
  /* Context* context = (Context*)user_data; */
  /* app_quit_context(context); */
  return true;
}

static bool on_window_focus_in(GtkWindow* window, GdkEvent* event, void* user_data)
{
  df();
  Context* context = (Context*)user_data;
  gtk_window_set_urgency_hint(window, false);
  hook_perform_activated(context->hook, context->lua);
  return false;
}

static bool on_window_focus_out(GtkWindow* window, GdkEvent* event, void* user_data)
{
  df();
  Context* context = (Context*)user_data;
  hook_perform_deactivated(context->hook, context->lua);
  return false;
}

static gboolean on_window_draw(GtkWidget* widget, cairo_t* cr, void* user_data)
{
  df();
  Context* context = (Context*)user_data;

  dd("is disposed?: %d", context_is_disposed(context));
  /* NOTICE: need check because this cb would be called after the window closed */
  if (context_is_disposed(context)) {
    dd("guarded!");
    return false;
  }
  const char* value = context_get_str(context, "color_window_background");
  if (is_none(value)) {
    return false;
  }
  GdkRGBA color = {};
  if (gdk_rgba_parse(&color, value)) {
    if (context->layout.alpha_supported) {
      cairo_set_source_rgba(cr, color.red, color.green, color.blue, color.alpha);
    } else {
      cairo_set_source_rgb(cr, color.red, color.green, color.blue);
    }
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);
  }
  return false;
}

void on_dbus_signal(
  GDBusConnection* conn,
  const char* sender_name,
  const char* object_path,
  const char* interface_name,
  const char* signal_name,
  GVariant* parameters,
  void* user_data)
{
  df();
  dd("signal received");
  dd("\tsender_name: %s", sender_name);
  dd("\tobject_path: %s", object_path);
  dd("\tinterface_name: %s", interface_name);
  dd("\tsignal_name: %s", signal_name);
  context_handle_signal((Context*)user_data, signal_name, parameters);
}

int on_command_line(GApplication* gapp, GApplicationCommandLine* cli, void* user_data)
{
  df();
  GError* error = NULL;

  Context* context = app_start_context((Option*)user_data);

  context_load_device(context);
  context_load_lua_context(context);

  context_build_layout(context);
  context_restore_default(context);
  context_load_theme(context);
  context_load_config(context);
  context_override_by_option(context);

  VteTerminal* vte = context->layout.vte;
  GtkWindow* window = context->layout.window;

  GtkTargetEntry drop_types[] = {
    {"text/uri-list", 0, 0}
  };
  gtk_drag_dest_set(GTK_WIDGET(vte), GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_DROP, drop_types, G_N_ELEMENTS(drop_types), GDK_ACTION_COPY);

  g_signal_connect(vte, "drag-data-received", G_CALLBACK(on_vte_drag_data_received), context);
  g_signal_connect(vte, "key-press-event", G_CALLBACK(on_vte_key_press), context);
  g_signal_connect(vte, "scroll-event", G_CALLBACK(on_vte_mouse_scroll), context);
  g_signal_connect(vte, "child-exited", G_CALLBACK(on_vte_child_exited), context);
  g_signal_connect(vte, "window-title-changed", G_CALLBACK(on_vte_title_changed), context);
  g_signal_connect(vte, "bell", G_CALLBACK(on_vte_bell), context);
  g_signal_connect(vte, "button-press-event", G_CALLBACK(on_vte_click), context);
  g_signal_connect(vte, "selection-changed", G_CALLBACK(on_vte_selection_changed), context);
  g_signal_connect(window, "destroy", G_CALLBACK(on_window_close), context);
  g_signal_connect(window, "focus-in-event", G_CALLBACK(on_window_focus_in), context);
  g_signal_connect(window, "focus-out-event", G_CALLBACK(on_window_focus_out), context);
  g_signal_connect(window, "draw", G_CALLBACK(on_window_draw), context);

  const char* app_id = g_application_get_application_id(app->gapp);

  char* object_path = g_strdup_printf(TYM_OBJECT_PATH_FMT, context->id);

  GDBusConnection* conn = g_application_get_dbus_connection(app->gapp);
  g_dbus_connection_signal_subscribe(
    conn,
    NULL,        // sender
    app_id,      // interface_name
    NULL,        // member
    object_path, // object_path
    NULL,        // arg0
    G_DBUS_SIGNAL_FLAGS_NONE,
    on_dbus_signal,
    context,
    NULL         // user data free func
  );
  g_message("DBus active on object_path:'%s' interface_name: '%s'", object_path, app_id);
  g_free(object_path);

  const char* shell_line = context_get_str(context, "shell");

  char** shell_argv;
  g_shell_parse_argv(shell_line, NULL, &shell_argv, &error);
  if (error) {
    g_error("%s", error->message);
    g_error_free(error);
    app_quit_context(context);
    return 1;
  }

/* TODO: get local env */
  char** env = g_get_environ();
  env = g_environ_setenv(env, "TERM", context_get_str(context, "term"), true);

#ifdef TYM_USE_VTE_SPAWN_ASYNC
  vte_terminal_spawn_async(
    vte,                 // terminal
    VTE_PTY_DEFAULT,     // pty flag
    NULL,                // working directory
    shell_argv,          // argv
    env,                 // envv
    G_SPAWN_SEARCH_PATH, // spawn_flags
    NULL,                // child_setup
    NULL,                // child_setup_data
    NULL,                // child_setup_data_destroy
    1000,                // timeout
    NULL,                // cancel callback
    on_vte_spawn,        // callback
    context              // user_data
  );
#else
  GPid child_pid;
  vte_terminal_spawn_sync(
    vte,
    VTE_PTY_DEFAULT,
    NULL,
    shell_argv,
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
    g_error("%s", error->message);
    g_error_free(error);
    app_quit_context(context);
    return;
  }
#endif

  g_strfreev(env);
  /* g_strfreev(argv); */
  g_strfreev(shell_argv);
  gtk_widget_grab_focus(GTK_WIDGET(vte));
  gtk_widget_show_all(GTK_WIDGET(window));
  return 0;
}
