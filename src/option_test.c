/**
 * option_test.c
 *
 * Copyright (c) 2022 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "tym_test.h"
#include "option.h"
#include "app.h"

static void test_parse()
{
  Meta* meta = meta_init();

  Option* option = option_init(meta_get_option_entries(meta));

  char* argv_base[] = {
    "tym",
    "-u", "config.lua",
    "--width", "123",
    "--autohide",
    NULL
  };
  int argc = sizeof(argv_base) / sizeof(char*) - 1;
  g_assert(option_parse(option, argc, argv_base));

  g_assert(is_equal("config.lua", option_get_str(option, "use")));

  g_assert(option_get_int(option, "width") == 123);

  g_assert(option_get_bool(option, "autohide") == true);

  g_assert(option_get_str(option, "theme") == NULL);
  g_assert(option_get_int(option, "height") == 0);
  g_assert_false(option_get_bool(option, "silent"));


  char** a = &argv_base[0];
  while (*a) {
    dd("ARG %s", *a);
    a++;
  }
  dd("ARGC: %d", argc);

  option_close(option);
}

void test_option()
{
  test_parse();
}
