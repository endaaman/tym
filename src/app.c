/**
 * app.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "app.h"

App* app = NULL;


int on_local_options(GApplication* gapp, GVariantDict* values, void* user_data);
int on_command_line(GApplication* app, GApplicationCommandLine* cli, void* user_data);

void app_init()
{
  df();
  app = g_new0(App, 1);
  app->meta = meta_init();
  app->ipc = ipc_init();
#ifdef TYM_USE_VTE_TERMPROP
  // VTE does not implement OSC 52, so the clipboard is written through a termprop
  // of our own instead. The value is base64 so that it needs no escaping.
  vte_install_termprop(TYM_TERMPROP_CLIPBOARD, VTE_PROPERTY_STRING, VTE_PROPERTY_FLAG_NONE);
  vte_install_termprop(TYM_TERMPROP_CLIPBOARD_FLAGS, VTE_PROPERTY_STRING, VTE_PROPERTY_FLAG_EPHEMERAL);
#endif
}

void app_close()
{
  df();
  for (GList* li = app->contexts; li != NULL; li = li->next) {
    Context* c = (Context*)li->data;
    context_close(c);
  }
  g_application_quit(app->gapp);
  g_object_unref(app->gapp);
  meta_close(app->meta);
  ipc_close(app->ipc);
  g_free(app);
}

static char* _get_dest_path_from_option(Option* option) {
  char* path = NULL;
  char* dest = option_get_str(option, "dest");
  if (dest) {
    path = g_strdup_printf(TYM_OBJECT_PATH_FMT_STR, dest);
  } else {
    char** env = g_get_environ();
    const char* dest_str = g_environ_getenv(env, "TYM_ID");
    if (!dest_str) {
      return NULL;
    }
    path = g_strdup_printf(TYM_OBJECT_PATH_FMT_STR, dest_str);
  }
  return path;
}

static int _perform_signal(char* dest_path, char* signal_name, char* method_name, char* param)
{
  GError* error = NULL;

  GDBusConnection* conn = g_application_get_dbus_connection(app->gapp);
  if (!dest_path) {
    g_warning("--dest is not provided and $TYM_ID is not set.");
    return 1;
  }

  GVariant* params = param
    ? g_variant_new("(s)", param)
    : g_variant_new("()");

  /* process signal */
  if (signal_name) {
    g_dbus_connection_emit_signal(conn, NULL, dest_path, TYM_APP_ID, signal_name, params, &error);
    g_print("Sent signal:%s to path:%s interface:%s\n", signal_name, dest_path, TYM_APP_ID);
    g_free(signal_name);
    if (error) {
      g_error("%s", error->message);
      g_error_free(error);
    }
    return 0;
  }

  /* process method call */
  GVariant* result = g_dbus_connection_call_sync(
      conn,        // conn
      TYM_APP_ID,  // bus_name
      dest_path,   // object_path
      TYM_APP_ID,  // interface_name
      method_name, // method_name
      params,      // parameters
      NULL,        // reply_type
      G_DBUS_CALL_FLAGS_NONE, // flags
      1000,        // timeout
      NULL,        // cancellable
      &error
  );
  g_print("Call method:%s on path:%s interface:%s\n", method_name, dest_path, TYM_APP_ID);
  if (error) {
    g_warning("%s", error->message);
    g_error_free(error);
    return 1;
  }
  dd("result type:%s", g_variant_get_type_string(result));
  char* msg = g_variant_print(result, true);
  g_print("%s\n", msg);
  g_free(msg);
  return 0;
}

