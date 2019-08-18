/**
 * property.h
 *
 * Copyright (c) 2019 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef PROPERTY_H
#define PROPERTY_H

#include "common.h"
#include "context.h"


typedef int (*PropertyIntGetter)(Context* context, const char* key);
typedef const char* (*PropertyStrGetter)(Context* context, const char* key);
typedef bool (*PropertyBoolGetter)(Context* context, const char* key);

typedef void (*PropertyIntSetter)(Context* context, const char* key, int value);
typedef void (*PropertyStrSetter)(Context* context, const char* key, const char* value);
typedef void (*PropertyBoolSetter)(Context* context, const char* key, bool value);

typedef void (*PropertyIntSetterWithExtra)(Context* context, const char* key, int value, void* extra);
typedef void (*PropertyStrSetterWithExtra)(Context* context, const char* key, const char* value, void* extra);
typedef void (*PropertyBoolSetterWithExtra)(Context* context, const char* key, bool value, void* extra);


// str
void setter_shell(Context* context, const char* key, const char* value);

void setter_term(Context* context, const char* key, const char* value);

const char* getter_title(Context* context, const char* key);
void setter_title(Context* context, const char* key, const char* value);

const char* getter_font(Context* context, const char* key);
void setter_font(Context* context, const char* key, const char* value);

const char* getter_icon(Context* context, const char* key);
void setter_icon(Context* context, const char* key, const char* value);

const char* getter_role(Context* context, const char* key);
void setter_role(Context* context, const char* key, const char* value);

const char* getter_cursor_shape(Context* context, const char* key);
void setter_cursor_shape(Context* context, const char* key, const char* value);

const char* getter_cursor_blink_mode(Context* context, const char* key);
void setter_cursor_blink_mode(Context* context, const char* key, const char* value);

const char* getter_cjk_width(Context* context, const char* key);
void setter_cjk_width(Context* context, const char* key, const char* value);


// int
int getter_width(Context* context, const char* key);
void setter_width(Context* context, const char* key, int value);

int getter_height(Context* context, const char* key);
void setter_height(Context* context, const char* key, int value);

int getter_scale(Context* context, const char* key);
void setter_scale(Context* context, const char* key, int value);

void setter_padding_horizontal(Context* context, const char* key, int value);

void setter_padding_vertical(Context* context, const char* key, int value);

int getter_scrollback_length(Context* context, const char* key);
void setter_scrollback_length(Context* context, const char* key, int value);


// bool
bool getter_silent(Context* context, const char* key);
void setter_silent(Context* context, const char* key, bool value);

bool getter_ignore_bold(Context* context, const char* key);
void setter_ignore_bold(Context* context, const char* key, bool value);

bool getter_autohide(Context* context, const char* key);
void setter_autohide(Context* context, const char* key, bool value);

// color
void setter_color_normal(Context* context, const char* key, const char* value);
void setter_color_window_background(Context* context, const char* key, const char* value);
void setter_color_background(Context* context, const char* key, const char* value);
void setter_color_foreground(Context* context, const char* key, const char* value);
void setter_color_bold(Context* context, const char* key, const char* value);
void setter_color_cursor(Context* context, const char* key, const char* value);
void setter_color_cursor_foreground(Context* context, const char* key, const char* value);
void setter_color_highlight(Context* context, const char* key, const char* value);
void setter_color_highlight_foreground(Context* context, const char* key, const char* value);


#endif
