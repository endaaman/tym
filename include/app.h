/**
 * app.h
 *
 * Copyright (c) 2019 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef APP_H
#define APP_H

#include "common.h"
#include "context.h"
#include "ipc.h"

typedef struct {
  GApplication* gapp;
  Meta* meta;
  IPC* ipc;
  GList* contexts;
} App;

extern App* app;

void app_init();
void app_close();
void app_quit_context(Context* context);
int app_start(int argc, char **argv);

#endif
