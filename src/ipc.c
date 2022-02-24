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
  const char* param = NULL;
  size_t size = g_variant_n_children(parameters);
  if (size > 0) {
    g_variant_get_child(parameters, 0, "s", &param);
  }
  hook_perform_signal(context->hook, context->lua, param);
}

void ipc_signal_do(Context* context, GVariant* parameters)
{
   const char* src = "";
    size_t size = g_variant_n_children(parameters);
    if (size != 1) {
      context_log_warn(context, true, "`do` signal was sent, but param is not provided.");
      return;
    }
    g_variant_get_child(parameters, 0, "s", &src);
    if (luaL_dostring(context->lua, src) != 0) {
      context_log_warn(context, true, lua_tostring(context->lua, -1));
      lua_pop(context->lua, -1);
    }
}

SignalDef signals[] = {
  { "hook", ipc_signal_hook, },
  { "do",   ipc_signal_do, },
  { NULL, NULL, }
};

void ipc_method_echo(Context* context, GVariant* parameters, GDBusMethodInvocation* invocation)
{
}

MethodDef methods[] = {
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
    g_hash_table_insert(ipc->signals, (void*)d->method_name, d);
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

void ipc_signal_perform(IPC* ipc, Context* context, const char* signal_name, GVariant* parameters)
{
  SignalDef* d = g_hash_table_lookup(ipc->signals, signal_name);
  if (!d) {
    dd("invalid signal_name: '%s'", signal_name);
    return;
  }
  d->func(context, parameters);
}

void ipc_method_perform(IPC* ipc, Context* context, const char* method_name, GVariant* parameters, GDBusMethodInvocation* invocation)
{
  df();
}