int app_start(Option* option, int argc, char **argv)
{
  df();
  g_assert(!app->gapp);

  GApplicationFlags flags = G_APPLICATION_HANDLES_COMMAND_LINE | G_APPLICATION_SEND_ENVIRONMENT;
  char* app_id = TYM_APP_ID;
  if (option_get_bool(option, "isolated")) {
    flags |= G_APPLICATION_NON_UNIQUE;
    app_id = TYM_APP_ID_ISOLATED;
  }

  app->gapp = G_APPLICATION(gtk_application_new(app_id, flags));
  GError* error = NULL;
  g_application_register(app->gapp, NULL, &error);

  g_signal_connect(app->gapp, "handle-local-options", G_CALLBACK(on_local_options), option);
  g_signal_connect(app->gapp, "command-line", G_CALLBACK(on_command_line), NULL);
  return g_application_run(app->gapp, argc, argv);
}

static int _contexts_sort_func(const void* a, const void* b)
{
  return ((Context*)a)->id - ((Context*)b)->id;
}

Context* app_spawn_context(Option* option)
{
  df();
  unsigned index = 0;
  int ordered_id = option_get_int(option, "id");
  if (ordered_id) {
    for (GList* li = app->contexts; li != NULL; li = li->next) {
      Context* c = (Context*)li->data;
      if (c->id == ordered_id) {
        context_log_warn(c, true, "id=%d has been already acquired.", ordered_id);
        return NULL;
      }
    }
    index = ordered_id;
  } else {
    for (GList* li = app->contexts; li != NULL; li = li->next) {
      Context* c = (Context*)li->data;
      /* scanning from 0 and if find first ctx that is not continus from 0, the index is new index. */
      if (c->id != index) {
        break;
      }
      index += 1;
    }
  }

  Context* context = context_init(index, option);
  app->contexts = g_list_insert_sorted(app->contexts, context, _contexts_sort_func);
  g_application_hold(app->gapp);

  context_log_message(context, false, "Started.");
  return context;
}

