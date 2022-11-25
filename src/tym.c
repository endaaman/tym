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
  GOptionEntry* entries = meta_get_option_entries(app->meta);
  Option* option = option_init(entries);

  if (!option_parse(option, argc, argv)) {
    return 1;
  }

  if (option_get_bool(option, "version")) {
    g_print("version %s\n", PACKAGE_VERSION);
    return 0;
  }

  int exit_code = app_start(option, argc, argv);
  app_close();
  return exit_code;
}
