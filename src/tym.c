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
  app_init();
  int exit_code = app_start(argc, argv);
  app_quit();
  return exit_code;
}
