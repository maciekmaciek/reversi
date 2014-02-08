/**
 * @file
 *
 * @brief Minimax solver module implementation.
 * @details It searches the end of the game for an exact outcome using the MINIMAX algorithm.
 *
 * @par minimax_solver.c
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

#include <stdio.h>
#include <stdlib.h>

#include <glib.h>

#include "minimax_solver.h"

//#define GAME_TREE_DEBUG



/*
 * Prototypes for internal functions.
 */

static SearchNode *
game_position_solve_impl (      ExactSolution * const result,
                          const GamePosition  * const gp);



/*
 * Internal variables and constants.
 */

/* Used to sort the legal moves based on an heuristic knowledge. */
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

/* The size of the legal_moves_priority_mask array. */
static const int legal_moves_priority_cluster_count =
  sizeof(legal_moves_priority_mask) / sizeof(legal_moves_priority_mask[0]);

#ifdef GAME_TREE_DEBUG

/* The total number of call to the recursive function that traverse the game DAG. */
static uint64 call_count = 0;

/* The log file used to record the game DAG traversing. */
static FILE *game_tree_debug_file = NULL;

/* The predecessor-successor array of game position hash values. */
static uint64 gp_hash_stack[128];

/* The index of the last entry into gp_hash_stack. */
static int gp_hash_stack_fill_point = 0;

#endif



/*********************************************************/
/* Function implementations for the GamePosition entity. */ 
/*********************************************************/

/**
 * @brief Solves the game position returning a new exact solution pointer.
 *
 * @param [in] root the starting game position to be solved
 * @return          a pointer to a new exact solution structure
 */
ExactSolution *
game_position_minimax_solve (const GamePosition * const root)
{
  ExactSolution *result; 
  SearchNode    *sn;

#ifdef GAME_TREE_DEBUG
  gp_hash_stack[0] = 0;
  game_tree_debug_file = fopen("minimax_log.csv", "w");
  fprintf(game_tree_debug_file, "%s;%s;%s;%s;%s;%s;%s;%s\n", "CALL_ID", "HASH", "PARENT_HASH", "GAME_POSITION", "EMPTY_COUNT", "LEVEL", "IS_LEF", "MOVE_LIST");
#endif

  result = exact_solution_new();

  result->solved_game_position = game_position_clone(root);

  sn = game_position_solve_impl(result, result->solved_game_position);

  if (sn) {
    result->principal_variation[0] = sn->move;
    result->outcome = sn->value;
  }
  sn = search_node_free(sn);

#ifdef GAME_TREE_DEBUG
  fclose(game_tree_debug_file);
#endif

  return result;
}



/*
 * Internal functions.
 */

static SearchNode *
game_position_solve_impl (      ExactSolution * const result,
                          const GamePosition  * const gp)
{
  SearchNode *node;
  SearchNode *node2;

  node  = NULL;
  node2 = NULL;
  result->node_count++;

  const SquareSet moves = game_position_legal_moves(gp);

#ifdef GAME_TREE_DEBUG
  call_count++;
  gp_hash_stack_fill_point++;
  const SquareSet empties = board_empties(gp->board);
  const int empty_count = bit_works_popcount(empties);
  const uint64 hash = game_position_hash(gp);
  gp_hash_stack[gp_hash_stack_fill_point] = hash;
  gchar *gp_to_s = game_position_to_string(gp);
  const gboolean is_leaf = !game_position_has_any_player_any_legal_move(gp);
  gchar *ml_to_s = square_set_to_string(moves);
  fprintf(game_tree_debug_file, "%8lld;%016llx;%016llx;%s;%2d;%2d;%s;%42s\n",
          call_count,
          hash,
          gp_hash_stack[gp_hash_stack_fill_point - 1],
          gp_to_s,
          empty_count,
          gp_hash_stack_fill_point,
          is_leaf ? "t" : "f",
          ml_to_s);
  g_free(gp_to_s);
  g_free(ml_to_s);
#endif

  if (moves == empty_square_set) {
    GamePosition *flipped_players = game_position_pass(gp);
    if (game_position_has_any_legal_move(flipped_players)) {
      node = search_node_negated(game_position_solve_impl(result, flipped_players));
    } else {
      result->leaf_count++;
      node = search_node_new((Square) -1, game_position_final_value(gp));
    }
    flipped_players = game_position_free(flipped_players);
  } else {
    node = search_node_new(-1, -65);
    SquareSet remaining_moves = moves;
    while (remaining_moves) {
      const Square move = bit_works_bitscanLS1B_64(remaining_moves);
      remaining_moves ^= 1ULL << move;
      GamePosition *gp2 = game_position_make_move(gp, move);
      node2 = search_node_negated(game_position_solve_impl(result, gp2));
      gp2 = game_position_free(gp2);
      if (node2->value > node->value) {
        search_node_free(node);
        node = node2;
        node->move = move;
        node2 = NULL;
      } else {
        node2 = search_node_free(node2);
      }
    }
  }

#ifdef GAME_TREE_DEBUG
  gp_hash_stack_fill_point--;
#endif

  return node;
}
