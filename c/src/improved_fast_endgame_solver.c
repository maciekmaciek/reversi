/**
 * @file
 *
 * @brief Improved fast endgame solver.
 * @details Solver derived from the Gunnar Andersson work.
 *
 * @par improved_fast_endgame_solver.c
 * <tt>
 * This file is part of the reversi program
 * http://github.com/rcrr/reversi
 * </tt>
 * @author Roberto Corradini mailto:rob_corradini@yahoo.it
 * @copyright 2013 Roberto Corradini. All rights reserved.
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

#include "improved_fast_endgame_solver.h"

/**
 * @enum IFES_SquareState
 * @brief The `IFES_SquareState` identifies the state, or as a synonym the "color",
 * of each board square.
 */
typedef enum {
  IFES_WHITE,   /**< A white piece. */
  IFES_EMPTY,   /**< An empty square. */
  IFES_BLACK,   /**< A black piece. */
  IFES_DUMMY    /**< A piece out of board. */
} IFES_SquareState;

/**
 * @typedef uchar
 * @brief Unsigned one byte integer.
 */
typedef unsigned char uchar;

/**
 * @typedef schar
 * @brief Signed one byte integer.
 */
typedef signed char schar;

/**
 * @typedef uint
 * @brief Unigned four byte integer.
 */
typedef signed char uint;

/**
 * @brief An empty list collects a set of ordered squares.
 *
 * Details to be documented.
 */
typedef struct em_list {
  int square;           /**< @brief To be documented. */
  int hole_id;
  struct em_list *pred;
  struct em_list *succ;
} EmList;



/*
 * Prototypes for internal functions.
 */

inline static void
directional_flips (uchar *sq, int inc, int color, int oppcol);

static int
do_flips (uchar *board, int sqnum, int color, int oppcol);

inline static int
ct_directional_flips (uchar *sq, int inc, int color, int oppcol);

static int
count_flips (uchar *board, int sqnum, int color, int oppcol);

inline static int
any_directional_flips (uchar *sq, int inc, int color, int oppcol);

static int
any_flips (uchar *board, int sqnum, int color, int oppcol);

inline static void
undo_flips (int FlipCount, int oppcol);

inline static uint
minu (uint a, uint b);

static int
count_mobility (uchar *board, int color);

static void
prepare_to_solve (uchar *board);

static int
no_parity_end_solve (uchar *board, double alpha, double beta, 
                     int color, int empties, int discdiff, int prevmove);

static int
parity_end_solve (uchar *board, double alpha, double beta, 
                  int color, int empties, int discdiff, int prevmove);

static int
fastest_first_end_solve (uchar *board, double alpha, double beta, 
                         int color, int empties, int discdiff,
                         int prevmove);

static int
end_solve (uchar *board, double alpha, double beta, 
           int color, int empties, int discdiff, int prevmove);

static void
game_position_to_ifes_board (const GamePosition * const gp, int *p_emp, int *p_wc, int *p_bc);

static IFES_SquareState
game_position_get_ifes_player(const GamePosition * const gp);



/*
 * Internal constants.
 */

/*
 * This code will work for any number of empties up to the number
 * of bits in a uint (should be 32).
 *
 * The function prepare_to_solve aborts if empties are more than max_empties.
 */
static const int max_empties = 32;

/*
 * This is the best/worst case board value.
 * Any value greather than 64 can be adopted.
 * The value thirty thousands has an hystorical heritage. 
 */
static const int infinity = 30000;

/*
 * It is plain alphabeta, no transposition table.  It can be used for
 * WLD solve by setting alpha=-1 and beta=1 or for full solve with 
 * alpha=-64, beta=64. It uses a fixed preference
 * ordering of squares. This is not such a bad thing to do when <=10
 * empties, where the demand for speed is paramount. When use_parity=X is
 * turned on (X>0), it also uses parity to help with move ordering,
 * specifically it will consider all moves into odd regions before
 * considering any moves into even regions, in situations with more than
 * X empties.
 */
