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

typedef struct {
  GApplication* gapp;
  Meta* meta;
  GList* contexts;
} App;

extern App* app;

void app_init();
void app_close();
int app_start(int argc, char **argv);

#endif
