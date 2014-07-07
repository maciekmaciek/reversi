/**
 * @file
 *
 * @brief Exact solver module implementation.
 * @details It searches the end of the game for an exact outcome..
 *
 * @par exact_solver.c
 * <tt>
 * This file is part of the reversi program
 * http://github.com/rcrr/reversi
 * </tt>
 * @author Roberto Corradini mailto:rob_corradini@yahoo.it
 * @copyright 2013, 2014 Roberto Corradini. All rights reserved.
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

#include <glib.h>
#include <glib/gstdio.h>

#include "exact_solver.h"

/**
 * @brief Elements of a doubly linked list that collects moves.
 *
 * Elements have the square and mobility fields.
 */
typedef struct MoveListElement_ {
  Square                   sq;           /**< @brief The square field. */
  uint8                    mobility;     /**< @brief The mobility field. */
  struct MoveListElement_ *pred;         /**< @brief A pointer to the predecesor element. */
  struct MoveListElement_ *succ;         /**< @brief A pointer to the successor element. */
} MoveListElement;

/**
 * @brief Move list, having head, tail, and elements fields.
 *
 * Head and tail are not part of the list.
 */
typedef struct {
  MoveListElement elements[64];          /**< @brief Elements array. */
  MoveListElement head;                  /**< @brief Head element, it is not part of the list. */
  MoveListElement tail;                  /**< @brief Tail element, it is not part of the list. */
} MoveList;

/*
 * game_tree_log module - BEGIN.
 */

/**
 * @brief The log file used to record the game DAG traversing.
 */
static FILE *game_tree_log_file = NULL;

static void
game_tree_log_open (const gchar * const filename);

static void
game_tree_log_write (void);

static void
game_tree_log_close (void);

static void
game_tree_log_filename_check (const gchar * const filename);

static void
game_tree_log_dirname_recursive_check (const gchar * const filename);

void
game_tree_log_dirname_recursive_check (const gchar * const filename)
{
  const gchar * const dirname  = g_path_get_dirname(filename);
  if (!g_file_test(filename, G_FILE_TEST_EXISTS)) {
    game_tree_log_dirname_recursive_check(dirname);
    g_mkdir(filename, 0755);
  } else {
    if (g_file_test(filename, G_FILE_TEST_IS_REGULAR)) {
      printf("The given \"%s\" path contains an existing file! Exiting with status -102.\n", filename);
      exit(-102);
    }
  }
}

void
game_tree_log_filename_check (const gchar * const filename)
{
  const gchar * const dirname  = g_path_get_dirname(filename);
  if (!g_file_test(filename, G_FILE_TEST_EXISTS)) {
    game_tree_log_dirname_recursive_check(dirname);
  } else {
    if (g_file_test(filename, G_FILE_TEST_IS_REGULAR)) {
      printf("Logging regular file \"%s\" does exist, overwriting it.\n", filename);
    } else {
      printf("Logging file \"%s\" does exist, but it is a directory! Exiting with status -101.\n", filename);
      exit(-101);
    }
  }
}

/**
 * @brief Opens the file for logging and writes the header.
 *
 * @param [in] filename the name of the logging file
 */
void
game_tree_log_open (const gchar * const filename)
{
  game_tree_log_filename_check(filename);
  game_tree_log_file = fopen(filename, "w");
  fprintf(game_tree_log_file, "%s;%s;%s;%s;%s;%s;%s;%s\n",
          "SUB_RUN_ID",
          "CALL_ID",
          "HASH",
          "PARENT_HASH",
          "BLACKS",
          "WHITES",
          "PLAYER",
          "JSON_DOC");
}

/**
 * @brief Writes one record to the logging file.
 */
void
game_tree_log_write (void)
{
  ;
}

/**
 * @brief Closes the logging file.
 */
void
game_tree_log_close (void)
{
  fclose(game_tree_log_file);
}
/*
 * game_tree_log module - END.
 */

/*
 * Prototypes for internal functions.
 */

static void
sort_moves_by_mobility_count (      MoveList     *       move_list,
                              const GamePosition * const gp);

static SearchNode *
game_position_solve_impl (      ExactSolution * const result,
                          const GamePosition  * const gp,
                          const int                   achievable,
                          const int                   cutoff);