static const int use_parity = 4;

/*
 * In positions with <= fastest_first empty, fastest first is disabled.
 */
static const int fastest_first = 7;

/*
 * The 8 legal directions, plus no direction an ninth value.
 */
static const schar dirinc[] = {1, -1, 8, -8, 9, -9, 10, -10, 0};

/*
 * Fixed square ordering.
 */
static const int worst_to_best[64] =
{
  /*B2*/      20, 25, 65, 70,
  /*B1*/      11, 16, 19, 26, 64, 71, 74, 79,
  /*C2*/      21, 24, 29, 34, 56, 61, 66, 69,
  /*D2*/      22, 23, 38, 43, 47, 52, 67, 68,
  /*D3*/      31, 32, 39, 42, 48, 51, 58, 59,
  /*D1*/      13, 14, 37, 44, 46, 53, 76, 77,
  /*C3*/      30, 33, 57, 60,
  /*C1*/      12, 15, 28, 35, 55, 62, 75, 78,
  /*A1*/      10, 17, 73, 80, 
  /*D4*/      40, 41, 49, 50
};

/*
 * The bit mask for direction i is 1<<i
 *
 * Bit masks for the directions squares can flip in,
 * for example dirmask[10]=81=64+16+1=(1<<6)+(1<<4)+(1<<0)
 * hence square 10 (A1) can flip in directions dirinc[0]=1,
 * dirinc[4]=9, and dirinc[6]=10:
 */
static const uchar dirmask[91] = {
  0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,  81,  81,  87,  87,  87,  87,  22,  22,
  0,  81,  81,  87,  87,  87,  87,  22,  22,
  0, 121, 121, 255, 255, 255, 255, 182, 182,
  0, 121, 121, 255, 255, 255, 255, 182, 182,
  0, 121, 121, 255, 255, 255, 255, 182, 182,
  0, 121, 121, 255, 255, 255, 255, 182, 182,
  0,  41,  41, 171, 171, 171, 171, 162, 162,
  0,  41,  41, 171, 171, 171, 171, 162, 162,
  0,   0,   0,   0,   0,   0,   0,   0,   0, 0
};



/*
 * Internal variables.
 */

/* leaf count, should be part of the solver. */
static uint64 leaf_count = 0;

/* node count, should be part of the solver. */
static uint64 node_count = 0;

/*
 * Inside this fast endgame solver, the board is represented by
 * a 1D array of 91 uchars board[0..90]:
 * ddddddddd
 * dxxxxxxxx
 * dxxxxxxxx
 * dxxxxxxxx
 * dxxxxxxxx
 * dxxxxxxxx
 * dxxxxxxxx
 * dxxxxxxxx       where A1 is board[10], H8 is board[80].
 * dxxxxxxxx       square(a,b) = board[10+a+b*9] for 0<= a,b <=7.
 * dddddddddd   
 * where d (dummy) squares contain DUMMY, x are EMPTY, BLACK, or WHITE:
 */
static uchar board[91];

/*
 * Also there is a doubly linked list of the empty squares.
 * EmHead points to the first empty square in the list (or NULL if none).
 * The list in maintained in a fixed best-to-worst order.
 */
static EmList EmHead, Ems[64];

/*
 * Also, and finally, each empty square knows the region it is in
 * and knows the directions you can flip in via some bit masks.
 * There are up to 32 regions. The parities of the regions are in
 * the RegionParity bit vector:
 */
static uint HoleId[91];
static uint RegionParity;

/* Must be documented. */
static uchar  *GlobalFlipStack[2048];

/* Must be documented. */
static uchar **FlipStack = &(GlobalFlipStack[0]);



/*********************************************************/
/* Function implementations for the GamePosition entity. */ 
/*********************************************************/

/**
 * @brief Documentation to be prepared.
 */
