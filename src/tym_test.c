/**
 * tym_test.c
 *
 * Copyright (c) 2019 endaaman, iTakeshi
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "tym_test.h"
#include "app.h"

int main(int argc, char* argv[])
{
  g_test_init(&argc, &argv, NULL);
  g_test_add_func("/tym/config", test_config);
  g_test_add_func("/tym/regex", test_regex);
  g_test_add_func("/tym/option", test_option);
  return g_test_run();
}
