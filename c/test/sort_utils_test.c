/**
 * @file
 *
 * @brief Sort Utils unit test suite.
 * @details Collects tests and helper methods for the sort utils module.
 *
 * @par sort_utils_test.c
 * <tt>
 * This file is part of the reversi program
 * http://github.com/rcrr/reversi
 * </tt>
 * @author Roberto Corradini mailto:rob_corradini@yahoo.it
 * @copyright 2014 Roberto Corradini. All rights reserved.
 *
 * @par License
 * <tt>
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3, or (at your option) any
 * later version.
 * \n
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * \n
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 * or visit the site <http://www.gnu.org/licenses/>.
 * </tt>
 */

#include <stdlib.h>
#include <stdio.h>

#include <glib.h>

#include "sort_utils.h"
#include "random.h"


/* Test function prototypes. */

static void dummy_test (void);
static void sort_utils_heapsort_d_test (void);
static void sort_utils_heapsort_p_test (void);
static void sort_utils_smoothsort_test (void);


int
main (int   argc,
      char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func("/sort_utils/dummy", dummy_test);
  g_test_add_func("/sort_utils/sort_utils_heapsort_d_test", sort_utils_heapsort_d_test);
  g_test_add_func("/sort_utils/sort_utils_heapsort_p_test", sort_utils_heapsort_p_test);
  g_test_add_func("/sort_utils/sort_utils_smoothsort_test", sort_utils_smoothsort_test);

  return g_test_run();
}



/*
 * Test functions.
 */

static void
dummy_test (void)
{
  g_assert(TRUE);
}

static void
sort_utils_heapsort_d_test (void)
{
  double a[]        = { 7., 3., 9., 0., 1., 5., 2., 8., 4., 6. };
  double expected[] = { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9. };

  const int a_length = sizeof(a) / sizeof(a[0]);

  sort_utils_heapsort_d(a, a_length);

  for (int i = 0; i < a_length; i++) {
    g_assert_cmpfloat(expected[i], ==, a[i]);
  }
}

static void
sort_utils_heapsort_p_test (void)
{
  const int a_length = 1024;

  static const size_t size_of_pointer = sizeof(void *);

  void **a = (void *) malloc(a_length * size_of_pointer);
  g_assert(a);

  for (int i = 0; i < a_length; i++) {
    a[i] = a + i;
  }

  RandomNumberGenerator *rng = rng_new(175);
  rng_shuffle_array_p(rng, a, a_length);
  rng_free(rng);

  sort_utils_heapsort_p(a, a_length);

  for (int i = 0; i < a_length; i++) {
    g_assert(a[i] == &a[i]);
  }
}

static void
sort_utils_smoothsort_test (void)
{
  double a[]        = { 7., 3., 9., 0., 1., 5., 2., 8., 4., 6. };
  double expected[] = { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9. };

  const int a_length = sizeof(a) / sizeof(a[0]);

  sort_utils_smoothsort (a, a_length);

  for (int i = 0; i < a_length; i++) {
    g_assert_cmpfloat(expected[i], ==, a[i]);
  }
}
