/**
 * ipc.h
 *
 * Copyright (c) 2022 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef IPC_H
#define IPC_H

#include "context.h"

typedef struct {
  GHashTable* signals;
  GHashTable* methods;
} IPC;


IPC* ipc_init();
void ipc_close(IPC* ipc);
void ipc_signal_perform(IPC* ipc, Context* context, const char* signal_name, GVariant* parameters);
void ipc_method_perform(IPC* ipc, Context* context, const char* method_name, GVariant* parameters, GDBusMethodInvocation* invocation);


#endif