void app_quit_context(Context* context)
{
  df();
  g_application_release(app->gapp);
  GDBusConnection* conn = g_application_get_dbus_connection(app->gapp);
  g_dbus_connection_unregister_object(conn, context->registration_id);
  context_log_message(context, false, "Quit.");
  app->contexts = g_list_remove(app->contexts, context);
  context_close(context);
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

#ifdef TYM_USE_VTE_TERMPROP
  const char* title = vte_terminal_get_termprop_string(context->layout.vte, "xterm.title", NULL);
#else
  const char* title = vte_terminal_get_window_title(context->layout.vte);
#endif

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

#ifdef TYM_USE_VTE_TERMPROP
static int _set_clipboard_from_termprop(void* user_data)
{
  df();
  GBytes* bytes = (GBytes*)user_data;
  gsize len = 0;
  const char* text = (const char*)g_bytes_get_data(bytes, &len);
  GtkClipboard* clipboard = gtk_clipboard_get_for_display(
    gdk_display_get_default(), GDK_SELECTION_CLIPBOARD);
  gtk_clipboard_set_text(clipboard, text, len);
  gtk_clipboard_store(clipboard);
  g_bytes_unref(bytes);
  return G_SOURCE_REMOVE;
}

static void on_vte_clipboard_termprop_changed(VteTerminal* vte, const char* name, void* user_data)
{
  df();
  Context* context = (Context*)user_data;
  if (!context_get_bool(context, "osc_clipboard")) {
    return;
  }
  // Only `vte_terminal_get_termprop_*()` may be called on the terminal from this
  // handler, so the payload is copied out and the clipboard is set from an idle.
  size_t size = 0;
  const char* value = vte_terminal_get_termprop_string(vte, TYM_TERMPROP_CLIPBOARD, &size);
  if (!value) {
    return;
  }
  gsize len = 0;
  char* text = (char*)g_base64_decode(value, &len);
  if (!text) {
    return;
  }
  if (len > 0 && g_utf8_validate(text, len, NULL)) {
    g_idle_add(_set_clipboard_from_termprop, g_bytes_new_take(text, len));
  } else {
    context_log_warn(context, false, "Ignored invalid clipboard payload from the application");
    g_free(text);
  }
}
#endif

static glong _cell_width(gunichar c, bool cjk_wide)
{
  if (g_unichar_iszerowidth(c)) {
    return 0;
  }
  if (cjk_wide ? g_unichar_iswide_cjk(c) : g_unichar_iswide(c)) {
    return 2;
  }
  return 1;
}

static bool _row_is_full(const char* text, glong cols, bool cjk_wide)
{
  glong width = 0;
  for (const char* p = text; *p; p = g_utf8_next_char(p)) {
    width += _cell_width(g_utf8_get_char(p), cjk_wide);
    if (width >= cols) {
      return true;
    }
  }
  return false;
}

// byte offset of the character covering `col`, or -1 when the cell is empty.
// also reports whether the row is filled up to the last column.
static glong _scan_row(const char* text, glong col, glong cols, bool cjk_wide, bool* full)
{
  glong width = 0;
  glong offset = -1;
  for (const char* p = text; *p; p = g_utf8_next_char(p)) {
    glong w = _cell_width(g_utf8_get_char(p), cjk_wide);
    if (offset < 0 && col < width + w) {
      offset = p - text;
    }
    width += w;
    if (offset >= 0 && width >= cols) {
      *full = true;
      return offset;
    }
  }
  *full = width >= cols;
  return offset;
}

static char* _get_row_text(VteTerminal* vte, glong row)
{
  char* text = tym_get_text_range(vte, row, 0, row, vte_terminal_get_column_count(vte));
  if (!text) {
    return NULL;
  }
  size_t len = strlen(text);
  while (len > 0 && (text[len - 1] == '\n' || text[len - 1] == '\r')) {
    text[--len] = '\0';
  }
  return text;
}

// Detect an URI spanning hard-wrapped lines. VTE joins soft-wrapped lines when
// matching, but TUI apps that wrap text by themselves (e.g. Ink-based ones)
// emit hard line breaks, so VTE matches only a single-line fragment. Here rows
// filled up to the last column are joined with the following row, then the URI
// regex is applied to the restored paragraph.
static char* check_wrapped_uri(Context* context, VteTerminal* vte, GdkEventButton* event)
{
  const glong MAX_JOINED_ROWS = 64;

  pcre2_code* code = context->layout.uri_regex;
  if (!code) {
    return NULL;
  }

  glong cols = vte_terminal_get_column_count(vte);
  glong char_width = vte_terminal_get_char_width(vte);
  glong char_height = vte_terminal_get_char_height(vte);
  if (cols <= 0 || char_width <= 0 || char_height <= 0) {
    return NULL;
  }
  bool cjk_wide = vte_terminal_get_cjk_ambiguous_width(vte) == 2;

  GtkStyleContext* style = gtk_widget_get_style_context(GTK_WIDGET(vte));
  GtkBorder padding;
  gtk_style_context_get_padding(style, gtk_style_context_get_state(style), &padding);

  GtkAdjustment* adj = gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(vte));
  double adj_lower = gtk_adjustment_get_lower(adj);
  double adj_upper = gtk_adjustment_get_upper(adj);
  double adj_value = gtk_adjustment_get_value(adj);
  glong lower = (glong)adj_lower;
  glong upper = (glong)adj_upper;
  glong row = (glong)(adj_value + (event->y - padding.top) / char_height);
  glong col = (glong)((event->x - padding.left) / char_width);
  if (col < 0 || col >= cols || row < lower || row >= upper) {
    return NULL;
  }

  char* text = _get_row_text(vte, row);
  if (!text) {
    return NULL;
  }
  bool full = false;
  glong offset = _scan_row(text, col, cols, cjk_wide, &full);
  if (offset < 0) {
    // clicked on an empty cell
    g_free(text);
    return NULL;
  }

  // rows above belong to the same wrapped paragraph while each of them is
  // filled up to the last column
  GPtrArray* above = g_ptr_array_new_with_free_func(g_free);
  for (glong r = row; r > lower && (glong)above->len < MAX_JOINED_ROWS; r--) {
    char* t = _get_row_text(vte, r - 1);
    if (!t || !_row_is_full(t, cols, cjk_wide)) {
      g_free(t);
      break;
    }
    g_ptr_array_add(above, t);
  }

  if (!full && above->len == 0) {
    // no wrapping around the clicked row; leave it to the plain VTE match
    g_free(text);
    g_ptr_array_free(above, true);
    return NULL;
  }

  GString* joined = g_string_new(NULL);
  for (guint i = above->len; i > 0; i--) {
    g_string_append(joined, g_ptr_array_index(above, i - 1));
  }
  g_ptr_array_free(above, true);
  PCRE2_SIZE clicked = joined->len + offset;
  g_string_append(joined, text);
  g_free(text);

  // join rows downward while the last joined row is filled up to the last column
  for (glong r = row; full && r + 1 < upper && r - row < MAX_JOINED_ROWS; r++) {
    char* t = _get_row_text(vte, r + 1);
    if (!t) {
      break;
    }
    full = _row_is_full(t, cols, cjk_wide);
    g_string_append(joined, t);
    g_free(t);
  }

  char* uri = NULL;
  pcre2_match_data* match_data = pcre2_match_data_create_from_pattern(code, NULL);
  if (match_data) {
    PCRE2_SIZE match_offset = 0;
    while (match_offset < joined->len) {
      int res = pcre2_match(code, (PCRE2_SPTR)joined->str, joined->len, match_offset, 0, match_data, NULL);
      if (res <= 0) {
        break;
      }
      PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(match_data);
      if (ovector[0] <= clicked && clicked < ovector[1]) {
        uri = g_strndup(joined->str + ovector[0], ovector[1] - ovector[0]);
        break;
      }
      if (ovector[1] > clicked) {
        break;
      }
      match_offset = ovector[1] > match_offset ? ovector[1] : match_offset + 1;
    }
    pcre2_match_data_free(match_data);
  }
  g_string_free(joined, true);
  return uri;
}

