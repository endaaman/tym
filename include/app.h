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


int on_command_line(GApplication* app, GApplicationCommandLine* cli, void* user_data);
void on_activate(GApplication* gapp, void* user_data);

#endif
