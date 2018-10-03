/**
 * tym.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "tym.h"


int main(int argc, char* argv[])
{
  dd("start");
  Context* context = context_init();
  int exit_code =  context_start(context, argc, argv);
  context_close(context);
  return exit_code;
}
