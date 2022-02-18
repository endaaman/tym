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
    G_APPLICATION_HANDLES_COMMAND_LINE
  ));
  app->meta = meta_init();
}

void app_quit()
{
  df();
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

  /* Option* option = option_init(app->meta); */
  /* option_register_entries(option, app->gapp); */

  g_signal_connect(app->gapp, "command-line", G_CALLBACK(on_command_line), NULL);
  /* g_signal_connect(app->gapp, "activate", G_CALLBACK(on_activate), app); */
  return g_application_run(app->gapp, argc, argv);
}

void app_drop_context(Context* context)
{
  context_close(context);
  app->contexts = g_list_remove(app->contexts, context);
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

static int _contexts_sort_func(const void* a, const void* b)
{
  return ((Context*)a)->id - ((Context*)b)->id;
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
  /* gtk_window_close(context->layout.window); */
  /* g_application_release() */
  /* g_application_quit(G_APPLICATION(context->app)); */
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
  context->state.initialized = false;
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
  Context* context = (Context*)user_data;
  /* context_quit(context); */
  app_drop_context(context);
  /* g_application_release(context->gapp); */
  return false;
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
  Context* context = (Context*)user_data;
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
  Context* context = context_init(index, app->meta, app->gapp);
  app->contexts = g_list_insert_sorted(app->contexts, context, _contexts_sort_func);

  dd("hi");
  int argc = -1;
  char** argv = g_application_command_line_get_arguments(cli, &argc);

  dd("ARGC: %d", argc);
  int i;
  for(i=1;i<argc;i++) {
      dd("%s", argv[i]);
  }

  GOptionContext* option_context = g_option_context_new(NULL);
  /* g_option_context_set_help_enabled(option_context, FALSE); */
  g_option_context_add_main_entries(option_context, context->option->entries, NULL);
  g_option_context_parse(option_context, &argc, &argv, &error);


  char* s = NULL;
  option_get_str_value(context->option, "shell", &s);
  dd("shell: '%s'", s);

  bool version = option_get_version(context->option);
  if (version) {
    g_print("version %s\n", PACKAGE_VERSION);
    return 0;
  }
  char* signal = option_get_signal(context->option);
  if (signal) {
    const char* path = g_application_get_dbus_object_path(gapp);
    dd("DBus is active: %s", path);
    GDBusConnection* conn = g_application_get_dbus_connection(gapp);
    g_dbus_connection_emit_signal(conn, NULL, path, TYM_APP_ID, signal, NULL, &error);
    g_message("Signal `%s` has been sent.\n", signal);
    if (error) {
      g_error("%s", error->message);
      g_error_free(error);
    }
    return 0;
  }

  g_application_hold(gapp);
  /* g_application_activate(gapp); */

  /* GtkWindow* w = gtk_application_get_active_window(GTK_APPLICATION(gapp)); */
  /* if (w) { */
  /*   gtk_window_present(w); */
  /*   return; */
  /* } */

  // TODO:
  // - context list
  // - option manage

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

  const char* path = g_application_get_dbus_object_path(context->gapp);
  const char* id = g_application_get_application_id(context->gapp);
  bool valid = g_application_id_is_valid(id);

  dd("id %s path %s valid %d %d", id, path, valid, true);
  GDBusConnection* conn = g_application_get_dbus_connection(context->gapp);
  g_dbus_connection_signal_subscribe(
    conn,
    NULL,       // sender
    "me.endaaman.tym", // interface_name
    NULL,       // member
    "/me/endaaman/tym2",       // object_path
    NULL,       // arg0
    G_DBUS_SIGNAL_FLAGS_NONE,
    on_dbus_signal,
    context,
    NULL        // user data free func
  );

  const char* line = context_get_str(context, "shell");

  char** shell_argv;
  g_shell_parse_argv(line, NULL, &shell_argv, &error);
  if (error) {
    g_error("%s", error->message);
    g_error_free(error);
    /* g_application_quit(app); */
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
    shell_argv,                // argv
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
    g_application_quit(app);
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
