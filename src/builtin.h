/**
 * builtin.h
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef BULTIN_H
#define BUILTIN_H

#include "common.h"
#include "context.h"

void builtin_reload(Context* context);
void builtin_copy_clipboard(Context* context);
void builtin_paste_clipboard(Context* context);
void builtin_increase_font_scale(Context* context);
void builtin_decrease_font_scale(Context* context);
void builtin_reset_font_scale(Context* context);

#endif