ExactSolution *
game_position_ifes_solve (const GamePosition * const root)
{
  ExactSolution *result;
  int            val;
  int            emp;
  int            wc, bc;

  result = exact_solution_new();
  result->solved_game_position = game_position_clone(root);

  game_position_to_ifes_board(root, &emp, &wc, &bc);

  IFES_SquareState player = game_position_get_ifes_player(root);

  prepare_to_solve(board);

  val = end_solve(board, -64, 64, player, emp, -wc+bc, 1);

  result->outcome = val;
  result->leaf_count = leaf_count;
  result->node_count = node_count;

  printf("use_parity=%d. fastest_first=%d.\n",
         use_parity, fastest_first);

  return result;
}



/*
 * Internal functions.
 */

static void
game_position_to_ifes_board (const GamePosition * const gp, int *p_emp, int *p_wc, int *p_bc)
{
  int emp, wc, bc, j, k, x, y;
  for (j = 0; j <= 90; j++) board[j] = IFES_DUMMY;
  wc = bc = emp = 0;
  for (j = 0; j < 64; j++) {
    x = j&7; y = (j>>3)&7; k = x+10+9*y;
    if      ((gp->board->whites & (1ULL << j)) != 0ULL) { board[k] = IFES_WHITE; wc++; }
    else if ((gp->board->blacks & (1ULL << j)) != 0ULL) { board[k] = IFES_BLACK; bc++; }
    else                                                { board[k] = IFES_EMPTY; emp++; }
  }
  *p_emp = emp;
  *p_wc = wc;
  *p_bc = bc;
}

static IFES_SquareState
game_position_get_ifes_player(const GamePosition * const gp)
{
  IFES_SquareState ifes_player;
  ifes_player = gp->player == BLACK_PLAYER ? IFES_BLACK : IFES_WHITE;
  return ifes_player;
}

/**
 * @brief Documentation to be prepared.
 *
 * sq is a pointer to the square the move is to.
 * inc is the increment to go in some direction.
 * color is the color of the mover.
 * oppcol = 2-color is the opposite color.
 * FlipStack records locations of flipped men so can unflip later.
 * This routine flips in direction inc and returns count of flips it made:
 */
inline static void
directional_flips (uchar *sq, int inc, int color, int oppcol)
{
  uchar *pt = sq + inc;
  if (*pt == oppcol) {
    pt += inc;
    if (*pt == oppcol) {
      pt += inc;
      if (*pt == oppcol) {
        pt += inc;
        if (*pt == oppcol) {
          pt += inc;
          if (*pt == oppcol) {
            pt += inc;
            if (*pt == oppcol) {
              pt += inc;
            }
          }
        }
      }
    }
    if (*pt == color) {
      pt -= inc;
      do {
        *pt = color;
        *(FlipStack++) = pt;
        pt -= inc;
      } while (pt != sq);
    }
  }
}

/**
 * @brief Do all flips involved in making a move to square sqnum of board,
 * and return their count.
 */
static int
do_flips (uchar *board, int sqnum,
          int color, int oppcol)
{
  int j = dirmask[sqnum];
  uchar **OldFlipStack = FlipStack;
  uchar *sq;
  sq = sqnum + board;
  if (j & 128)
    directional_flips(sq, dirinc[7], color, oppcol);
  if (j & 64)
    directional_flips(sq, dirinc[6], color, oppcol);
  if (j & 32)
    directional_flips(sq, dirinc[5], color, oppcol);
  if (j & 16)
    directional_flips(sq, dirinc[4], color, oppcol);
  if (j & 8)
    directional_flips(sq, dirinc[3], color, oppcol);
  if (j & 4)
    directional_flips(sq, dirinc[2], color, oppcol);
  if (j & 2)
    directional_flips(sq, dirinc[1], color, oppcol);
  if (j & 1)
    directional_flips(sq, dirinc[0], color, oppcol);

  return FlipStack - OldFlipStack;
}

/**
 * @brief For the last move, we compute the score without updating the board:
 */
