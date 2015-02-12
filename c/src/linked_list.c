/**
 * @file
 *
 * @todo Program insert_before, insert_after, reverse,
 *       concat, last, nth, sort methods.
 *
 * @brief Linked list module implementation.
 *
 * @details This module defines a linked list implementation.
 *
 * @par linked_list.c
 * <tt>
 * This file is part of the reversi program
 * http://github.com/rcrr/reversi
 * </tt>
 * @author Roberto Corradini mailto:rob_corradini@yahoo.it
 * @copyright 2015 Roberto Corradini. All rights reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "linked_list.h"



/****************************************************/
/* Function implementations for the llist_t entity. */
/****************************************************/

/**
 * @brief Linked list structure constructor.
 *
 * An assertion checks that the received pointer to the allocated
 * linked list structure is not `NULL`.
 *
 * @return a pointer to a new linked list structure
 */
llist_t *
llist_new (void)
{
  llist_t *l;
  static const size_t size_of_t = sizeof(llist_t);
  l = (llist_t *) malloc(size_of_t);
  assert(l);
  l->fe = NULL;
  l->length = 0;
  return l;
}

/**
 * @brief Deallocates the memory previously allocated by a call to #llist_new,
 *        and frees also all the linked elements by recursevely calling #llist_elm_free.
 *
 * @details If a null pointer is passed as argument, no action occurs.
 *
 * @param [in,out] l the pointer to be deallocated
 */
void
llist_free (l)
     llist_t *l;
{
  if (!l) return;
  llist_elm_t *e = l->fe;
  while (e) {
    llist_elm_t *n = e->next;
    llist_elm_free(e);
    e = n;
  }
  free(l);
}

/**
 * @brief Adds a new element at the beginning of the linked list.
 *
 * @details A new element is created, having as data the parameter d, and
 *          as next the first element of the list, then it becomes the actual
 *          first element of the list.
 *
 * An assertion checks that the received pointer to the
 * linked list structure is not `NULL`.
 *
 * @param [in,out] l the linked list
 * @param [in]     d the data referenced by the added element
 */
void
llist_add (l, d)
     llist_t *const l;
     void *const d;
{
  assert(l);
  llist_elm_t *const e = llist_elm_new();
  e->data = d;
  e->next = l->fe;
  l->fe = e;
  l->length++;
}

/**
 * @brief Removes an element from the list.
 *
 * @details If two elements contain the same data, only the first is removed.
 *          If none of the elements contain the data, the list is unchanged.
 *
 * @param [in,out] l the linked list
 * @param [in]     d the data of the element to remove
 */
void
llist_remove (l, d)
     llist_t *const l;
     void *const d;
{
  assert(l);
  llist_elm_t **e = &(l->fe);
  while (*e) {
    if ((*e)->data == d) {
      *e = (*e)->next;
      l->length--;
      return;
    }
    e = &((*e)->next);
  }
}

/**
 * @brief Calls a function for each element of a list.
 *
 * @details The function applyed to each element receives a pointer
 *          to the data of the element and to the auxiliary data.
 *          The auxiliary data pointer is shared among all elements.
 *
 * @param [in,out] l        the linked list
 * @param [in]     fn       the function applyed on each element
 * @param [in,out] aux_data shared auxiliary data
 */
void
llist_foreach (l, fn, aux_data)
     llist_t *const l;
     void (* fn) (void *const elm_data,
                  void *const aux_data);
     void *const aux_data;
{
  assert(l);
  for (llist_elm_t *e = l->fe; e; e = e->next) {
    if (0) printf("llist_foreach: e=%p, e->data=%p, e->next=%p\n", (void *)e, (void *)e->data, (void *)e->next);
    fn(e->data, aux_data);
  }
}

/**
 * @brief Gets the element at the given position in a linked list.
 *
 * @param [in,out] l the linked list
 * @param [in]     n the position of the element, counting from 0
 * @return           the element, or NULL when n is off the end of the list
 */
llist_elm_t *
llist_nth (l, n)
     llist_t *const l;
     const size_t n;
{
  assert(l);
  if (n > l->length) return NULL;
  llist_elm_t *e;
  size_t i;
  for (e = l->fe, i = 0; e; e = e->next, i++) {
    if (i == n) return e;
  }
  abort(); /* Should never happens. */
}

/**
 * @brief Finds the element in a list which contains the given data.
 *
 * @details The pointer data in each element of the list is compared with
 *          the `d` parameter. The first element matching is returned, `NULL`
 *          when none matches.
 *          When the search is successful, and the parameter `i` is not `NULL`,
 *          the index of the matching element, starting from `0`, is returned
 *          in the value pointed by `i`.
 *
 * @param [in,out] l the linked list
 * @param [in]     d the element data to find
 * @param [out]    i the element index if the search is successful
 * @return           the found element or NULL
 */
llist_elm_t *
llist_find (l, d, i)
     llist_t *const l;
     void *const d;
     size_t *i;
{
  assert(l);
  llist_elm_t *e;
  size_t k;
  for (e = l->fe, k = 0; e; e = e->next, k++) {
    if (e->data == d) {
      if (i) *i = k;
      return e;
    }
  }
  return NULL;
}

/**
 * @brief Inserts a new element into the list at the given position.
 *
 * @details If the parameter `i` is larger than the number of elements in the list,
 *          the new element is added on to the end of the list.
 *
 * @param [in,out] l the linked list
 * @param [in]     d the element data to find
 * @param [in]     i the position to insert the element
 */
void
llist_insert_at_position (l, d, i)
     llist_t *const l;
     void *const d;
     const size_t i;
{
  assert(l);
  const size_t position = (i > l->length) ? l->length : i;
  llist_elm_t **e = &(l->fe);
  for (size_t k = 0; k < position; k++, e = &((*e)->next)) ;
  llist_elm_t *const new = llist_elm_new();
  new->data = d;
  new->next = *e;
  *e = new;
  l->length++;
}



/********************************************************/
/* Function implementations for the llist_elm_t entity. */
/********************************************************/

/**
 * @brief Element structure constructor.
 *
 * An assertion checks that the received pointer to the allocated
 * element is not `NULL`.
 *
 * @return a pointer to a new element structure
 */
llist_elm_t *
llist_elm_new (void)
{
  llist_elm_t *e;
  static const size_t size_of_t = sizeof(llist_elm_t);
  e = (llist_elm_t *) malloc(size_of_t);
  assert(e);
  e->data = NULL;
  e->next = NULL;
  return e;
  ;
}

/**
 * @brief Deallocates the memory previously allocated by a call to #llist_elm_new.
 *
 * @details If a null pointer is passed as argument, no action occurs.
 *
 * @param [in,out] e the pointer to be deallocated
 */
void
llist_elm_free (e)
     llist_elm_t *e;
{
  free(e);
}