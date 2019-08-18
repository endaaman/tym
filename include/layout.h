/**
 * layout.h
 *
 * Copyright (c) 2019 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef LAYOUT_H
#define LAYOUT_H

#include "common.h"
#include "config.h"


typedef struct {
  GtkWindow* window;
  VteTerminal* vte;
  GtkBox* hbox;
  GtkBox* vbox;
  int* uri_tag;
  bool alpha_supported;
} Layout;


Layout* layout_init();
void layout_close(Layout* layout);
void layout_build(Layout* layout, GApplication* app);

#endif