static void
move_list_init (MoveList *ml);

/*
 * Internal variables and constants.
 */

/**
 * @brief True if the module logs to file.
 */
static gboolean log = FALSE;

/**
 * @brief The total number of call to the recursive function that traverse the game DAG.
 */
static uint64 call_count = 0;

/**
 * @brief The predecessor-successor array of game position hash values.
 */
static uint64 gp_hash_stack[128];

/**
 * @brief The index of the last entry into gp_hash_stack.
 */
static int gp_hash_stack_fill_point = 0;

/**
 * @brief The sub_run_id used for logging.
 */
static const int sub_run_id = 0;

/**
 * @brief Used d to sort the legal moves based on an heuristic knowledge.
 */
static const uint64 legal_moves_priority_mask[] = {
  /* D4, E4, E5, D5 */                 0x0000001818000000,
  /* A1, H1, H8, A8 */                 0x8100000000000081,
  /* C1, F1, F8, C8, A3, H3, H6, A6 */ 0x2400810000810024,
  /* C3, F3, F6, C6 */                 0x0000240000240000,
  /* D1, E1, E8, D8, A4, H4, H5, A5 */ 0x1800008181000018,
  /* D3, E3, E6, D6, C4, F4, F5, C5 */ 0x0000182424180000,
  /* D2, E2, E7, D7, B4, G4, G5, B5 */ 0x0018004242001800,
  /* C2, F2, F7, C7, B3, G3, G6, B6 */ 0x0024420000422400,
  /* B1, G1, G8, B8, A2, H2, H7, A7 */ 0x4281000000008142,
  /* B2, G2, G7, B7 */                 0x0042000000004200
};

/**
 * @brief The size of the legal_moves_priority_mask array.
 */
static const int legal_moves_priority_cluster_count =
  sizeof(legal_moves_priority_mask) / sizeof(legal_moves_priority_mask[0]);



/*******************************************************/
/* Function implementations for the SearchNode entity. */ 
/*******************************************************/

/**
 * @brief Search node structure constructor.
 *
 * @param [in] move  the move field
 * @param [in] value the value field
 * @return           a pointer to a new search node structure
 */
SearchNode *
search_node_new (const Square move, const int value)
{
  SearchNode * sn;
  static const size_t size_of_search_node = sizeof(SearchNode);

  sn = (SearchNode*) malloc(size_of_search_node);
  g_assert(sn);

  sn->move = move;
  sn->value = value;

  return sn;
}

/**
 * @brief Search node structure destructor.
 *
 * @invariant Parameter `sn` cannot be `NULL`.
 * The invariant is guarded by an assertion.
 *
 * @param [in] sn the pointer to be deallocated
 * @return        always the NULL pointer
 */
SearchNode *
search_node_free (SearchNode *sn)
{
  g_assert(sn);

  free(sn);
  sn = NULL;

  return sn;
}

/**
 * @brief Negate the value of the node.
 *
 * @param sn the search node to be negated
 * @return   a new node having the negated value
 */
SearchNode *
search_node_negated (SearchNode *sn)
{
  SearchNode *result;
  result = search_node_new(sn->move, -sn->value);
  search_node_free(sn);
  return result;
}



/**********************************************************/
/* Function implementations for the ExactSolution entity. */ 
/**********************************************************/

/**
 * @brief Exact solution structure constructor.
 *
 * @return a pointer to a new exact solution structure
 */
ExactSolution *
exact_solution_new (void)
{
  ExactSolution * es;
  static const size_t size_of_exact_solution = sizeof(ExactSolution);

  es = (ExactSolution*) malloc(size_of_exact_solution);
  g_assert(es);

  es->solved_game_position = NULL;
  es->outcome = 65;
  for (int i = 0; i < 60; i++) {
    es->principal_variation[i] = -1;
  }
  es->final_board = NULL;
  es->node_count = 0;
  es->leaf_count = 0;

  return es;
}

/**
 * @brief Exact solution structure destructor.
 *
 * @invariant Parameter `es` cannot be `NULL`.
 * The invariant is guarded by an assertion.
 *
 * @param [in] es the pointer to be deallocated
 * @return        always the NULL pointer
 */
