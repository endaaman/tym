/**
 * ipc.c
 *
 * Copyright (c) 2022 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */


#include "ipc.h"
#include "app.h"


typedef void (*TymSignalHandler)(Context*, GVariant*);
typedef void (*TymMethodHandler)(Context*, GVariant*, GDBusMethodInvocation*);

typedef struct {
  const char* signal_name;
  TymSignalHandler func;
} SignalDef;

typedef struct {
  const char* method_name;
  TymMethodHandler func;
} MethodDef;


void ipc_signal_hook(Context* context, GVariant* parameters)
{
  df();
  const char* param = NULL;
  size_t size = g_variant_n_children(parameters);
  if (size > 0) {
    g_variant_get_child(parameters, 0, "s", &param);
  }
  hook_perform_signal(context->hook, context->lua, param);
}

SignalDef signals[] = {
  { "hook", ipc_signal_hook, },
  { NULL, NULL, }
};

void ipc_method_get_ids(Context* context, GVariant* parameters, GDBusMethodInvocation* invocation)
{
  GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE_ARRAY);
  for (GList* li = app->contexts; li != NULL; li = li->next) {
    Context* c = (Context*)li->data;
    if (c->id < 0) {
      continue;
    }
    g_variant_builder_add(builder, "i", c->id);
  }
  GVariant* v = g_variant_builder_end(builder);
  v = g_variant_new_tuple(&v, 1);
  g_dbus_method_invocation_return_value(invocation, v);
}

void ipc_method_echo(Context* context, GVariant* parameters, GDBusMethodInvocation* invocation)
{
  g_dbus_method_invocation_return_value(invocation, parameters);
}

void ipc_method_eval(Context* context, GVariant* parameters, GDBusMethodInvocation* invocation)
{
  lua_State* L = context->lua;
  char* param = NULL;
  g_variant_get_child(parameters, 0, "s", &param);

  char* result = NULL;
  if (luaL_dostring(L, param) != 0) {
    result = g_strdup(lua_tostring(L, -1));
    context_log_warn(context, true, result);
    lua_pop(L, -1);
  } else {
    int top = lua_gettop(L);
    if (top == 1) {
      result = g_strdup(lua_tostring(L, -1));
    } else {
      result = g_strdup_printf("Stack top is not 1(top = %d)", top);
    }
  }

  GVariant* v = g_variant_new("(s)", result);
  g_free(result);
  g_dbus_method_invocation_return_value(invocation, v);
}

void ipc_method_exec(Context* context, GVariant* parameters, GDBusMethodInvocation* invocation)
{
  char* param = NULL;
  g_variant_get_child(parameters, 0, "s", &param);
  if (luaL_dostring(context->lua, param) != 0) {
    context_log_warn(context, true, lua_tostring(context->lua, -1));
    lua_pop(context->lua, -1);
  }
  GVariant* v = g_variant_new("()");
  g_dbus_method_invocation_return_value(invocation, v);
}

void ipc_method_exec_file(Context* context, GVariant* parameters, GDBusMethodInvocation* invocation)
{
  char* param = NULL;
  g_variant_get_child(parameters, 0, "s", &param);
  if (luaL_dofile(context->lua, param) != 0) {
    context_log_warn(context, true, lua_tostring(context->lua, -1));
    lua_pop(context->lua, -1);
  }
  GVariant* v = g_variant_new("()");
  g_dbus_method_invocation_return_value(invocation, v);
}

MethodDef methods[] = {
  { "echo",      ipc_method_echo, },
  { "get_ids",   ipc_method_get_ids, },
  { "eval",      ipc_method_eval, },
  { "exec",      ipc_method_exec, },
  { "exec_file", ipc_method_exec_file, },
  { NULL, NULL, }
};

IPC* ipc_init()
{
  IPC* ipc = g_malloc0(sizeof(IPC));
  ipc->signals = g_hash_table_new_full(
    g_str_hash,
    g_str_equal,
    NULL,
    NULL
  );

  ipc->methods = g_hash_table_new_full(
    g_str_hash,
    g_str_equal,
    NULL,
    NULL
  );

  int i = 0;
  while (signals[i].signal_name) {
    SignalDef* d = &signals[i];
    g_hash_table_insert(ipc->signals, (void*)d->signal_name, d);
    i += 1;
  }

  i = 0;
  while (methods[i].method_name) {
    MethodDef* d = &methods[i];
    g_hash_table_insert(ipc->methods, (void*)d->method_name, d);
    i += 1;
  }
  return ipc;
}

void ipc_close(IPC* ipc)
{
  g_hash_table_destroy(ipc->signals);
  g_hash_table_destroy(ipc->methods);
  g_free(ipc);
}

bool ipc_signal_perform(IPC* ipc, Context* context, const char* signal_name, GVariant* parameters)
{
  SignalDef* d = g_hash_table_lookup(ipc->signals, signal_name);
  if (!d) {
    dd("invalid signal_name: '%s'", signal_name);
    return false;
  }
  d->func(context, parameters);
  return true;
}

bool ipc_method_perform(IPC* ipc, Context* context, const char* method_name, GVariant* parameters, GDBusMethodInvocation* invocation)
{
  MethodDef* d = g_hash_table_lookup(ipc->methods, method_name);
  if (!d) {
    dd("invalid method_name: '%s'", method_name);
    return false;
  }
  d->func(context, parameters, invocation);
  return true;
}
