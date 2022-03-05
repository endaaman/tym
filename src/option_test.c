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
    "-u", "hi",
    "--width", "123",
    "--autohide",
    NULL
  };
  int argc = sizeof(argv_base) / sizeof(char*) - 1;
  g_assert(option_parse(option, argc, argv_base));

  char* use = NULL;
  g_assert(option_get_str(option, "use", &use));
  g_assert(is_equal("hi", use));

  int width = -1;
  g_assert(option_get_int(option, "width", &width));
  g_assert(width == 123);

  bool autohide = false;
  g_assert(option_get_bool(option, "autohide", &autohide));
  g_assert(autohide);

  g_assert_false(option_get_str(option, "theme", NULL));
  g_assert_false(option_get_int(option, "height", NULL));
  g_assert_false(option_get_bool(option, "silent", NULL));


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