ExactSolution *
exact_solution_free (ExactSolution *es)
{
  g_assert(es);

  if (es->solved_game_position) game_position_free(es->solved_game_position);
  if (es->final_board) board_free(es->final_board);

  free(es);
  es = NULL;

  return es;
}

/**
 * @brief Returns a formatted string describing the exact solution structure.
 *
 * @todo MUST BE COMPLETED!
 *
 * The returned string has a dynamic extent set by a call to malloc. It must then properly
 * garbage collected by a call to free when no more referenced.
 *
 * @invariant Parameter `es` must be not `NULL`.
 * Invariants are guarded by assertions.
 *
 * @param [in] es a pointer to the exact solution structure
 * @return        a string being a representation of the structure
 */
gchar *
exact_solution_print (const ExactSolution * const es)
{
  g_assert(es);

  gchar *es_to_string;

  es_to_string = game_position_print(es->solved_game_position);

  return es_to_string;
}



/*********************************************************/
/* Function implementations for the GamePosition entity. */ 
/*********************************************************/

/**
 * @brief Solves the game position returning a new exact solution pointer.
 *
 * @param [in] root     the starting game position to be solved
 * @param [in] log_flag true when logging is enabled
 * @return              a pointer to a new exact solution structure
 */
ExactSolution *
game_position_solve (const GamePosition * const root,
                     const gboolean             log_flag)
{
  ExactSolution *result; 
  SearchNode    *sn;

  log = log_flag;

  if (log) {
    gp_hash_stack[0] = 0; 
    game_tree_log_open("out/exact_solver_log.csv");
   /*
    game_tree_log_file = fopen("out/exact_solver_log.csv", "w");
    fprintf(game_tree_log_file, "%s;%s;%s;%s;%s;%s;%s;%s\n",
            "SUB_RUN_ID",
            "CALL_ID",
            "HASH",
            "PARENT_HASH",
            "BLACKS",
            "WHITES",
            "PLAYER",
            "JSON_DOC");
    */
  }

  result = exact_solution_new();

  result->solved_game_position = game_position_clone(root);

  sn = game_position_solve_impl(result, result->solved_game_position, -64, +64);

  if (sn) {
    result->principal_variation[0] = sn->move;
    result->outcome = sn->value;
  }
  sn = search_node_free(sn);

  if (log) {
    game_tree_log_close();
    //fclose(game_tree_log_file);
  }

  return result;
}



/*
 * Internal functions.
 */

static void
sort_moves_by_mobility_count (MoveList *move_list, const GamePosition * const gp)
{
  MoveListElement *curr = NULL;
  int move_index = 0;
  const SquareSet moves = game_position_legal_moves(gp);
  SquareSet moves_to_search = moves;
  for (int i = 0; i < legal_moves_priority_cluster_count; i++) {
    moves_to_search = legal_moves_priority_mask[i] & moves;
    while (moves_to_search) {
      curr = &move_list->elements[move_index];
      const Square move = bit_works_bitscanLS1B_64(moves_to_search);
      moves_to_search &= ~(1ULL << move);
      GamePosition *next_gp = game_position_make_move(gp, move);
      const SquareSet next_moves = game_position_legal_moves(next_gp);
      next_gp = game_position_free(next_gp);
      const int next_move_count = bit_works_popcount(next_moves);
      curr->sq = move;
      curr->mobility = next_move_count;
      for (MoveListElement *element = move_list->head.succ; element != NULL; element = element->succ) {
        if (curr->mobility < element->mobility) { /* Insert current before element. */
          MoveListElement *left  = element->pred;
          MoveListElement *right = element;
          curr->pred  = left;
          curr->succ  = right;
          left->succ  = curr;
          right->pred = curr;
          goto out;
        }
      }
    out:
      move_index++;
    }
  }
  return;
}

