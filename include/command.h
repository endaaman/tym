/**
 * commad.h
 *
 * Copyright (c) 2019 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef COMMAND_H
#define COMMAND_H

#include "common.h"
#include "context.h"


void command_reload(Context* context);
void command_reload_theme(Context* context);
void command_copy_selection(Context* context);
void command_paste(Context* context);

#endif