inline static int
ct_directional_flips (uchar *sq, int inc, int color, int oppcol)
{
  uchar *pt = sq + inc;
  if (*pt == oppcol) {
    int count = 1;
    pt += inc;
    if (*pt == oppcol) {
      count++;                /* 2 */
      pt += inc;
      if (*pt == oppcol) {
        count++;              /* 3 */
        pt += inc;
        if (*pt == oppcol) {
          count++;            /* 4 */
          pt += inc;
          if (*pt == oppcol) {
            count++;          /* 5 */
            pt += inc;
            if (*pt == oppcol) {
              count++;        /* 6 */
              pt += inc;
            }
          }
        }
      }
    }
    if (*pt == color) return count;
  }
  return 0;
}

static int
count_flips (uchar *board, int sqnum, int color, int oppcol)
{
  int ct = 0;
  int j = dirmask[sqnum];
  uchar *sq;
  sq = sqnum + board;
  if (j & 128)
    ct += ct_directional_flips(sq, dirinc[7], color, oppcol);
  if (j & 64)
    ct += ct_directional_flips(sq, dirinc[6], color, oppcol);
  if (j & 32)
    ct += ct_directional_flips(sq, dirinc[5], color, oppcol);
  if (j & 16)
    ct += ct_directional_flips(sq, dirinc[4], color, oppcol);
  if (j & 8)
    ct += ct_directional_flips(sq, dirinc[3], color, oppcol);
  if (j & 4)
    ct += ct_directional_flips(sq, dirinc[2], color, oppcol);
  if (j & 2)
    ct += ct_directional_flips(sq, dirinc[1], color, oppcol);
  if (j & 1)
    ct += ct_directional_flips(sq, dirinc[0], color, oppcol);
  return(ct);
}

/**
 * @brief Sometimes we only want to know if a move is legal, not how
 * many discs it flips.
 */
inline static int
any_directional_flips (uchar *sq, int inc, int color, int oppcol)
{
  uchar *pt = sq + inc;
  if (*pt == oppcol) {
    pt += inc;
    if (*pt == oppcol) {
      pt += inc;
      if (*pt == oppcol) {
        pt += inc;
        if (*pt == oppcol) {
          pt += inc;
          if (*pt == oppcol) {
            pt += inc;
            if (*pt == oppcol) {
              pt += inc;
            }
          }
        }
      }
    }
    if(*pt == color) return 1;
  }
  return 0;
}

/**
 * @brief To be documented
 */
static int
any_flips (uchar *board, int sqnum, int color, int oppcol)
{
  int any = 0;
  int j = dirmask[sqnum];
  uchar *sq;
  sq = sqnum + board;
  if (j & 128)
    any += any_directional_flips(sq, dirinc[7], color, oppcol);
  if (j & 64)
    any += any_directional_flips(sq, dirinc[6], color, oppcol);
  if (j & 32)
    any += any_directional_flips(sq, dirinc[5], color, oppcol);
  if (j & 16)
    any += any_directional_flips(sq, dirinc[4], color, oppcol);
  if (j & 8)
    any += any_directional_flips(sq, dirinc[3], color, oppcol);
  if (j & 4)
    any += any_directional_flips(sq, dirinc[2], color, oppcol);
  if (j & 2)
    any += any_directional_flips(sq, dirinc[1], color, oppcol);
  if (j & 1)
    any += any_directional_flips(sq, dirinc[0], color, oppcol);
  return any;
}

/**
 * @brief Call this function right after `FlipCount = do_flips()` to undo those flips!
 */
inline static void
undo_flips (int FlipCount, int oppcol)
{
  /*
   * This is functionally equivalent to the simpler but slower code line:
   * while(FlipCount){ FlipCount--;  *(*(--FlipStack)) = oppcol; }
   */
  if (FlipCount & 1) {
    FlipCount--;
    * (*(--FlipStack)) = oppcol;
  }
  while (FlipCount) {
    FlipCount -= 2;
    * (*(--FlipStack)) = oppcol;
    * (*(--FlipStack)) = oppcol;
  }
}

/**
 * @brief To be documented
 */
inline static uint
minu (uint a, uint b)
{
  if(a<b) return a;
  return b;
}

