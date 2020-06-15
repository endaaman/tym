/**
 * tym-test.c
 *
 * Copyright (c) 2019 endaaman, iTakeshi
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "test.h"

void test_hoge() {
  assert(0 == 0);
}

int main(int argc, char* argv[])
{
  g_test_init(&argc, &argv, NULL);
  g_test_add_func("/tym/regex", test_regex);
  return g_test_run();
}