static SearchNode *
game_position_solve_impl (      ExactSolution * const result,
                          const GamePosition  * const gp,
                          const int                   achievable,
                          const int                   cutoff)
{
  result->node_count++;
  SearchNode *node  = NULL;
  SearchNode *node2 = NULL;

  if (log) {
    game_tree_log_write();
    call_count++;
    gp_hash_stack_fill_point++;
    const uint64 hash = game_position_hash(gp);
    gp_hash_stack[gp_hash_stack_fill_point] = hash;
    const sint64 hash_to_signed = (sint64) hash;
    const sint64 previous_hash_to_signed = (sint64) gp_hash_stack[gp_hash_stack_fill_point - 1];
    const Board  *current_board = gp->board;
    const sint64 *blacks_to_signed = (sint64 *) &current_board->blacks;
    const sint64 *whites_to_signed = (sint64 *) &current_board->whites;
    GString *json_doc;
    json_doc = g_string_sized_new(256);
    const gboolean is_leaf = !game_position_has_any_player_any_legal_move(gp);
    const SquareSet legal_moves = game_position_legal_moves(gp);
    const int legal_move_count = bit_works_popcount(legal_moves);
    const SquareSet empties = board_empties(gp->board);
    const int empty_count = bit_works_popcount(empties);
    const int legal_move_count_adj = legal_move_count + ((legal_moves == 0 && !is_leaf) ? 1 : 0);
    gchar *legal_moves_pg_json_array = square_set_to_pg_json_array(legal_moves);
    /*
     * cl:   call level
     * ec:   empty count
     * il:   is leaf
     * lmc:  legal move count
     * lmca: legal move count adjusted
     * lma:  legal move array ([""A1"", ""B4"", ""H8""])
     */
    g_string_append_printf(json_doc,
                           "\"{ \"\"cl\"\": %2d, \"\"ec\"\": %2d, \"\"il\"\": %s, \"\"lmc\"\": %2d, \"\"lmca\"\": %2d, \"\"lma\"\": %s }\"",
                           gp_hash_stack_fill_point,
                           empty_count,
                           is_leaf ? "true" : "false",
                           legal_move_count,
                           legal_move_count_adj,
                           legal_moves_pg_json_array);
    g_free(legal_moves_pg_json_array);
    fprintf(game_tree_log_file, "%6d;%8llu;%+20lld;%+20lld;%+20lld;%+20lld;%1d;%s\n",
            sub_run_id,
            call_count,
            hash_to_signed,
            previous_hash_to_signed,
            *blacks_to_signed,
            *whites_to_signed,
            gp->player,
            json_doc->str);
    g_string_free(json_doc, TRUE);
  }

  const SquareSet moves = game_position_legal_moves(gp);
  if (0ULL == moves) {
    GamePosition *flipped_players = game_position_pass(gp);
    if (game_position_has_any_legal_move(flipped_players)) {
      node = search_node_negated(game_position_solve_impl(result, flipped_players, -cutoff, -achievable));
    } else {
      result->leaf_count++;
      node = search_node_new((Square) -1, game_position_final_value(gp));
    }
    flipped_players = game_position_free(flipped_players);
  } else {
    MoveList move_list;
    move_list_init(&move_list);
    sort_moves_by_mobility_count(&move_list, gp);
    for (MoveListElement *element = move_list.head.succ; element != &move_list.tail; element = element->succ) {
      const Square move = element->sq;
      if (!node) node = search_node_new(move, achievable);
      GamePosition *gp2 = game_position_make_move(gp, move);
      node2 = search_node_negated(game_position_solve_impl(result, gp2, -cutoff, -node->value));
      gp2 = game_position_free(gp2);
      if (node2->value > node->value) {
        search_node_free(node);
        node = node2;
        node->move = move;
        node2 = NULL;
        if (node->value >= cutoff) goto out;
      } else {
        node2 = search_node_free(node2);
      }
    }
  }
 out:
  ;
  
  if (log) {
    gp_hash_stack_fill_point--;
  }

  return node;
}

static void
move_list_init (MoveList *ml)
{
  for (int i = 0; i < 64; i++) {
    ml->elements[i].sq = -1;
    ml->elements[i].mobility = -1;
    ml->elements[i].pred = NULL;
    ml->elements[i].succ = NULL;
  }
  ml->head.sq = -1;
  ml->head.mobility = -1;
  ml->head.pred = NULL;
  ml->head.succ = &ml->tail;
  ml->tail.sq = -1;
  ml->tail.mobility = -1;
  ml->tail.pred = &ml->head;
  ml->tail.succ = NULL;
}