/**
 * @brief To be documented
 */
static int
count_mobility (uchar *board, int color)
{
  int oppcol = 2 - color;
  int mobility;
  int square;
  EmList *em;

  mobility = 0;
  for (em = EmHead.succ; em != NULL; em = em->succ) {
    square = em->square;
    if (any_flips(board, square, color, oppcol))
      mobility++;
  }

  return mobility;
}

/**
 * @brief Set up the data structures, other than board array,
 * which will be used by solver. Since this routine consumes
 * about 0.0002 of the time needed for a 12-empty solve,
 * I haven't worried about speeding it up.
 */
static void
prepare_to_solve (uchar *board)
{
  int i, sqnum;
  uint k;
  EmList *pt;
  int z;
  /* find hole IDs: */
  k = 1;
  for (i = 10; i <= 80; i++) {
    if (board[i] == IFES_EMPTY) {
      if (board[i-10] == IFES_EMPTY) HoleId[i] = HoleId[i-10];
      else if (board[i - 9] == IFES_EMPTY) HoleId[i] = HoleId[i - 9];
      else if (board[i - 8] == IFES_EMPTY) HoleId[i] = HoleId[i - 8];
      else if (board[i - 1] == IFES_EMPTY) HoleId[i] = HoleId[i - 1];
      else { HoleId[i] = k; k<<=1; }
    }
    else HoleId[i] = 0;
  }

  /* In some sense this is wrong, since you
   * ought to keep doing iters until reach fixed point, but in most
   * othello positions with few empties this ought to work, and besides,
   * this is justifiable since the definition of "hole" in othello
   * is somewhat arbitrary anyway. */
  const int max_iters = 1;

  for (z = max_iters; z > 0; z--) {
    for (i = 80; i >= 10; i--) {
      if (board[i] == IFES_EMPTY) {
        k = HoleId[i];
        if (board[i +10] == IFES_EMPTY) HoleId[i] = minu(k,HoleId[i +10]);
        if (board[i + 9] == IFES_EMPTY) HoleId[i] = minu(k,HoleId[i + 9]);
        if (board[i + 8] == IFES_EMPTY) HoleId[i] = minu(k,HoleId[i + 8]);
        if (board[i + 1] == IFES_EMPTY) HoleId[i] = minu(k,HoleId[i + 1]);
      }
    }
    for (i = 10; i <= 80; i++) {
      if (board[i] == IFES_EMPTY) {
        k = HoleId[i];
        if (board[i - 10] == IFES_EMPTY) HoleId[i] = minu(k,HoleId[i - 10]);
        if (board[i -  9] == IFES_EMPTY) HoleId[i] = minu(k,HoleId[i -  9]);
        if (board[i -  8] == IFES_EMPTY) HoleId[i] = minu(k,HoleId[i -  8]);
        if (board[i -  1] == IFES_EMPTY) HoleId[i] = minu(k,HoleId[i -  1]);
      }
    }
  }
  /* find parity of holes: */
  RegionParity = 0;
  for (i = 10; i <= 80; i++){
    RegionParity ^= HoleId[i];
  }
  /* create list of empty squares: */
  k = 0;
  pt = &EmHead;
  for (i = 60-1; i >= 0; i--){
    sqnum = worst_to_best[i];
    if (board[sqnum] == IFES_EMPTY) {
      pt->succ = &(Ems[k]);
      Ems[k].pred = pt;
      k++;
      pt = pt->succ;
      pt->square = sqnum;
      pt->hole_id = HoleId[sqnum];
    }
  }
  pt->succ = NULL;
  if (k > max_empties) abort(); /* better not have too many empties... */
}

/**
 * @brief To be documented
 */
