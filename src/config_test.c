/**
 * config_test.c
 *
 * Copyright (c) 2020 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "tym_test.h"
#include "config.h"

static void test_read_and_write()
{
  Config* c = config_init();

  c->locked = false;
  config_set_int(c, "int", 123);
  config_set_str(c, "str", "tym");
  config_set_bool(c, "bool", true);
  c->locked = true;

  g_assert_cmpint(config_get_int(c, "int"), ==, 123);
  g_assert_cmpstr(config_get_str(c, "str"), ==, "tym");
  g_assert_cmpuint(config_get_bool(c, "bool"), ==, true);
  config_close(c);
}

static void test_locked()
{
  Config* c = config_init();

  config_set_int(c, "int", 123);
  config_set_str(c, "str", "tym");
  config_set_bool(c, "bool", true);

  // can not save values
  g_assert_cmpint(config_get_int(c, "int"), ==, 0);
  g_assert_cmpstr(config_get_str(c, "str"), ==, "");
  g_assert_cmpuint(config_get_bool(c, "bool"), ==, false);
  config_close(c);
}

void test_config()
{
  test_read_and_write();
  test_locked();
}