static bool on_vte_click(VteTerminal* vte, GdkEventButton* event, void* user_data)
{
  df();
  Context* context = (Context*)user_data;
  char* uri = NULL;
  if (context->layout.uri_tag >= 0) {
    uri = check_wrapped_uri(context, vte, event);
    if (!uri) {
      uri = vte_terminal_match_check_event(vte, (GdkEvent*)event, NULL);
    }
  }
  bool result = false;
  if (hook_perform_clicked(context->hook, context->lua, event->button, uri, &result)) {
    g_free(uri);
    return result;
  }
  if (uri) {
    for (int i = strlen(uri) - 1; uri[i] == '.' || uri[i] == ','; i--) {
      uri[i] = '\0';
    }
    context_launch_uri(context, uri);
    g_free(uri);
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
  g_free(text);
}

static void on_vte_resize_request(GtkWidget* widget, unsigned int width, unsigned int height, void* user_data)
{
  Context* context = (Context*)user_data;
  dd("Recieve resize sequence: width=%d height=%d", width, height);
  context_resize(context, width, height);
}


static gboolean on_window_close(GtkWidget* widget, cairo_t* cr, void* user_data)
{
  df();
  // close context in child-exited handler
  return true;
}

static bool on_window_focus_in(GtkWindow* window, GdkEvent* event, void* user_data)
{
  Context* context = (Context*)user_data;
  gtk_window_set_urgency_hint(window, false);
  hook_perform_activated(context->hook, context->lua);
  return false;
}

static bool on_window_focus_out(GtkWindow* window, GdkEvent* event, void* user_data)
{
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

static void on_window_resize(GtkWidget* widget, GtkAllocation* allocation, gpointer user_data)
{
  Context* context = (Context*)user_data;
  hook_perform_resized(context->hook, context->lua);
}

void on_dbus_signal(
  GDBusConnection* conn,
  const char* sender_name,
  const char* object_path,
  const char* interface_name,
  const char* signal_name,
  GVariant* params,
  void* user_data)
{
  Context* context = (Context*)user_data;
  dd("DBus signal received");
  dd("\tcontext id: %d", context->id);
  dd("\tsender_name: %s", sender_name);
  dd("\tobject_path: %s", object_path);
  dd("\tinterface_name: %s", interface_name);
  dd("\tsignal_name: %s", signal_name);

  if (ipc_signal_perform(app->ipc, context, signal_name, params)) {
    context_log_message(context, false, "Signal received:`%s` object_path:`%s`", signal_name, object_path);
    return;
  }

  context_log_warn(context, true, "Unsupported signal: `%s`", signal_name);
}

void on_dbus_call_method(
    GDBusConnection* conn,
    const gchar* sender_name,
    const gchar* object_path,
    const gchar* interface_name,
    const gchar* method_name,
    GVariant* params,
    GDBusMethodInvocation* invocation,
    gpointer user_data)
{
  Context* context = (Context*)user_data;
  dd("DBus method call");
  dd("\tcontext id: %d", context->id);
  dd("\tsender_name: %s", sender_name);
  dd("\tobject_path: %s", object_path);
  dd("\tinterface_name: %s", interface_name);
  dd("\tmethod_name: %s", method_name);

  if (ipc_method_perform(app->ipc, context, method_name, params, invocation)) {
    context_log_message(context, false, "Method call:`%s` object_path:`%s`", method_name, object_path);
    g_dbus_connection_flush(conn, NULL, NULL, NULL);
    return;
  }

  context_log_warn(context, true, "Unsupported method call:`%s`", method_name);
  GError* error = g_error_new(
      g_quark_from_static_string("TymInvalidMethodCall"),
      TYM_ERROR_INVALID_METHOD_CALL,
      "Unsupported method call: %s",
      method_name);

  g_dbus_method_invocation_return_gerror(invocation, error);
  g_dbus_connection_flush(conn, NULL, NULL, NULL);
}

int on_local_options(GApplication* gapp, GVariantDict* values, void* user_data)
{
  df();
  Option* option = (Option*)(user_data);

  char* dest_path = _get_dest_path_from_option(option);
  char* signal_name = option_get_str(option, "signal");
  char* method_name = option_get_str(option, "call");
  char* param = option_get_str(option, "param");
  if (signal_name || method_name) {
    int code = _perform_signal(dest_path, signal_name, method_name, param);
    g_free(dest_path);
    return code;
  }

  if (option_get_bool(option, "daemon")) {
    if (g_application_get_is_remote(app->gapp)) {
      /* If there is a normal primary instance, --daemon flag would make it "zombie" */
      /* So daemonization should be allowed only when the instanciation is the primary */
      g_warning("There is any tym instance. So could not start as daemon process.");
      return 1;
    }
  }

  const char* cwd = option_get_str(option, "cwd");
  if (cwd != NULL && !g_path_is_absolute(cwd)) {
    g_warning("cwd must be an absolute path");
    return 1;
  }

  return -1;
}

static bool _subscribe_dbus(Context* context)
{
  df();
  GError* error = NULL;

  const char* app_id = g_application_get_application_id(app->gapp);
  GDBusConnection* conn = g_application_get_dbus_connection(app->gapp);

  g_dbus_connection_signal_subscribe(
    conn,
    NULL,        // sender
    app_id,      // interface_name
    NULL,        // member
    context->object_path, // object_path
    NULL,        // arg0
    G_DBUS_SIGNAL_FLAGS_NONE,
    on_dbus_signal,
    context,
    NULL         // user data free func
  );

  GDBusInterfaceVTable vtable = {
    on_dbus_call_method,
    NULL,
    NULL,
  };

  static const char introspection_xml[] =
    "<node>"
    "  <interface name='" TYM_APP_ID "'>"
    "    <method name='echo'>"
    "      <arg type='s' direction='in'/>"
    "      <arg type='s' direction='out'/>"
    "    </method>"
    "    <method name='get_ids'>"
    "      <arg type='ai' direction='out'/>"
    "    </method>"
    "    <method name='eval'>"
    "      <arg type='s' direction='in'/>"
    "      <arg type='s' direction='out'/>"
    "    </method>"
    "    <method name='exec'>"
    "      <arg type='s' direction='in'/>"
    "    </method>"
    "    <method name='exec_file'>"
    "      <arg type='s' direction='in'/>"
    "    </method>"
    "  </interface>"
    "</node>";

  GDBusNodeInfo* introspection_data = g_dbus_node_info_new_for_xml(introspection_xml, &error);
  if (error) {
    g_error("%s", error->message);
    g_error_free(error);
    app_quit_context(context);
    return false;
  }

  context_log_message(context, false, "DBus: object_path='%s' interface_name:'%s'", context->object_path, app_id);
  context->registration_id = g_dbus_connection_register_object(
      conn,
      context->object_path,
      introspection_data->interfaces[0], // interface_info,
      &vtable, // vtable
      context, // user_data,
      NULL,    // user_data_free_func,
      &error   // error
  );
  if (context->registration_id <= 0) {
    context_log_warn(context, true, "Could not subscribe DBus with path:%s", context->object_path);
  }

  return true;
}

#ifdef TYM_USE_VTE_SPAWN_ASYNC
static void on_vte_spawn(VteTerminal* vte, GPid child_pid, GError* error, void* user_data)
{
  Context* context = (Context*)user_data;
  context->initialized = true;
  context->child_pid = child_pid;
  if (error) {
    g_warning("vte-spawn error: %s", error->message);
    /* g_error_free(error); */
    gtk_window_close(context->layout.window);
    app_quit_context(context);
    /* dd("%d", gtk_application_new); */
    return;
  }
}
#endif

int on_command_line(GApplication* gapp, GApplicationCommandLine* cli, void* user_data)
{
  df();
  GError* error = NULL;

  int argc = -1;
  char** argv = g_application_command_line_get_arguments(cli, &argc);

  Option* option = option_init(meta_get_option_entries(app->meta));
  if (!option_parse(option, argc, argv)){
    return 1;
  };

  if (option_get_bool(option, "daemon")) {
    GtkWindow* window = gtk_application_get_active_window(GTK_APPLICATION(gapp));
    if (window) {
      g_warning("Blocked another instance from trying to start as daemon process.");
      return 1;
    }

    /* Only creates a window, never shows it. */
    window = GTK_WINDOW(gtk_application_window_new(GTK_APPLICATION(gapp)));
    UNUSED(window);
    g_message("Starting as daemon process.");
    return 0;
  }

  if (option_get_str(option, "signal") || option_get_str(option, "call")) {
    /* Do nothing */
    dd("D-Bus signal/method call was performed on a remote process.");
    return 0;
  }

  Context* context = app_spawn_context(option);
  if (!context) {
    return 1;
  }

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

  context_signal_connect(context, vte, "drag-data-received", G_CALLBACK(on_vte_drag_data_received));
  context_signal_connect(context, vte, "key-press-event", G_CALLBACK(on_vte_key_press));
  context_signal_connect(context, vte, "scroll-event", G_CALLBACK(on_vte_mouse_scroll));
  context_signal_connect(context, vte, "child-exited", G_CALLBACK(on_vte_child_exited));
  context_signal_connect(context, vte, "window-title-changed", G_CALLBACK(on_vte_title_changed));
  context_signal_connect(context, vte, "bell", G_CALLBACK(on_vte_bell));
  context_signal_connect(context, vte, "button-press-event", G_CALLBACK(on_vte_click));
  context_signal_connect(context, vte, "selection-changed", G_CALLBACK(on_vte_selection_changed));
#ifdef TYM_USE_VTE_TERMPROP
  context_signal_connect(context, vte, "termprop-changed::" TYM_TERMPROP_CLIPBOARD, G_CALLBACK(on_vte_clipboard_termprop_changed));
#endif
  context_signal_connect(context, vte, "resize-window", G_CALLBACK(on_vte_resize_request));
  context_signal_connect(context, window, "destroy", G_CALLBACK(on_window_close));
  context_signal_connect(context, window, "focus-in-event", G_CALLBACK(on_window_focus_in));
  context_signal_connect(context, window, "focus-out-event", G_CALLBACK(on_window_focus_out));
  context_signal_connect(context, window, "draw", G_CALLBACK(on_window_draw));
  context_signal_connect(context, window, "size-allocate", G_CALLBACK(on_window_resize));

  if (app->is_isolated) {
    g_message("This process is isolated so never listen to D-Bus signal/method call.");
  } else {
    _subscribe_dbus(context);
  }

  const char* shell_line = context_get_str(context, "shell");

  char** shell_argv = NULL;
  if (g_strv_length(option->rest_argv) >= 2) {
    GStrvBuilder* builder = g_strv_builder_new();
    char** a = &option->rest_argv[1];
    while (*a) {
      /* Skips an unnecessary entry that equals `--` */
      if (is_equal(*a, "--")) {
        a++;
        continue;
      }
      g_strv_builder_add(builder, *a);
      a++;
    }
    shell_argv = g_strv_builder_end(builder);
  } else {
    g_shell_parse_argv(shell_line, NULL, &shell_argv, &error);
    if (error) {
      g_warning("Parse error: %s", error->message);
      g_error_free(error);
      app_quit_context(context);
      return 0;
    }
  }

  const char* const* env = g_application_command_line_get_environ(cli);
  char** shell_env = g_new0(char*, g_strv_length((char**)env) + 1);
  int i = 0;
  while (env[i]) {
    shell_env[i] = g_strdup(env[i]);
    i += 1;
  }
  shell_env = g_environ_setenv(shell_env, "TERM", context_get_str(context, "term"), true);
  char* id_str = g_strdup_printf("%i", context->id);
  shell_env = g_environ_setenv(shell_env, "TYM_ID", id_str, true);
  g_free(id_str);

  const char* cwd = option_get_str(option, "cwd");
  if (cwd == NULL) {
    cwd = g_application_command_line_get_cwd(cli);
  }

#ifdef TYM_USE_VTE_SPAWN_ASYNC
  vte_terminal_spawn_async(
    vte,                 // terminal
    VTE_PTY_DEFAULT,     // pty flag
    cwd,                 // working directory
    shell_argv,          // argv
    shell_env,           // envv
    G_SPAWN_SEARCH_PATH, // spawn_flags
    NULL,                // child_setup
    NULL,                // child_setup_data
    NULL,                // child_setup_data_destroy
    5000,                // timeout
    NULL,                // cancel callback
    on_vte_spawn,        // callback
    context              // user_data
  );
#else
  GPid child_pid;
  vte_terminal_spawn_sync(
    vte,
    VTE_PTY_DEFAULT,
    cwd,
    shell_argv,
    shell_env,
    G_SPAWN_SEARCH_PATH,
    NULL,
    NULL,
    &child_pid,
    NULL,
    &error
  );
  context->child_pid = child_pid;

  if (error) {
    g_strfreev(shell_env);
    g_strfreev(shell_argv);
    g_error("%s", error->message);
    g_error_free(error);
    app_quit_context(context);
    return 1;
  }
#endif

  g_strfreev(shell_env);
  g_strfreev(shell_argv);
  gtk_widget_grab_focus(GTK_WIDGET(vte));
  gtk_widget_show_all(GTK_WIDGET(context->layout.window));
  return 0;
}