static int
no_parity_end_solve (uchar *board, double alpha, double beta, 
                     int color, int empties, int discdiff, int prevmove)
{
  node_count++;

  int score = -infinity;
  int oppcol = 2-color;
  int sqnum,j,ev;
  EmList *em, *old_em;
  for (old_em = &EmHead, em = old_em->succ; em != NULL;
      old_em = em, em = em->succ){
    /* go thru list of possible move-squares */
    sqnum = em->square;
    j = do_flips(board, sqnum, color, oppcol );
    if (j) { /* legal move */
      /* place your disc: */
      *(board+sqnum) = color;
      /* delete square from empties list: */
      old_em->succ = em->succ;
      if (empties == 2){ /* So, now filled but for 1 empty: */
        int j1;
        j1 = count_flips(board, EmHead.succ->square, oppcol, color);
        if (j1) { /* I move then he moves */
          ev = discdiff + 2*(j-j1);
        }
        else { /* he will have to pass */
          j1 = count_flips(board, EmHead.succ->square, color, oppcol);
          ev = discdiff + 2*j;
          if (j1) { /* I pass then he passes then I move */
            ev += 2 * (j1 + 1);
          }
          else { /* I move then both must pass, so game over */
            if (ev >= 0)
              ev += 2;
          }
        }
      }
      else {
        ev = -no_parity_end_solve(board, -beta, -alpha, 
                                  oppcol, empties-1, -discdiff-2*j-1, sqnum);
      }
      undo_flips(j, oppcol);
      /* un-place your disc: */
      *(board+sqnum) = IFES_EMPTY;
      /* restore deleted empty square: */
      old_em->succ = em;

      if (ev > score) { /* better move: */
        score = ev;
        if (ev > alpha) {
          alpha = ev;
          if (ev >= beta) { /* cutoff */
            return score;
          }
        }
      }
    }
  }
  if (score == -infinity) {  /* No legal move */
    if (prevmove == 0) { /* game over: */
      leaf_count++;
      if (discdiff > 0) return discdiff+empties;
      if (discdiff < 0) return discdiff-empties;
      return 0;
    }
    else /* I pass: */
      return -no_parity_end_solve(board, -beta, -alpha, oppcol, empties, -discdiff, 0);
  }
  return score;
}

/**
 * @brief To be documented
 */
static int
parity_end_solve (uchar *board, double alpha, double beta, 
                  int color, int empties, int discdiff, int prevmove)
{
  node_count++;

  int score = -infinity;
  int oppcol = 2-color;
  int sqnum,j,ev;
  EmList *em, *old_em;
  uint parity_mask;
  int par, holepar;

  for (par = 1, parity_mask = RegionParity; par >= 0;
       par--, parity_mask = ~parity_mask) {

    for (old_em = &EmHead, em = old_em->succ; em != NULL;
         old_em = em, em = em->succ){
      /* go thru list of possible move-squares */
      holepar = em->hole_id;
      if (holepar & parity_mask) {
        sqnum = em->square;
        j = do_flips(board, sqnum, color, oppcol);
        if (j) { /* legal move */
          /* place your disc: */
          *(board+sqnum) = color;
          /* update parity: */
          RegionParity ^= holepar;
          /* delete square from empties list: */
	  old_em->succ = em->succ;
          if (empties <= 1 + use_parity)
            ev = -no_parity_end_solve(board, -beta, -alpha, 
                                      oppcol, empties-1, -discdiff-2*j-1, sqnum);
          else
            ev = -parity_end_solve(board, -beta, -alpha, 
                                   oppcol, empties-1, -discdiff-2*j-1, sqnum);
          undo_flips(j, oppcol);
          /* restore parity of hole */
          RegionParity ^= holepar;
          /* un-place your disc: */
          *(board+sqnum) = IFES_EMPTY;
          /* restore deleted empty square: */
	  old_em->succ = em;

          if(ev > score){ /* better move: */
            score = ev;
            if(ev > alpha){
              alpha = ev;
              if(ev >= beta){ 
                return score;
              }
	    }
          }
        }
      }
    }
  }

  if (score == -infinity) {  /* No legal move found */
    if (prevmove == 0) { /* game over: */
      leaf_count++;
      if (discdiff > 0) return discdiff+empties;
      if (discdiff < 0) return discdiff-empties;
      return 0;
    }
    else /* I pass: */
      return -parity_end_solve(board, -beta, -alpha, oppcol, empties, -discdiff, 0);
  }
  return score;
}

