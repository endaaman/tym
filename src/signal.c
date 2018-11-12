/**
 * signal.c
 *
 * Copyright (c) 2019 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "tym.h"


void signal_reload_theme(Context* context)
{
  context_load_theme(context);
  context_apply_theme(context);
}