/**
 * @brief To be documented
 */
static int
fastest_first_end_solve (uchar *board, double alpha, double beta, 
                         int color, int empties, int discdiff,
                         int prevmove)
{
  int i, j;
  int score = -infinity;
  int oppcol = 2 - color;
  int sqnum, ev;
  int flipped;
  int moves, mobility;
  int best_value, best_index;
  EmList *em, *old_em;
  EmList *move_ptr[64];
  int holepar;
  int goodness[64];

  node_count++;

  moves = 0;
  for (old_em = &EmHead, em = old_em->succ; em != NULL;
       old_em = em, em = em->succ ) {
    sqnum = em->square;
    flipped = do_flips(board, sqnum, color, oppcol );
    if (flipped) {
      board[sqnum] = color;
      old_em->succ = em->succ;
      mobility = count_mobility(board, oppcol);
      old_em->succ = em;
      undo_flips(flipped, oppcol);
      board[sqnum] = IFES_EMPTY;
      move_ptr[moves] = em;
      goodness[moves] = -mobility;
      moves++;
    }
  }

  if (moves != 0) {
    for (i = 0; i < moves; i++) {
      best_value = goodness[i];
      best_index = i;
      for (j = i + 1; j < moves; j++)
	if (goodness[j] > best_value) {
	  best_value = goodness[j];
	  best_index = j;
	}
      em = move_ptr[best_index];
      move_ptr[best_index] = move_ptr[i];
      goodness[best_index] = goodness[i];

      sqnum = em->square;
      holepar = em->hole_id;
      j = do_flips(board, sqnum, color, oppcol );
      board[sqnum] = color;
      RegionParity ^= holepar;
      em->pred->succ = em->succ;
      if (em->succ != NULL)
	em->succ->pred = em->pred;
      if (empties <= fastest_first + 1)
	ev = -parity_end_solve(board, -beta, -alpha, oppcol, empties - 1,
                               -discdiff - 2 * j - 1, sqnum);
      else
	ev = -fastest_first_end_solve(board, -beta, -alpha, oppcol,
                                      empties - 1, -discdiff - 2 * j - 1,
                                      sqnum);
      undo_flips(j, oppcol);
      RegionParity ^= holepar;
      board[sqnum] = IFES_EMPTY;
      em->pred->succ = em;
      if (em->succ != NULL)
	em->succ->pred = em;

      if (ev > score) { /* better move: */
	score = ev;
	if (ev > alpha) {
	  alpha = ev;
	  if (ev >= beta) {
	    return score;
          }
	}
      }
    }
  }
  else {
    if (prevmove == 0) { // game-over
      leaf_count++;
      if (discdiff > 0)
	return discdiff + empties;
      if (discdiff < 0)
	return discdiff - empties;
      return 0;
    }
    else { /* I pass: */
      score = -fastest_first_end_solve(board, -beta, -alpha, oppcol,
                                       empties, -discdiff, 0);
    }
  }

  return score;
}

/**
 * @brief The search itself.
 * Assumes relevant data structures have been set up with prepare_to_solve().
 * color is the color on move. Discdiff is color disc count - opposite
 * color disc count. The value of this at the end of the game is returned.
 * prevmove==0 if previous move was a pass, otherwise non0.
 * empties>0 is number of empty squares.
 */
static int
end_solve (uchar *board, double alpha, double beta, 
           int color, int empties, int discdiff, int prevmove)
{
  if (empties > fastest_first)
    return fastest_first_end_solve(board,alpha,beta,color,empties,discdiff,prevmove);
  else {
    if (empties <= (2 > use_parity ? 2 : use_parity))
      return no_parity_end_solve(board,alpha,beta,color,empties,discdiff,prevmove);
    else
      return parity_end_solve(board,alpha,beta,color,empties,discdiff,prevmove);
  }
}
