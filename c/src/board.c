/**
 * @file
 *
 * @todo Axis functions are used only internally. Should be removed from the public interface.
 *       But tests has to be mantained ..... (including the board.c file?).
 *
 * @brief Board module implementation.
 * @details This module defines functions for the #Player, #SquareState,
 * #Square, #SquareSet, #Board, #GamePosition, #Direction entities.
 *
 * @par board.c
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

#include "board.h"
#include "random.h"



/**
 * @cond
 */

/*
 * Prototypes for internal functions.
 */

static void
board_initialize_bitrow_changes_for_player_array (uint8_t *array);

static void
board_initialize_shift_square_set_by_amount_mask_array (SquareSet *array);

static SquareSet
direction_shift_back_square_set_by_amount (const Direction dir,
                                           const SquareSet squares,
                                           const int amount);



/*
 * Internal variables and constants.
 */

/* Array used for conversion between square/move and its string representation. */
static const gchar *const sq_to_s[66] = {
  "A1", "B1", "C1", "D1", "E1", "F1", "G1", "H1",
  "A2", "B2", "C2", "D2", "E2", "F2", "G2", "H2",
  "A3", "B3", "C3", "D3", "E3", "F3", "G3", "H3",
  "A4", "B4", "C4", "D4", "E4", "F4", "G4", "H4",
  "A5", "B5", "C5", "D5", "E5", "F5", "G5", "H5",
  "A6", "B6", "C6", "D6", "E6", "F6", "G6", "H6",
  "A7", "B7", "C7", "D7", "E7", "F7", "G7", "H7",
  "A8", "B8", "C8", "D8", "E8", "F8", "G8", "H8",
  "--", "NA"
};

/* A square set being all set with the exception of column A. */
static const SquareSet all_squares_except_column_a = 0xFEFEFEFEFEFEFEFE;

/* A square set being all set with the exception of column H. */
static const SquareSet all_squares_except_column_h = 0x7F7F7F7F7F7F7F7F;

/* A bitboard being set on column A. */
static const SquareSet column_a = 0x0101010101010101;

/* A bitboard being set on diagonal A1-H8. */
static const SquareSet diagonal_a1_h8 = 0x8040201008040201;

/* A bitboard being set on diagonal H1-A8. */
static const SquareSet diagonal_h1_a8 = 0x0102040810204080;

/* A bitboard having set squares B1 F1 A2 E2. */
static const SquareSet squares_b1_f1_a2_e2 = 0x1122;

/*
 * This array is an implementation of the precomputed table that contains the effects of moving
 * a piece in any of the eigth squares in a row.
 * The size is so computed:
 *  - there are 256 arrangments of player discs,
 *  - and 256 arrangements of opponent pieces,
 *  - the potential moves are 8.
 * So the array size is 256 * 256 * 8 = 524,288 Bytes = 512kB.
 * Not all the entries are legal! The first set of eigth bits and the second one (opponent row)
 * must not set the same position.
 *
 * The index of the array is computed by this formula:
 * index = playerRow | (opponentRow << 8) | (movePosition << 16);
 *
 * After initialization the array is never changed.
 */
static uint8_t bitrow_changes_for_player_array[256 * 256 * 8];

/*
 * This array is a precomputed table used by the direction_shift_square_set_by_amount function.
 * It has an entry for each couple Direction-Amount, amount having as range 0..7.
 *
 * The index of the array is computed by this formula:
 * index = amount | (direction << 3)
 */
static SquareSet shift_square_set_by_amount_mask_array[8 * 8];

/*
 * This array has sixtyfour entries. The index, having range 0-63, represent one of the squares
 * of the table. Each entry is a bitboard mask having set all the squares that are
 * reachable moving along the eigth directions, when starting from the square identified by
 * the index itself.
 *
 * Values do not change.
 */
static const SquareSet bitboard_mask_for_all_directions[] = {
  0x81412111090503FE, 0x02824222120A07FD, 0x0404844424150EFB, 0x08080888492A1CF7,
  0x10101011925438EF, 0x2020212224A870DF, 0x404142444850E0BF, 0x8182848890A0C07F,
  0x412111090503FE03, 0x824222120A07FD07, 0x04844424150EFB0E, 0x080888492A1CF71C,
  0x101011925438EF38, 0x20212224A870DF70, 0x4142444850E0BFE0, 0x82848890A0C07FC0,
  0x2111090503FE0305, 0x4222120A07FD070A, 0x844424150EFB0E15, 0x0888492A1CF71C2A,
  0x1011925438EF3854, 0x212224A870DF70A8, 0x42444850E0BFE050, 0x848890A0C07FC0A0,
  0x11090503FE030509, 0x22120A07FD070A12, 0x4424150EFB0E1524, 0x88492A1CF71C2A49,
  0x11925438EF385492, 0x2224A870DF70A824, 0x444850E0BFE05048, 0x8890A0C07FC0A090,
  0x090503FE03050911, 0x120A07FD070A1222, 0x24150EFB0E152444, 0x492A1CF71C2A4988,
  0x925438EF38549211, 0x24A870DF70A82422, 0x4850E0BFE0504844, 0x90A0C07FC0A09088,
  0x0503FE0305091121, 0x0A07FD070A122242, 0x150EFB0E15244484, 0x2A1CF71C2A498808,
  0x5438EF3854921110, 0xA870DF70A8242221, 0x50E0BFE050484442, 0xA0C07FC0A0908884,
  0x03FE030509112141, 0x07FD070A12224282, 0x0EFB0E1524448404, 0x1CF71C2A49880808,
  0x38EF385492111010, 0x70DF70A824222120, 0xE0BFE05048444241, 0xC07FC0A090888482,
  0xFE03050911214181, 0xFD070A1222428202, 0xFB0E152444840404, 0xF71C2A4988080808,
  0xEF38549211101010, 0xDF70A82422212020, 0xBFE0504844424140, 0x7FC0A09088848281
};

/*
 * Zobrist bitstrings generated by the URANDOM Linux generator.
 * The array has 128 entries, 64 for black squares, and 64 for white ones.
 */
static const uint64_t zobrist_bitstrings[] = {
  0x4EC764D9D2FC995C, 0x4689879C5E2B6C8D, 0x7B8509672627BDE6, 0x5EC9EED7E83A1070,
  0x8C61E367C148A670, 0x12BF65D47CB500F3, 0x2D128CD312314B15, 0xEC8FFA579C3FF006,
  0x1B772E0C17A8637D, 0xFEA5D5093AE4F127, 0x3F6223B7D74731E8, 0x9BF6F10BF080F403,
  0xCE03618789AACB66, 0x29264F46A5C844E3, 0xD4BD485D4B88FC6E, 0xAAC3C81799DAE8F5,
  0x46B256D2079DA270, 0x5765CB86D7BE25B4, 0x7754057541AAAF9E, 0xB9E9E765C5214D6D,
  0x957833AE886E5003, 0x59BEDBE41933BEB6, 0xD5D328EC1AE90B22, 0x26FA194CD5A4CC9A,
  0x34790BFC50B7571E, 0xE91BC0D61C9FA3C5, 0x8FB89136550EFB06, 0x36701B34D43E30CD,
  0xAAFD6ACF34BB27C1, 0x401DD1B2B7A72AB4, 0x6F5903B64BBD50D7, 0x9067F2A3CB7EBB34,
  0x2C7807F6E28CD962, 0x42B2DB1AED17AFD5, 0xF42857186A09F927, 0x258AB4C5E9986E0C,
  0x8F8BA0BC0571B0F6, 0xCB7AC29208905214, 0x89914660AA38896A, 0x380CAC8536886A07,
  0x7B7F399C6EC9563D, 0xB9CBFBEEE95F05B1, 0x46AAAFC2159C8F19, 0xDCE680A0882BCCEF,
  0x347DDA36E42E2D99, 0x48EAF8CB224BCBB6, 0x1EACE389084B2674, 0x261BC842C735974D,
  0xF334875975FD986B, 0xF6B436CC3615018C, 0x6658B92BCD930893, 0x4B6BF5B88078852E,
  0x30461B28416207E1, 0xA2EE71D448670786, 0x0C8CD31138E8A683, 0x5FF41A78E0014DD0,
  0xA57DE5B0A848A8E0, 0x71D68A3A9600F975, 0xEBB35D1E75B69FFB, 0xFCC1D327A771464B,
  0xB6C1F45DB043CF56, 0x92EF77063313F3CE, 0x5F48E97C72B167E2, 0x5806342E75C7BB6B,
  0x056C0070CF70F8DD, 0x81AD4662EE05E75A, 0x1C10E0B05C7B3C49, 0xC684C383E396972A,
  0x35747D859FC1A7E1, 0xFDDCF0169427CEE5, 0x3952CBE313B377E2, 0xDA1B5C1E5BCCB131,
  0x10B747061AF1559C, 0x8388EDE0F8EDCBFC, 0x4873A84B8ABF0FA8, 0xB30E990FE793A807,
  0x6B8C5C09C78D0A9F, 0x7A488BC144D344AA, 0xC04C26EC10A73AE9, 0x9278CDFF5EBA539E,
  0xFF3A1C4370FB8703, 0x70D16AE16E34B5B5, 0x2732FCB913E2E5C3, 0xC59EE008F48EB1FA,
  0x38C7DB9E584C3680, 0x16F9194953701E88, 0x2E4C5FAC8051C8FD, 0x27DD41B7B5D864FA,
  0xECA6A65750A55AEA, 0x5EBECF093FF380E7, 0xAA4B94B2F0585B82, 0xC88047DA71E83C4D,
  0xB55C0550FE29B255, 0x2A1F8CB7ECEB6200, 0x7B841E8CB0945F2D, 0x6E5D5D9ED39B4961,
  0x67222624CF379C81, 0x0CC24C31A8BA7F90, 0x4B54A89AF4A4DF4C, 0x91A6E548CAC3E417,
  0x83DEB6121EA84F96, 0x9F951DD98DBB9DF5, 0x25584291FF15CB27, 0x3371E38504F0CB1F,
  0x0EB9B65B5DE9637D, 0x2D292615860C0AE9, 0x8DAFEA0E2F5DD418, 0x5E30A0DFD3A6BA54,
  0x5C442D0B47BF7364, 0x7C2EEBB8C8734B16, 0x440AF7B361B686A6, 0xBC6C2CB56CE41EDD,
  0x2DF7B73E01F1DB6C, 0xD76592C6235F3FFE, 0x2139DC845CE8D2B0, 0x42168DFAFC8BF871,
  0xB0E6598E3C707763, 0xD7C55A777BF74808, 0x04C0175493D0C6EC, 0x78B2B49BF53DD1A5,
  0xC6020A48A9F0B2CD, 0x08CA89D2EA10FEB4, 0x07DE9D1304C3B2BD, 0x769EF9C4E120BC5F,
  0xEEBD5772D344846F, 0x1E972C3F168BBF7F, 0x0C72560FAD1EC422, 0xD85B15F5DBCD19B9
};

/**
 * @endcond
 */


/************************************/
/* Module initialization functions. */
/************************************/

/**
 * @brief Initializes required data structures used by other module's functions.
 *
 * @details This function must be called once before any use of other functions
 * contained in this module.
 *
 * @callergraph
 */
void
board_module_init (void)
{
  board_initialize_bitrow_changes_for_player_array(bitrow_changes_for_player_array);
  board_initialize_shift_square_set_by_amount_mask_array(shift_square_set_by_amount_mask_array);
}



/***************************************************/
/* Function implementations for the Square entity. */
/***************************************************/

/**
 * @brief Returns a string representation for the sqare.
 *
 * @details The returned string cannot be changed and must not be deallocated.
 * When the `sq` parameter is not part of the set defined by the enum `Square`
 * the returned value is `"NA"`.
 *
 * A sample usage scenario taken from unit tests is here exemplified:
 *
 * @snippet board_test.c square_to_string usage
 *
 * @param [in] sq the square to be converted
 * @return        a string of two chars representing the square
 */
const gchar *
square_to_string (const Square sq)
{
  if (square_belongs_to_enum_set(sq)) {
    return sq_to_s[sq];
  } else {
    return sq_to_s[65]; // sq_to_s[65] is "NA"
  }
}

/**
 * @brief Returns a string representation for the move.
 *
 * @details The returned string cannot be changed and must not be deallocated.
 * When the `move` parameter  is equal to #pass_move the returned value is `"--"`,
 * otherwise the returned value is consistent with the function #square_to_string.
 *
 * A sample usage scenario taken from unit tests is here exemplified:
 *
 * @snippet board_test.c square_as_move_to_string usage
 *
 * @param [in] move the move to be converted
 * @return          a string of two chars representing the move
 */
const gchar *
square_as_move_to_string (const Square move)
{
  if (move == pass_move) {
    return sq_to_s[64]; // sq_to_s[64] is "--"
  } else {
    return square_to_string(move);
  }
}

/**
 * @brief Returns a string that represents the square array.
 *
 * @details The returned string has to be freed by the caller.
 *
 * A sample usage scenario taken from unit tests is here exemplified:
 *
 * @snippet board_test.c square_array_to_string usage
 * A sample call is here exemplified:
 *
 * @param [in] sqa    the square array to convert
 * @param [in] length the count of elements in the array
 * @return            a string representation for the square array
 */
gchar *
square_array_to_string (const Square sqa[],
                        const int length)
{
  gchar *squares_to_string;
  GString *tmp;

  tmp = g_string_sized_new(10);

  for (int i = 0; i < length; i++) {
    g_string_append_printf(tmp, "%s", square_to_string(sqa[i]));
    if (length - i > 1) g_string_append_printf(tmp, " ");
  }

  squares_to_string = tmp->str;
  g_string_free(tmp, FALSE);

  return squares_to_string;
}

/**
 * @brief Returns a string that represents the move array.
 *
 * @details The returned string has to be freed by the caller.
 *
 * A sample usage scenario taken from unit tests is here exemplified:
 *
 * @snippet board_test.c square_as_move_array_to_string usage
 *
 * @param [in] mova   the move array to convert
 * @param [in] length the count of elements in the array
 * @return            a string representation for the move array
 */
gchar *
square_as_move_array_to_string (const Square mova[],
                                const int length)
{
  gchar *moves_to_string;
  GString *tmp;

  tmp = g_string_sized_new(10);

  for (int i = 0; i < length; i++) {
    g_string_append_printf(tmp, "%s", square_as_move_to_string(mova[i]));
    if (length - i > 1) g_string_append_printf(tmp, " ");
  }

  moves_to_string = tmp->str;
  g_string_free(tmp, FALSE);

  return moves_to_string;
}

/**
 * @brief Returns TRUE if the `sq` parameter is in the valid range.
 *
 * @details The valid range is `[0,63]`, as defined by the `Square` enum.
 *
 * A sample usage scenario taken from unit tests is here exemplified:
 *
 * @snippet board_test.c square_belongs_to_enum_set usage
 *
 * @param [in] sq the given square
 * @return        true if the square is in the valid range
 */
gboolean
square_belongs_to_enum_set (const Square sq)
{
  return sq >= A1 && sq <= H8;
}


/**
 * @brief Returns TRUE if the `move` parameter is in the valid range.
 *
 * @details The valid range is `[-1,63]`, where `-1` means move_pass,
 * and values from `0` to `63` are defined by the `Square` enum.
 *
 * A sample usage scenario taken from unit tests is here exemplified:
 *
 * @snippet board_test.c square_is_valid_move usage
 *
 * @param [in] move the given move
 * @return          true if the move is in the valid range
 */
gboolean
square_is_valid_move (const Square move)
{
  return square_belongs_to_enum_set(move) || move == pass_move;
}



/******************************************************/
/* Function implementations for the SquareSet entity. */
/******************************************************/

/**
 * @brief Returns a string representation for the square set used
 * to load a json array by postgresql command COPY.
 *
 * @details The returned string has to be freed by the caller.
 *
 * PostgreSQL command COPY requires that the json array elements are
 * "double quoted", so to obtain a db field equal to `["A1", "C1"]`,
 * the string to be prepared is `[""A1"", ""C1""]`.
 *
 * A sample usage scenario taken from unit tests is here exemplified:
 *
 * @snippet board_test.c square_set_to_pg_json_array usage
 *
 * @param [in] squares the square set to be converted into a string
 * @return             a string having the given squares represented
 *                     as a postgresql json array
 */
gchar *
square_set_to_pg_json_array (const SquareSet squares)
{
  GString *tmp = g_string_sized_new(10);
  g_string_append_printf(tmp, "[");
  Square move = 0;
  gboolean passed = FALSE;
  for (SquareSet cursor = 1; cursor != 0; cursor <<= 1) {
    if ((cursor & squares) != empty_square_set) {
      const char row = '1' + (move / 8);
      const char col = 'A' + (move % 8);
      if (passed) {
        g_string_append_printf(tmp, ", ");
      }
      g_string_append_printf(tmp, "\"\"%c%c\"\"", col, row);
      passed = TRUE;
    }
    move++;
  }
  g_string_append_printf(tmp, "]");
  gchar *squares_to_string = tmp->str;
  g_string_free(tmp, FALSE);
  return squares_to_string;
}

/**
 * @brief Returns a string representation for the square set.
 *
 * @details The returned string has to be freed by the caller.
 *
 * A sample usage scenario taken from unit tests is here exemplified:
 *
 * @snippet board_test.c square_set_to_string usage
 *
 * @param [in] squares the square set to be converted into a string
 * @return             a string having the squares sorted as the `Square` enum
 */
gchar *
square_set_to_string (const SquareSet squares)
{
  GString *tmp = g_string_sized_new(10);
  Square move = 0;
  gboolean passed = FALSE;
  for (SquareSet cursor = 1; cursor != 0; cursor <<= 1) {
    if ((cursor & squares) != empty_square_set) {
      const char row = '1' + (move / 8);
      const char col = 'A' + (move % 8);
      if (passed) {
        g_string_append_printf(tmp, " ");
      }
      g_string_append_printf(tmp, "%c%c", col, row);
      passed = TRUE;
    }
    move++;
  }
  gchar *squares_to_string = tmp->str;
  g_string_free(tmp, FALSE);
  return squares_to_string;
}

/**
 * @brief Returns a random square among the given set.
 *
 * @invariant Parameter `squares` must not be empty.
 * The invariant is guarded by an assertion.
 *
 * A sample usage scenario taken from unit tests is here exemplified:
 *
 * @snippet board_test.c square_set_random_selection usage
 *
 * @param [in,out] rng the random number generator instance
 * @param [in]         squares a square set
 * @return             one square selected among the set
 */
Square
square_set_random_selection (RandomNumberGenerator *const rng,
                             const SquareSet squares)
{
  g_assert(squares != empty_square_set);
  SquareSet s = squares;
  const int square_count = bit_works_popcount(squares);
  const int square_index = rng_random_choice_from_finite_set(rng, square_count);
  for (int i = 0; i < square_count; i++) {
    if (i == square_index) break;
    s ^= bit_works_lowest_bit_set_64(s);
  }
  return (Square) bit_works_bitscanLS1B_64(s);
}

/**
 * @brief Transforms the square set into an array.
 *
 * @details The array size, equal to the number of squares held in the set, is
 * returned into the value pointed by the `sq_count` parameter.
 * The array is dynamically allocated by the function, squares are returned
 * manteining the `Square` enum natural order.
 * The `sq_array` parameter is a pointer to the allocated array.
 *
 * The returned array has to be freed by the caller.
 *
 * A sample usage scenario taken from unit tests is here exemplified:
 *
 * @snippet board_test.c square_set_to_array usage
 *
 * @param [out] sq_count the size of the returned array
 * @param [out] sq_array a pointer to the allocated new array
 * @param [in]  squares  a square set
 */
void
square_set_to_array (int *sq_count,
                     Square **sq_array,
                     const SquareSet squares)
{
  static const size_t size_of_square = sizeof(Square);
  const int square_count = bit_works_popcount(squares);
  Square *array = (Square *) malloc(square_count * size_of_square);
  g_assert(array);
  SquareSet s = squares;
  for (int i = 0; i < square_count; i++) {
    const Square sq = bit_works_bitscanLS1B_64(s);
    *(array + i) = sq;
    s ^= (SquareSet) 1 << sq;
  }
  *sq_count = square_count;
  *sq_array = array;
}

/**
 * @brief Transform the array of squares into a square set.
 *
 * @invariant Parameter `sq_array` must be composed of elements belonging to the `Squae` enum.
 * The invariant is guarded by an assertion.
 *
 * @invariant Parameter `sq_count` must be not negative.
 * The invariant is guarded by an assertion.
 *
 * A sample usage scenario taken from unit tests is here exemplified:
 *
 * @snippet board_test.c square_set_from_array usage
 *
 * @param sq_array the array to be converted
 * @param sq_count the number of squares in the array
 * @return         the resulting square set
 */
SquareSet
square_set_from_array (const Square sq_array[],
                       const int sq_count)
{
  g_assert(sq_count >= 0);
  SquareSet squares = 0;
  for (int i = 0; i < sq_count; i++) {
    const Square sq = sq_array[i];
    g_assert(square_belongs_to_enum_set(sq));
    squares |= (SquareSet) 1 << sq;
  }
  return squares;
}

/***************************************************/
/* Function implementations for the Player entity. */
/***************************************************/

/**
 * @brief Returns the square state value representing the player's color.
 *
 * @invariant Parameter `p` must have a value belonging to the `Player` enum.
 * The invariant is guarded by an assertion.
 *
 * A sample usage scenario taken from unit tests is here exemplified:
 *
 * @snippet board_test.c player_color usage
 *
 * @param [in] p the player
 * @return       the square state of the player
 */
SquareState
player_color (const Player p)
{
  g_assert(p == BLACK_PLAYER || p == WHITE_PLAYER);
  return (p == BLACK_PLAYER) ? BLACK_SQUARE : WHITE_SQUARE;
}

/**
 * @brief Returns the player's description.
 *
 * @invariant Parameter `p` must have a value belonging to the `Player` enum.
 * The invariant is guarded by an assertion.
 *
 * A sample usage scenario taken from unit tests is here exemplified:
 *
 * @snippet board_test.c player_description usage
 *
 * @param [in] p the player
 * @return       the player's description
 */
gchar *
player_description (const Player p)
{
  g_assert(p == BLACK_PLAYER || p == WHITE_PLAYER);
  return (p == BLACK_PLAYER) ? "The Black player" : "The White player";
}

/**
 * @brief Returns the player's opponent.
 *
 * @invariant Parameter `p` must have a value belonging to the `Player` enum.
 * The invariant is guarded by an assertion.
 *
 * A sample usage scenario taken from unit tests is here exemplified:
 *
 * @snippet board_test.c player_opponent usage
 *
 * @param [in] p the player
 * @return       the player's opponent
 */
Player
player_opponent (const Player p)
{
  g_assert(p == BLACK_PLAYER || p == WHITE_PLAYER);
  return 1 - p;
}



/*************************************************/
/* Function implementations for the Axis entity. */
/*************************************************/

/**
 * @brief Computes the shift quantity.
 *
 * @details The function is used only in the board module!
 *
 * The parameter `column` and `row` identify a square, and the `axis`
 * then select the line passing by the square and parallel to it.
 * The returned shift quantity is the amount to apply to the bit board
 * The `bit_works_signed_left_shift` function applyed to a bitboard
 * with this computed quantity move the selected line to the main line
 * for the axis.
 *
 * @invariant Parameters `axis` must belong to its enum.
 * Parameters `column` and `row` must stay in the range `0...7`.
 * The invariants are guarded by assertions.
 *
 * @param [in] axis   the selected axis
 * @param [in] column the square's column
 * @param [in] row    the square's row

 * @return     the shift quantity
 */
int
axis_shift_distance (const Axis axis,
                     const uint8_t column,
                     const uint8_t row)
{
  g_assert(axis >= HO && axis <= DU);
  g_assert(column >= 0 && column <= 7);
  g_assert(row >= 0 && row <= 7);

  switch (axis) {
  case HO:
    return -row << 3;
  case VE:
    return -column;
  case DD:
    return (column - row) << 3;
  case DU:
    return (7 - column - row) << 3;
  default:
    abort();
    return EXIT_FAILURE;
  }
}

/**
 * @brief Returns the ordinal position of the move.
 *
 * @details The function is used only in the board module!
 *
 * The parameter `column` and `row` identify a move in the bitboard,
 * given the `axis` the function return the ordinal position of the move
 * in the selected line.
 *
 * @invariant Parameters `axis` must belong to its enum.
 * Parameters `column` and `row` must stay in the range `0...7`.
 * The invariants are guarded by assertions.
 *
 * @param [in] axis   the selected axis
 * @param [in] column the move's column
 * @param [in] row    the move's row
 * @return            the shift quantity
 */
uint8_t
axis_move_ordinal_position_in_bitrow (const Axis axis,
                                      const uint8_t column,
                                      const uint8_t row)
{
  g_assert(axis >= HO && axis <= DU);
  g_assert(column >= 0 && column <= 7);
  g_assert(row >= 0 && row <= 7);

  switch (axis) {
  case VE:
    return row;
  default:
    return column;
  }
}

/**
 * @brief Maps the principal line of each axis into row one.
 *
 * @details The function is used only in the board module!
 *
 * Returns an int having the bits from position 0 to position 7, corresponding to Row One in the board,
 * transformed from:
 *  - `ROW 1` for the `HO` axis.
 *  - `COLUMN A` for the `VE` axis.
 *  - `DIAGONAL A1-H8` for the `DD` axis.
 *  - `DIAGONAL A8-H1` for the `DU` axis.
 *
 * @invariant Parameters `axis` must belong to its enum.
 * The invariants are guarded by assertions.
 *
 * @param [in] axis    the given axis
 * @param [in] squares the set of board squares
 * @return             the transformed line
 */
uint8_t
axis_transform_to_row_one (const Axis axis,
                           const SquareSet squares)
{
  g_assert(axis >= HO && axis <= DU);

  SquareSet tmp;

  tmp = squares;
  switch (axis) {
  case HO:
    break;
  case VE:
    tmp &= column_a;
    tmp |= tmp >> 28;
    tmp |= tmp >> 14;
    tmp |= tmp >> 7;
    break;
  case DD:
    tmp &= diagonal_a1_h8;
    tmp |= tmp >> 32;
    tmp |= tmp >> 16;
    tmp |= tmp >> 8;
    break;
  case DU:
    tmp &= diagonal_h1_a8;
    tmp |= tmp >> 32;
    tmp |= tmp >> 16;
    tmp |= tmp >> 8;
    break;
  default:
    abort();
  }
  return (uint8_t) tmp;
}

/**
 * @brief Maps back the principal line of each axis from row one.
 *
 * @details The function is used only in the board module!
 *
 * Returns a square set having the bits along the axis reference file set to
 * the corresponding ones on the `bitrow` parameter.
 *
 * @invariant Parameters `axis` must belong to its enum.
 * The invariants are guarded by assertions.
 *
 * @param [in] axis   the given axis
 * @param [in] bitrow represents row one
 * @return            a bitboard having the axis reference file set as the bitboard parameter,
 *                    all other position are set to zero
 */
SquareSet
axis_transform_back_from_row_one (const Axis axis,
                                  const uint32_t bitrow)
{
  g_assert(axis >= HO && axis <= DU);

  uint32_t tmp;
  SquareSet bit_board;

  switch (axis) {
  case HO:
    return (SquareSet) bitrow;
  case VE:
    tmp = bitrow;
    tmp |= tmp << 7;
    tmp |= tmp << 14;
    bit_board = (SquareSet) tmp | ((SquareSet) tmp << 28);
    return bit_board & column_a;
  case DD:
    tmp = bitrow;
    tmp |= tmp << 8;
    bit_board = (SquareSet) tmp | ((SquareSet) tmp << 16);
    bit_board |= bit_board << 32;
    return bit_board & diagonal_a1_h8;
  case DU:
    tmp = bitrow;
    tmp |= tmp << 8;
    tmp |= (tmp & squares_b1_f1_a2_e2) << 16;
    bit_board = (SquareSet) tmp | ((SquareSet) tmp << 32);
    return bit_board & diagonal_h1_a8;
  default:
    abort();
    return EXIT_FAILURE;
  }
}

/**************************************************/
/* Function implementations for the Board entity. */
/**************************************************/

/**
 * @brief Board structure constructor.
 *
 * An assertion checks that the received pointer to the allocated
 * board structure is not `NULL`.
 *
 * @invariant Parameters `b` and `w` cannot have common square set.
 * The invariant is guarded by an assertion.
 * It means that a square cannot have a white and a black disc set together.
 *
 * @param [in] b the set of black squares
 * @param [in] w the set of white squares
 * @return       a pointer to a new board structure
 */
Board *
board_new (const SquareSet b,
           const SquareSet w)
{
  g_assert((w & b) == empty_square_set);

  Board *board;
  static const size_t size_of_board = sizeof(Board);

  board = (Board *) malloc(size_of_board);
  g_assert(board);

  board->blacks = b;
  board->whites = w;

  return board;
}

/**
 * @brief Deallocates the memory previously allocated by a call to #board_new.
 *
 * @details If a null pointer is passed as argument, no action occurs.
 *
 * @param [in,out] b the pointer to be deallocated
 */
void
board_free (Board *b)
{
  free(b);
}

/**
 * @brief Clones a Board structure.
 *
 * @invariant Parameter `b` cannot be null.
 * The invariant is guarded by an assertion.
 *
 * @param [in] b the board to clone
 * @return     a pointer to a new board structure
 */
Board *
board_clone (const Board *const b)
{
  g_assert(b);

  return board_new(b->blacks, b->whites);
}

/**
 * @brief Returns the SquareState value for the given board's square.
 *
 * @invariant Parameter `b` must be not `NULL`.
 * Parameter `sq` must belongs to the `Square` enum.
 * Invariants are both guarded by assetions.
 *
 * @param [in] b  a pointer to the board structure
 * @param [in] sq the square to query for
 * @return        the color of the given square
 */
SquareState
board_get_square (const Board *const b,
                  const Square sq)
{
  g_assert(b);
  g_assert(square_belongs_to_enum_set(sq));

  SquareSet bitsquare = (SquareSet) 1 << sq;
  if (bitsquare & b->blacks)
    return BLACK_SQUARE;
  else if (bitsquare & b->whites)
    return WHITE_SQUARE;
  else
    return EMPTY_SQUARE;
}

/**
 * @brief Returns the disk count for the color.
 *
 * @invariant Parameter `b` must be not `NULL`.
 * Parameter `color` must belongs to the `SquareState` enum.
 * Both invariants are guarded by assertions.
 *
 * @param [in] b     a pointer to the board structure
 * @param [in] color the square color
 * @return           the piece count for the given color
 */
int
board_count_pieces (const Board *const b,
                    const SquareState color)
{
  g_assert(b);
  g_assert(color == EMPTY_SQUARE || color == BLACK_SQUARE || color == WHITE_SQUARE);

  return bit_works_popcount(board_get_color(b, color));
}

/**
 * @brief Returns the disk difference between the player and her opponent.
 *
 * @invariant Parameter `b` must be not `NULL`.
 * Parameter `p` must be a value belonging to the `Player` enum.
 * Both invariants are guarded by assertions.
 *
 * @param [in] b a pointer to the board structure
 * @param [in] p the player for whom the difference is computed
 * @return       the disc count difference
 */
int
board_count_difference (const Board *const b,
                        const Player p)
{
  g_assert(b);
  g_assert(p == BLACK_PLAYER || p == WHITE_PLAYER);

  int pcount, ocount;
  Player o;

  o = player_opponent(p);
  pcount = board_count_pieces(b, player_color(p));
  ocount = board_count_pieces(b, player_color(o));

  return pcount - ocount;
}

/**
 * @brief Used for the score at the end of the game.
 *
 * @details Returns the disk difference between the player and her opponent,
 * assigning the empty squares to the player having most discs.
 *
 * From the web site of the World Othello Federation,
 * World Othello Chanpionship Rules, scoring:<br>
 * <em>"At the end of the game, if both players have completed their moves in
 * the allowed time, the winner is the player with the greater number of
 * discs of his colour on the board at the end. The official score of the
 * game will be determined by counting up the discs of each colour on the
 * board, counting empty squares for the winner. In the event of a draw,
 * the score will always be 32-32"</em>.
 *
 * @invariant Parameter `b` must be not `NULL`.
 * Parameter `p` must be a value belonging to the `Player` enum.
 * Both invariants are guarded by assertions.
 *
 * @param [in] b a pointer to the board structure
 * @param [in] p the player for whom the difference is computed
 * @return       the game score according to the WOF rules
 */
int
board_count_diff_winner_get_empties (const Board *const b,
                                     const Player p)
{
  g_assert(b);
  g_assert(p == BLACK_PLAYER || p == WHITE_PLAYER);

  int pcount, ocount, difference, empties;
  Player o;

  o = player_opponent(p);
  pcount = board_count_pieces(b, player_color(p));
  ocount = board_count_pieces(b, player_color(o));

  difference = pcount - ocount;
  empties = 64 - (pcount + ocount);

  return difference + ((difference > 0) ? +empties : -empties);
}

/**
 * @brief Returns 1 if the move, done by the specified player, is legal,
 * otherwise 0.
 *
 * @invariant Parameter `b` must be not `NULL`.
 * Parameter `move` must be a value in the range defined by
 * the #square_is_valid_move function.
 * Parameter `p` must be a value belonging to the `Player` enum.
 * All invariants are guarded by assertions.
 *
 * @param [in] b    a pointer to the board structure
 * @param [in] move the square where to put the new disk
 * @param [in] p    the player moving
 * @return          1 if the move is legal, otherwise 0
 */
int
board_is_move_legal (const Board *const b,
                     const Square move,
                     const Player p)
{
  g_assert(b);
  g_assert(square_is_valid_move(move));
  g_assert(p == BLACK_PLAYER || p == WHITE_PLAYER);

  if (move == pass_move) {
    if (board_empties(b) == 0) {
      return TRUE;
    } else if (board_legal_moves(b, p) == 0) {
      return TRUE;
    } else {
      return FALSE;
    }
  }

  SquareSet bit_move = (SquareSet) 1 << move;

  if ((board_empties(b) & bit_move) == empty_square_set) return FALSE;

  SquareSet p_bit_board = board_get_player(b, p);
  SquareSet o_bit_board = board_get_player(b, player_opponent(p));

  HiLo xy;
  bit_works_bitscan_MS1B_to_base8(&xy, bit_move);
  uint8_t column = xy.lo;
  uint8_t row    = xy.hi;

  for (Axis axis = HO; axis <= DU; axis++) {
    const int move_ordinal_position = axis_move_ordinal_position_in_bitrow(axis, column, row);
    const int shift_distance = axis_shift_distance(axis, column, row);
    const uint8_t p_bitrow = axis_transform_to_row_one(axis, bit_works_signed_left_shift(p_bit_board, shift_distance));
    const uint8_t o_bitrow = axis_transform_to_row_one(axis, bit_works_signed_left_shift(o_bit_board, shift_distance));
    if (board_bitrow_changes_for_player(p_bitrow, o_bitrow, move_ordinal_position) != p_bitrow) {
      return TRUE;
    }
  }

  /* If no capture on the four directions happens, return false. */
  return FALSE;
}

/**
 * @brief Returns a list holding the legal moves that the player can do at the board position.
 *        When no moves are available to the player the method returns an empty list.
 *
 * Implements the legal moves call by waveing the potential legal moves up to the bracketing
 * pieces. Directions are computed one by one, squares work in parallel.
 *
 * @invariant Parameter `b` must be not `NULL`.
 * Parameter `p` must be a value belonging to the `Player` enum.
 * All invariants are guarded by assertions.
 *
 * @param [in] b the given board
 * @param [in] p the player that has to move
 * @return       legal moves for the player
 */
SquareSet
board_legal_moves (const Board *const b,
                   const Player p)
{
  g_assert(b);
  g_assert(p == BLACK_PLAYER || p == WHITE_PLAYER);

  register SquareSet result;

  result = empty_square_set;

  const Player o = player_opponent(p);
  const SquareSet empties = board_empties(b);
  const SquareSet p_bit_board = board_get_player(b, p);
  const SquareSet o_bit_board = board_get_player(b, o);

  for (Direction dir = NW; dir <= SE; dir++) {
    const Direction opposite = direction_opposite(dir);
    SquareSet wave = direction_shift_square_set(dir, empties) & o_bit_board;
    int shift = 1;
    while (wave != empty_square_set) {
      wave = direction_shift_square_set(dir, wave);
      shift++;
      result |= direction_shift_back_square_set_by_amount(opposite, (wave & p_bit_board), shift);
      wave &= o_bit_board;
    }
  }

  return result;
}

/**
 * @brief Returns `TRUE` if the board is not final.
 *
 * @invariant Parameter `b` must be not `NULL`.
 * All invariants are guarded by assertions.
 *
 * @param [in] b the given board
 * @return       true if one of the player has one or more legal moves
 */
gboolean
board_has_any_player_any_legal_move (const Board *const b)
{
  g_assert(b);

  return (empty_square_set == board_legal_moves(b, BLACK_PLAYER) &&
          empty_square_set == board_legal_moves(b, WHITE_PLAYER)) ? FALSE : TRUE;
}

/**
 * @brief Returns the set of empty squares in the board.
 *
 * @invariant Parameter `b` must be not `NULL`.
 * The invariant is guarded by an assertion.
 *
 * @param [in] b a pointer to the board structure
 * @return       the set of empty squares
 */
SquareSet
board_empties (const Board *const b)
{
  g_assert(b);

  return ~(b->blacks | b->whites);
}

/**
 * @brief Returns the set of black squares in the board.
 *
 * @invariant Parameter `b` must be not `NULL`.
 * The invariant is guarded by an assertion.
 *
 * @param [in] b a pointer to the board structure
 * @return       the set of black squares
 */
SquareSet
board_blacks (const Board *const b)
{
  g_assert(b);

  return b->blacks;
}

/**
 * @brief Returns the set of white squares in the board.
 *
 * @invariant Parameter `b` must be not `NULL`.
 * The invariant is guarded by an assertion.
 *
 * @param [in] b a pointer to the board structure
 * @return       the set of white squares
 */
SquareSet
board_whites (const Board *const b)
{
  g_assert(b);

  return b->whites;
}

/**
 * @brief Returns the #SquareSet of the #Board addressed by `b`
 * corresponding to the #SquareState identified by `color`.
 *
 * @invariant Parameter `b` must be not `NULL`.
 * Parameter `color` must belong to the #SquareState enum.
 * Invariants are guarded by assertions.
 *
 * @param [in] b     a pointer to the board structure
 * @param [in] color a given color
 * @return           the set of squares in the board having the given color
 */
SquareSet
board_get_color (const Board *const b,
                 const SquareState color)
{
  g_assert(b);
  g_assert(color == EMPTY_SQUARE || color == BLACK_SQUARE || color == WHITE_SQUARE);

  SquareSet squares;

  switch (color) {
  case EMPTY_SQUARE:
    squares = board_empties(b);
    break;
  case BLACK_SQUARE:
    squares = b->blacks;
    break;
  case WHITE_SQUARE:
    squares = b->whites;
    break;
  default:
    abort();
  }

  return squares;
}

/**
 * @brief Returns the #SquareSet of the #Board addressed by `b`
 * corresponding to the #Player identified by `p`.
 *
 * @invariant Parameter `b` must be not `NULL`.
 * Parameter `p` must belong to the #Player enum.
 * Invariants are guarded by assertions.
 *
 * @param [in] b a pointer to the board structure
 * @param [in] p a given player
 * @return       the set of squares in the board belonging to the given player
 */
SquareSet
board_get_player (const Board *const b,
                  const Player p)
{
  g_assert(b);
  g_assert(p == BLACK_PLAYER || p == WHITE_PLAYER);

  SquareSet squares;

  switch (p) {
  case BLACK_PLAYER:
    squares = b->blacks;
    break;
  case WHITE_PLAYER:
    squares = b->whites;
    break;
  default:
    abort();
  }

  return squares;
}

/**
 * @brief Returns a formatted string showing a 2d graphical represention of the board.
 *
 * The returned string has a dynamic extent. It must then properly
 * garbage collected by a call to `g_free` when no more referenced.
 *
 * @invariant Parameter `b` must be not `NULL`.
 * Invariants are guarded by assertions.
 *
 * @param [in] b a pointer to the board structure
 * @return       a string being a 2d representation of the board
 */
gchar *
board_print (const Board *const b)
{
  g_assert(b);

  char *b_to_string;
  GString *bs;

  bs = g_string_sized_new(220);
  g_string_append(bs, "    a b c d e f g h ");
  for (int row = 0; row < 8; row++) {
    g_string_append_printf(bs, "\n %1d  ", row + 1);
    for (int col = 0; col < 8; col++) {
      g_string_append_printf(bs, "%c ", square_state_symbol(board_get_square(b, (8 * row) + col)));
    }
  }
  g_string_append(bs, "\n");

  b_to_string = bs->str;
  g_string_free(bs, FALSE);

  return b_to_string;
}

/**
 * @brief Compares board `a` and board `b`.
 *
 * When the two board are equal it returns `0`, when `a` is greather then `b` it
 * returns `+1`, otherwise `-1`.
 *
 * Boards are equals when have the same square configuration.
 *
 * The returned string has a dynamic extent set by a call to malloc. It must then properly
 * garbage collected by a call to free when no more referenced.
 *
 * @invariant Parameters `a` and `b` must be not `NULL`.
 * Invariants are guarded by assertions.
 *
 * @param [in] a a pointer to a board structure
 * @param [in] b a pointer to another board structure
 * @return       `-1` when `a < b`, `+1` when `a > b`, or `0` when the two boards are equal
 */
int
board_compare (const Board *const a,
               const Board *const b)
{
  g_assert(a);
  g_assert(b);

  if (a->blacks < b->blacks) {
    return -1;
  } else if (a->blacks > b->blacks) {
    return +1;
  } else {          /* Blacks are equal. */
    if (a->whites < b->whites) {
      return -1;
    } else if (a->whites > b->whites) {
      return +1;
    } else {        /* Also whites are equal, and so are a and b boards. */
      return  0;
    }
  }
}

/**
 * Returns an 8-bit row representation of the player pieces after applying the move.
 *
 * @param [in] player_row    8-bit bitboard corrosponding to player pieces
 * @param [in] opponent_row  8-bit bitboard corrosponding to opponent pieces
 * @param [in] move_position square to move
 * @return                   the new player's row index after making the move
 */
uint8_t
board_bitrow_changes_for_player (int player_row,
                                 int opponent_row,
                                 int move_position)
{
  const int array_index = player_row | (opponent_row << 8) | (move_position << 16);
  return bitrow_changes_for_player_array[array_index];
}



/******************************************************/
/* Function implementations for the Direction entity. */
/******************************************************/

/**
 * @brief Returns a new #SquareSet value by shifting the
 * `squares` parameter by one position on the board.
 *
 * @invariant Parameter `dir` must belong to the #Direction enum.
 * The invariant is guarded by an assertion.
 *
 * @param [in] dir     the direction to shift to
 * @param [in] squares the squares set on the bitboard
 * @return             the shifted squares
*/
SquareSet
direction_shift_square_set (const Direction dir,
                            const SquareSet squares)
{
  g_assert(dir >= NW && dir <= SE);

  switch (dir) {
  case NW: return (squares >> 9) & all_squares_except_column_h;
  case N:  return (squares >> 8);
  case NE: return (squares >> 7) & all_squares_except_column_a;
  case W:  return (squares >> 1) & all_squares_except_column_h;
  case E:  return (squares << 1) & all_squares_except_column_a;
  case SW: return (squares << 7) & all_squares_except_column_h;
  case S:  return (squares << 8);
  case SE: return (squares << 9) & all_squares_except_column_a;
  default: abort();
  }
}

/**
 * @brief Returns a new #SquareSet value by shifting the `squares` parameter
 * by a number of positions as given by the `amount` parameter.
 *
 * Amount must be in the 0..7 range, meaning that 0 is equal to no shift, 1 is
 * on position, and so on.
 *
 * @invariant Parameter `dir` must belong to the #Direction enum.
 * The invariant is guarded by an assertion.
 *
 * @invariant Parameter `amount` must be in the range 0..7 enum.
 * The invariant is guarded by an assertion.
 *
 * @param [in] dir     the direction to shift to
 * @param [in] squares the squares set on the bitboard
 * @param [in] amount  the amount to shift
 * @return             the shifted squares
*/
SquareSet
direction_shift_square_set_by_amount (const Direction dir,
                                      const SquareSet squares,
                                      const int       amount)
{
  g_assert(dir >= NW && dir <= SE);
  g_assert(amount >= 0 && amount <= 7);

  SquareSet ret;

  const int index = amount | (dir << 3);

  switch (dir) {
  case NW: ret = squares >> (9 * amount); break;
  case N:  ret = squares >> (8 * amount); break;
  case NE: ret = squares >> (7 * amount); break;
  case W:  ret = squares >> (1 * amount); break;
  case E:  ret = squares << (1 * amount); break;
  case SW: ret = squares << (7 * amount); break;
  case S:  ret = squares << (8 * amount); break;
  case SE: ret = squares << (9 * amount); break;
  default: abort();
  }
  ret = ret & shift_square_set_by_amount_mask_array[index];
  return ret;
}

/**
 * @brief Returns the opposite direction of the one given by the `dir` parameter.
 *
 * @invariant Parameter `dir` must belong to the #Direction enum.
 * The invariant is guarded by an assertion.
 *
 * @param [in] dir the given direction
 * @return         the opposite direction
 */
Direction
direction_opposite (const Direction dir)
{
  g_assert(dir >= NW && dir <= SE);

  switch (dir) {
  case NW: return SE;
  case N:  return S;
  case NE: return SW;
  case W:  return E;
  case E:  return W;
  case SW: return NE;
  case S:  return N;
  case SE: return NW;
  default: abort();
  }
}

/********************************************************/
/* Function implementations for the SquareState entity. */
/********************************************************/

/**
 * @brief Returns the #SquareState printable representation.
 *
 * @invariant Parameter `color` must belong to the #SquareState enum.
 * The invariant is guarded by an assertion.
 *
 * @param [in] color the given color
 * @return           the color's `symbol`
 */
char
square_state_symbol (const SquareState color)
{
  g_assert(color >= EMPTY_SQUARE && color <= WHITE_SQUARE);

  switch (color) {
  case EMPTY_SQUARE: return '.';
  case BLACK_SQUARE: return '@';
  case WHITE_SQUARE: return 'O';
  default: abort();
  }
}



/*********************************************************/
/* Function implementations for the GamePosition entity. */
/*********************************************************/

/**
 * @brief GamePosition structure constructor.
 *
 * An assertion checks that the received pointer to the allocated
 * game position structure is not `NULL`.
 *
 * @invariant Parameter `b` cannot be null.
 * The invariant is guarded by an assertion.
 *
 * @invariant Parameter `p` must belong to the `Player` enum set.
 * The invariant is guarded by an assertion.
 *
 * @param [in] b the board field
 * @param [in] p the player field
 * @return       a pointer to a new game position structure
 */
GamePosition *
game_position_new (Board *b,
                   Player p)
{
  g_assert(b);
  g_assert(p == BLACK_PLAYER || p == WHITE_PLAYER);

  GamePosition *gp;
  static const size_t size_of_game_position = sizeof(GamePosition);

  gp = (GamePosition *) malloc(size_of_game_position);
  g_assert(gp);

  gp->board = b;
  gp->player = p;

  return gp;
}

/**
 * @brief Deallocates the memory previously allocated by a call to #game_position_new.
 *
 * @details If a null pointer is passed as argument, no action occurs.
 *
 * @param [in,out] gp the pointer to be deallocated
 */
void
game_position_free (GamePosition *gp)
{
  if (gp) {
    board_free(gp->board);
    g_free(gp);
  }
}

/**
 * @brief Clones a GamePosition structure.
 *
 * @invariant Parameter `gp` cannot be null.
 * The invariant is guarded by an assertion.
 *
 * @param [in] gp the game position to clone
 * @return     a pointer to a new game position structure
 */
GamePosition *
game_position_clone (const GamePosition *const gp)
{
  g_assert(gp);

  return game_position_new(board_new(gp->board->blacks,
                                     gp->board->whites),
                           gp->player);
}

/**
 * @brief Compares game positions `a` and `b`.
 *
 * When the two game position are equal it returns `0`, when `a` is greather then `b` it
 * returns `+1`, otherwise `-1`.
 *
 * Game positions are equals when have the same board and player.
 *
 * @invariant Parameters `a` and `b` must be not `NULL`.
 * Invariants are guarded by assertions.
 *
 * @param [in] a a pointer to a game position structure
 * @param [in] b a pointer to a second structure
 * @return       `-1` when `a < b`, `+1` when `a > b`, or `0` when the two game position are equal
 */
int
game_position_compare (const GamePosition *const a,
                       const GamePosition *const b)
{
  g_assert(a);
  g_assert(b);

  if (a == b) return 0;

  const int board_comp = board_compare(a->board, b->board);
  if (board_comp != 0) {
    return board_comp;
  } else {
    if (a->player < b->player) {
      return -1;
    } else if (a->player > b->player) {
      return +1;
    } else {
      return 0;             /* Players are equal, and so are a and b. */
    }
  }
}

/**
 * @brief Returns a formatted string showing a 2d graphical represention of the game position.
 *
 * The returned string has a dynamic extent set by a call to malloc. It must then properly
 * garbage collected by a call to free when no more referenced.
 *
 * @invariant Parameter `gp` must be not `NULL`.
 * Invariants are guarded by assertions.
 *
 * @param [in] gp a pointer to the game position structure
 * @return        a string being a 2d representation of the game position
 */
gchar *
game_position_print (const GamePosition *const gp)
{
  g_assert(gp);

  const gchar *separator = NULL;

  gchar *gp_to_string;
  gchar *b_to_string;

  b_to_string = board_print(gp->board);

  gp_to_string = g_strjoin(separator,
                           b_to_string,
                           "Player to move: ",
                           (gp->player == BLACK_PLAYER) ? "BLACK" : "WHITE",
                           "\n",
                           NULL);

  g_free(b_to_string);

  return gp_to_string;
}

/**
 * @brief Returns a string representation of the game position.
 *
 * @invariant Parameter `gp` must be not `NULL`.
 * Invariants are guarded by assertions.
 *
 * @param [in] gp the given game position
 * @return        a string representing the game position
 */
gchar *
game_position_to_string (const GamePosition *const gp)
{
  g_assert(gp);

  const Board  *b = gp->board;
  const Player  p = gp->player;

  gchar *gp_to_string;
  GString *s;

  s = g_string_sized_new(66); // 64 for the boards + 1 for the player + the final NULL termination
  for (int pos = 0; pos < 64; pos++) {
    const SquareSet sq = (SquareSet) 1 << pos;
    char color = '.';
    if (sq & b->blacks) {
      color = 'b';
    } else if (sq & b->whites) {
      color = 'w';
    }
    g_string_append_printf(s, "%c", color);
  }
  g_string_append_printf(s, "%c", (p == BLACK_PLAYER) ? 'b' : 'w');

  gp_to_string = s->str;
  g_string_free(s, FALSE);

  return gp_to_string;
}

/**
 * @brief Returns the disk difference between the player and her opponent.
 *
 * @invariant Parameter `gp` must be not `NULL`.
 * Invariants are guarded by assertions.
 *
 * @param [in] gp the given game position
 * @return        the disc count difference
 */
int
game_position_count_difference (const GamePosition *gp)
{
  g_assert(gp);

  return board_count_difference(gp->board, gp->player);
}

/**
 * @brief Returns a list holding the legal moves for the game position.
 *
 * @invariant Parameter `gp` must be not `NULL`.
 * Invariants are guarded by assertions.
 *
 * @param [in] gp the given game position
 * @return        a square set holding the legal moves
 */
SquareSet
game_position_legal_moves (const GamePosition *gp)
{
  g_assert(gp);

  return board_legal_moves(gp->board, gp->player);
}

/**
 * @brief Returns if the game state admit one or more legal moves.
 *
 * @invariant Parameter `gp` must be not `NULL`.
 * Invariants are guarded by assertions.
 *
 * @param [in] gp the given game position
 * @return        true if the game state admit a legal move
 */
gboolean
game_position_has_any_legal_move (const GamePosition *const gp)
{
  g_assert(gp);

  return (empty_square_set == game_position_legal_moves(gp)) ? FALSE : TRUE;
}

/**
 * @brief Returns `TRUE` if the game position is not final.
 *
 * @invariant Parameter `gp` must be not `NULL`.
 * Invariants are guarded by assertions.
 *
 * @param [in] gp the given game position
 * @return        true if the game state admit a legal move for at last one player
 */
gboolean
game_position_has_any_player_any_legal_move (const GamePosition *const gp)
{
  g_assert(gp);

  return board_has_any_player_any_legal_move(gp->board);
}

/**
 * @brief Returns true if the `move` is legal for the game position.
 *
 * @invariant Parameter `gp` must be not `NULL`.
 * Parameter `move` must be a value in the range defined by
 * the #square_is_valid_move function.
 * Invariants are guarded by assertions.
 *
 * @param [in] gp   the given game position
 * @param [in] move the square where to put the new disk
 * @return          true if the move is legal
 */
gboolean
game_position_is_move_legal (const GamePosition *const gp,
                             const Square move)
{
  g_assert(gp);
  g_assert(square_is_valid_move(move));

  return board_is_move_legal(gp->board, move, gp->player);
}

/**
 * @brief Executes a game move on the given position.
 *
 * @invariant Parameter `gp` must be not `NULL`.
 * Parameter `move` must be a value in the range defined by
 * the #square_is_valid_move function.
 * Invariants are guarded by assertions.
 *
 * @param [in] gp   the given game position
 * @param [in] move the square where to put the new disk
 * @return          a pointer to a newly created game position as a rusult of the move
 */
GamePosition *
game_position_make_move (const GamePosition *const gp,
                         const Square move)
{
  g_assert(gp);
  g_assert(square_is_valid_move(move));
  g_assert(game_position_is_move_legal(gp, move));

  if (move == pass_move) {
    return game_position_pass(gp);
  }

  const Player p = gp->player;
  const Player o = player_opponent(p);
  const Board *b = gp->board;
  const SquareSet p_bit_board = board_get_player(b, p);
  const SquareSet o_bit_board = board_get_player(b, o);
  const int column = move % 8;
  const int row = move / 8;

  SquareSet new_bit_board[2];
  const SquareSet unmodified_mask = ~bitboard_mask_for_all_directions[move];
  new_bit_board[p] = p_bit_board & unmodified_mask;
  new_bit_board[o] = o_bit_board & unmodified_mask;

  /*
  for (Axis axis = HO; axis <= DU; axis++) {
    const int move_ordinal_position = axis_move_ordinal_position_in_bitrow(axis, column, row);
    const int shift_distance = axis_shift_distance(axis, column, row);
    uint8_t p_bitrow = axis_transform_to_row_one(axis, bit_works_signed_left_shift(p_bit_board, shift_distance));
    uint8_t o_bitrow = axis_transform_to_row_one(axis, bit_works_signed_left_shift(o_bit_board, shift_distance));
    p_bitrow = board_bitrow_changes_for_player(p_bitrow, o_bitrow, move_ordinal_position);
    o_bitrow &= ~p_bitrow;
    new_bit_board[p] |= bit_works_signed_left_shift(axis_transform_back_from_row_one(axis, p_bitrow), -shift_distance);
    new_bit_board[o] |= bit_works_signed_left_shift(axis_transform_back_from_row_one(axis, o_bitrow), -shift_distance);
  }
  */

  int shift_distance;
  uint8_t p_bitrow;
  uint8_t o_bitrow;

  /* Axis HO. */
  const uint8_t right_shift_for_HO = 8 * row;
  p_bitrow = axis_transform_to_row_one(HO, p_bit_board >> right_shift_for_HO);
  o_bitrow = axis_transform_to_row_one(HO, o_bit_board >> right_shift_for_HO);
  p_bitrow = board_bitrow_changes_for_player(p_bitrow, o_bitrow, column);
  o_bitrow &= ~p_bitrow;
  new_bit_board[p] |= ((SquareSet) p_bitrow << right_shift_for_HO);
  new_bit_board[o] |= ((SquareSet) o_bitrow << right_shift_for_HO);

  /* Axis VE. */
  p_bitrow = axis_transform_to_row_one(VE, p_bit_board >> column);
  o_bitrow = axis_transform_to_row_one(VE, o_bit_board >> column);
  p_bitrow = board_bitrow_changes_for_player(p_bitrow, o_bitrow, row);
  o_bitrow &= ~p_bitrow;
  new_bit_board[p] |= axis_transform_back_from_row_one(VE, p_bitrow) << column;
  new_bit_board[o] |= axis_transform_back_from_row_one(VE, o_bitrow) << column;

  /* Axis DD. */
  shift_distance = axis_shift_distance(DD, column, row);
  p_bitrow = axis_transform_to_row_one(DD, bit_works_signed_left_shift(p_bit_board, shift_distance));
  o_bitrow = axis_transform_to_row_one(DD, bit_works_signed_left_shift(o_bit_board, shift_distance));
  p_bitrow = board_bitrow_changes_for_player(p_bitrow, o_bitrow, column);
  o_bitrow &= ~p_bitrow;
  new_bit_board[p] |= bit_works_signed_left_shift(axis_transform_back_from_row_one(DD, p_bitrow), -shift_distance);
  new_bit_board[o] |= bit_works_signed_left_shift(axis_transform_back_from_row_one(DD, o_bitrow), -shift_distance);

  /* Axis DU. */
  shift_distance = axis_shift_distance(DU, column, row);
  p_bitrow = axis_transform_to_row_one(DU, bit_works_signed_left_shift(p_bit_board, shift_distance));
  o_bitrow = axis_transform_to_row_one(DU, bit_works_signed_left_shift(o_bit_board, shift_distance));
  p_bitrow = board_bitrow_changes_for_player(p_bitrow, o_bitrow, column);
  o_bitrow &= ~p_bitrow;
  new_bit_board[p] |= bit_works_signed_left_shift(axis_transform_back_from_row_one(DU, p_bitrow), -shift_distance);
  new_bit_board[o] |= bit_works_signed_left_shift(axis_transform_back_from_row_one(DU, o_bitrow), -shift_distance);

  return game_position_new(board_new(new_bit_board[0], new_bit_board[1]), o);
}

/**
 * @brief Returns a new game position by passing the move.
 *
 * The function doesn't check that the current player has to pass.
 *
 * @param [in] gp the current game position
 * @return     a new game position computed by passing the move
 */
GamePosition *
game_position_pass (const GamePosition *const gp)
{
  g_assert(gp);
  g_assert(TRUE != game_position_has_any_legal_move(gp));

  return game_position_new(board_new(gp->board->blacks,
                                     gp->board->whites),
                           player_opponent(gp->player));
}

/**
 * @brief Returns the hash value for the game position.
 *
 * The hash is computed by mean of a zobrist technique.
 *
 * @param [in] gp the current game position
 * @return        the hash value for the game position
 */
uint64_t
game_position_hash (const GamePosition *const gp)
{
  const SquareSet whites = gp->board->whites;
  const SquareSet blacks = gp->board->blacks;
  const Player p = gp->player;

  uint64_t hash = 0;

  for (int i = 0; i < 64; i++) {
    const uint64_t mask = (uint64_t) 1 << i;
    if (blacks & mask) hash ^= zobrist_bitstrings[i];
    if (whites & mask) hash ^= zobrist_bitstrings[i + 64];
  }
  if (p) hash = ~hash; /* In this way passing doesn't require a full new hash. */

  return hash;
}

/**
 * @brief Used for the score at the end of the game.
 *
 * @details Returns the disk difference between the player and her opponent,
 * assigning the empty squares to the player having most discs.
 *
 * From the web site of the World Othello Federation,
 * World Othello Chanpionship Rules, scoring:<br>
 * <em>"At the end of the game, if both players have completed their moves in
 * the allowed time, the winner is the player with the greater number of
 * discs of his colour on the board at the end. The official score of the
 * game will be determined by counting up the discs of each colour on the
 * board, counting empty squares for the winner. In the event of a draw,
 * the score will always be 32-32"</em>.
 *
 * @param [in] gp a pointer to the game position structure
 * @return        the game score according to the WOF rules
 */
int
game_position_final_value (const GamePosition *const gp)
{
  return board_count_diff_winner_get_empties(gp->board, gp->player);
}

/**
 * @brief Returns the count of empty discs.
 *
 * @param [in] gp a pointer to the game position structure
 * @return        the empty count
 */
int
game_position_empty_count (const GamePosition *const gp)
{
  return bit_works_popcount(board_empties(gp->board));
}



/**********************************************************/
/* Function implementations for the GamePositionX entity. */
/**********************************************************/

/**
 * @brief Game position x structure constructor.
 *
 * An assertion checks that the received pointer to the allocated
 * game position x structure is not `NULL`.
 *
 * @invariant Parameters `b` and `w` cannot have common square set.
 * The invariant is guarded by an assertion.
 * It means that a square cannot have a white and a black disc set together.
 *
 * @param [in] b the set of black squares
 * @param [in] w the set of white squares
 * @param [in] p the player having to move
 * @return       a pointer to a new game position x structure
 */
GamePositionX *
game_position_x_new (const SquareSet b,
                     const SquareSet w,
                     const Player p)
{
  g_assert((w & b) == empty_square_set);

  GamePositionX *gpx;
  static const size_t size_of_game_position_x = sizeof(GamePositionX);

  gpx = (GamePositionX *) malloc(size_of_game_position_x);
  g_assert(gpx);

  gpx->blacks = b;
  gpx->whites = w;
  gpx->player = p;

  return gpx;
}

/**
 * @brief Deallocates the memory previously allocated by a call to #game_position_x_new.
 *
 * @details If a null pointer is passed as argument, no action occurs.
 *
 * @param [in,out] gpx the pointer to be deallocated
 */
void
game_position_x_free (GamePositionX *gpx)
{
  free(gpx);
}

/**
 * @brief Clones a `GamePositionX` structure.
 *
 * @invariant Parameter `gpx` cannot be null.
 * The invariant is guarded by an assertion.
 *
 * @param [in] gpx the game position x to clone
 * @return         a pointer to a new game position structure
 */
GamePositionX *
game_position_x_clone (const GamePositionX *const gpx)
{
  g_assert(gpx);

  return game_position_x_new(gpx->blacks,
                             gpx->whites,
                             gpx->player);
}

/**
 * @brief Returns a new `GamePositionX` structure cloning `gp`.
 *
 * @invariant Parameter `gp` cannot be null.
 * The invariant is guarded by an assertion.
 *
 * @param [in] gp the game position to convert
 * @return        a pointer to a new game position x structure
 */
GamePositionX *
game_position_x_gp_to_gpx (const GamePosition *const gp)
{
  g_assert(gp);

  GamePositionX *gpx;
  gpx = game_position_x_new(gp->board->blacks,
                            gp->board->whites,
                            gp->player);
  return gpx;
}

/**
 * @brief Returns a new `GamePosition` structure cloning `gpx`.
 *
 * @invariant Parameter `gpx` cannot be null.
 * The invariant is guarded by an assertion.
 *
 * @param [in] gpx the game position x to convert
 * @return         a pointer to a new game position structure
 */
GamePosition *
game_position_x_gpx_to_gp (const GamePositionX *const gpx)
{
  g_assert(gpx);

  GamePosition *gp;
  gp = game_position_new(board_new(gpx->blacks,
                                   gpx->whites),
                         gpx->player);
  return gp;
}

/**
 * @brief Copies a `GamePositionX` structure.
 *
 * @param [in]  from the game position x to copy from
 * @param [out] to   the game position x to copy to
 */
void
game_position_x_copy (const GamePositionX *const from,
                      GamePositionX *const to)
{
  to->blacks = from->blacks;
  to->whites = from->whites;
  to->player = from->player;
}

/**
 * @brief Copies a `GamePosition` structure into a `GamePositionX` one.
 *
 * @param [in]  from the game position to copy from
 * @param [out] to   the game position x to copy to
 */
void
game_position_x_copy_from_gp  (const GamePosition   *const from,
                               GamePositionX *const to)
{
  to->blacks = from->board->blacks;
  to->whites = from->board->whites;
  to->player = from->player;
}

/**
 * @brief Updates game position x `next` by passing the move from `current`.
 *
 * The function doesn't check that the current player has to pass.
 *
 * @param [in]  current the current game position x
 * @param [out] next    the updated game position x after passing
 */
void
game_position_x_pass  (const GamePositionX *const current,
                       GamePositionX *const next)
{
  next->blacks = current->blacks;
  next->whites = current->whites;
  next->player = 1 - current->player;
}

/**
 * @brief Returns the hash value for the game position x.
 *
 * The hash is computed by mean of a zobrist technique.
 *
 * @param [in] gpx the current game position
 * @return         the hash value for the game position
 */
uint64_t
game_position_x_hash (const GamePositionX *const gpx)
{
  const SquareSet whites = gpx->whites;
  const SquareSet blacks = gpx->blacks;
  const Player p = gpx->player;

  uint64_t hash = 0;

  for (int i = 0; i < 64; i++) {
    const uint64_t mask = (uint64_t) 1 << i;
    if (blacks & mask) hash ^= zobrist_bitstrings[i];
    if (whites & mask) hash ^= zobrist_bitstrings[i + 64];
  }
  if (p) hash = ~hash; /* In this way passing doesn't require a full new hash. */

  return hash;
}

/**
 * @brief Returns the set of empty squares in the game position.
 *
 * @param [in] gpx a pointer to the game position structure
 * @return         the set of empty squares
 */
SquareSet
game_position_x_empties (const GamePositionX *const gpx)
{
  return ~(gpx->blacks | gpx->whites);
}

/**
 * @brief Returns the square set belonging to the moving player.
 *
 * @param [in] gpx a pointer to the game position structure
 * @return         the set of squares in the board belonging to the given player
 */
SquareSet
game_position_x_get_player (const GamePositionX *const gpx)
{
  if (gpx->player == BLACK_PLAYER) {
    return gpx->blacks;
  } else {
    return gpx->whites;
  }
}

/**
 * @brief Returns the square set belonging to the opponent player.
 *
 * @param [in] gpx a pointer to the game position structure
 * @return         the set of squares in the board belonging to the opponent
 */
SquareSet
game_position_x_get_opponent (const GamePositionX *const gpx)
{
  if (gpx->player == BLACK_PLAYER) {
    return gpx->whites;
  } else {
    return gpx->blacks;
  }
}

/**
 * @brief Returns the SquareState value for the given game position's square.
 *
 * @invariant Parameter `gpx` must be not `NULL`.
 * Parameter `sq` must belongs to the `Square` enum.
 * Invariants are both guarded by assetions.
 *
 * @param [in] gpx a pointer to the board structure
 * @param [in] sq  the square to query for
 * @return         the color of the given square
 */
SquareState
game_position_x_get_square (const GamePositionX *const gpx,
                            const Square sq)
{
  g_assert(gpx);
  g_assert(square_belongs_to_enum_set(sq));

  SquareSet bitsquare = (SquareSet) 1 << sq;
  if (bitsquare & gpx->blacks)
    return BLACK_SQUARE;
  else if (bitsquare & gpx->whites)
    return WHITE_SQUARE;
  else
    return EMPTY_SQUARE;
}

/**
 * @brief Returns a set of squares that represents the legal moves for the game position.
 *
 * @param [in] gpx the given game position
 * @return         a square set holding the legal moves
 */
SquareSet
game_position_x_legal_moves (const GamePositionX *const gpx)
{
  SquareSet result = empty_square_set;

  const SquareSet empties = game_position_x_empties(gpx);
  const SquareSet p_bit_board = game_position_x_get_player(gpx);
  const SquareSet o_bit_board = game_position_x_get_opponent(gpx);

  for (Direction dir = NW; dir <= SE; dir++) {
    const Direction opposite = direction_opposite(dir);
    SquareSet wave = direction_shift_square_set(dir, empties) & o_bit_board;
    int shift = 1;
    while (wave != empty_square_set) {
      wave = direction_shift_square_set(dir, wave);
      shift++;
      result |= direction_shift_back_square_set_by_amount(opposite, (wave & p_bit_board), shift);
      wave &= o_bit_board;
    }
  }

  return result;
}

/**
 * @brief Returns the disk difference between the player and her opponent.
 *
 * @param [in] gpx the given game position
 * @return         the disc count difference
 */
int
game_position_x_count_difference (const GamePositionX *const gpx)
{
  const int square_difference = bit_works_popcount(gpx->blacks) - bit_works_popcount(gpx->whites);
  return (gpx->player == BLACK_PLAYER) ? square_difference : - square_difference;
}

/**
 * @brief Computes the string representation of the game position.
 *
 * @details The function overwrites sixty six chars starting from the position
 * identified by the `out` pointer.
 *
 * A sample call is here exemplified:
 *
 * @code
 * GamePositionX *gpx;
 * char gpx_to_string[66];
 *
 * gpx = game_position_x_new(0x00000000000000FF,
 *                           0xFF0000000000FF00,
 *                           WHITE_PLAYER);
 *
 * game_position_x_to_string(gpx, gpx_to_string);
 *
 * g_assert(g_strcmp0("bbbbbbbbwwwwwwww........................................wwwwwwwww", gpx_to_string) == 0);
 * @endcode
 *
 * @invariant Parameters `gpx` and `out` must be not `NULL`.
 * The invariants are guarded by assertions.
 * Moreover it is responsibility of the caller to garantee that the `out`
 * pointer refers to a `char` vector thta has `66` positions ore more.
 *
 * @param [in]  gpx the given game position
 * @param [out] out the string written by the function
 */
void
game_position_x_to_string (const GamePositionX const *gpx,
                           char *out)
{
  g_assert(gpx);
  g_assert(out);

  for (int pos = 0; pos < 64; pos++) {
    const SquareSet sq = (SquareSet) 1 << pos;
    char color = '.';
    if (sq & gpx->blacks) {
      color = 'b';
    } else if (sq & gpx->whites) {
      color = 'w';
    }
    *out++ = color;
  }
  *out++ = (gpx->player == BLACK_PLAYER) ? 'b' : 'w';
  *out = '\0';
}

/**
 * @brief Compares game positions `a` and `b`.
 *
 * When the two game position are equal it returns `0`, when `a` is greather then `b` it
 * returns `+1`, otherwise `-1`.
 *
 * Game positions are equals when have the same board and player.
 *
 * @invariant Parameters `a` and `b` must be not `NULL`.
 * Invariants are guarded by assertions.
 *
 * @param [in] a a pointer to a game position x structure
 * @param [in] b a pointer to a second structure
 * @return       `-1` when `a < b`, `+1` when `a > b`, or `0` when the two game position are equal
 */
int
game_position_x_compare (const GamePositionX *const a,
                         const GamePositionX *const b)
{
  g_assert(a);
  g_assert(b);

  if (a == b) return 0;

  if (a->blacks < b->blacks) {
    return -1;
  } else if (a->blacks > b->blacks) {
    return +1;
  } else {          /* Blacks are equal. */
    if (a->whites < b->whites) {
      return -1;
    } else if (a->whites > b->whites) {
      return +1;
    } else {        /* Also whites are equal. */
      if (a->player < b->player) {
        return -1;
      } else if (a->player > b->player) {
        return +1;
      } else {
        return 0;   /* Players are equal, and so are a and b. */
      }
    }
  }
}

/**
 * @brief Used for the score at the end of the game.
 *
 * @details Returns the disk difference between the player and her opponent,
 * assigning the empty squares to the player having most discs.
 *
 * From the web site of the World Othello Federation,
 * World Othello Chanpionship Rules, scoring:<br>
 * <em>"At the end of the game, if both players have completed their moves in
 * the allowed time, the winner is the player with the greater number of
 * discs of his colour on the board at the end. The official score of the
 * game will be determined by counting up the discs of each colour on the
 * board, counting empty squares for the winner. In the event of a draw,
 * the score will always be 32-32"</em>.
 *
 * @param [in] gpx a pointer to the game position x structure
 * @return         the game score according to the WOF rules
 */
int
game_position_x_final_value (const GamePositionX *const gpx)
{
  const int b_count = bit_works_popcount(gpx->blacks);
  const int w_count = bit_works_popcount(gpx->whites);
  const int difference = b_count - w_count;
  if (difference == 0) return 0;
  const int empties = 64 - (b_count + w_count);
  const int delta = (difference > 0) ? difference + empties : difference - empties;
  return (gpx->player == BLACK_PLAYER) ? +delta : -delta;
}

/**
 * @brief Returns a formatted string showing a 2d graphical represention of the game position x.
 *
 * The returned string has a dynamic extent set by a call to malloc. It must then properly
 * garbage collected by a call to free when no more referenced.
 *
 * @invariant Parameter `gpx` must be not `NULL`.
 * Invariants are guarded by assertions.
 *
 * @param [in] gpx a pointer to the game position x structure
 * @return         a string being a 2d representation of the game position
 */
gchar *
game_position_x_print (const GamePositionX *const gpx)
{
  g_assert(gpx);
  GamePosition *gp = game_position_x_gpx_to_gp(gpx);
  gchar *result = game_position_print(gp);
  game_position_free(gp);
  return result;
}

/**
 * @brief Returns if the game state admit one or more legal moves.
 *
 * @invariant Parameter `gpx` must be not `NULL`.
 * Invariants are guarded by assertions.
 *
 * @param [in] gpx the given game position x
 * @return         true if the game state admit a legal move
 */
gboolean
game_position_x_has_any_legal_move (const GamePositionX *const gpx)
{
  g_assert(gpx);

  return (empty_square_set == game_position_x_legal_moves(gpx)) ? FALSE : TRUE;
}

/**
 * @brief Returns `TRUE` if the game position x is not final.
 *
 * @invariant Parameter `gpx` must be not `NULL`.
 * All invariants are guarded by assertions.
 *
 * @param [in] gpx the given game position x
 * @return         true if one of the player has one or more legal moves
 */
gboolean
game_position_x_has_any_player_any_legal_move (const GamePositionX *const gpx)
{
  g_assert(gpx);

  const SquareSet empties = game_position_x_empties(gpx);
  const SquareSet blacks = gpx->blacks;
  const SquareSet whites = gpx->whites;

  for (Direction dir = NW; dir <= SE; dir++) {
    SquareSet wave;
    int shift;
    wave = direction_shift_square_set(dir, empties) & whites;
    shift = 1;
    while (wave != empty_square_set) {
      wave = direction_shift_square_set(dir, wave);
      shift++;
      if ((wave & blacks) != empty_square_set) return TRUE;
      wave &= whites;
    }
    wave = direction_shift_square_set(dir, empties) & blacks;
    shift = 1;
    while (wave != empty_square_set) {
      wave = direction_shift_square_set(dir, wave);
      shift++;
      if ((wave & whites) != empty_square_set) return TRUE;
      wave &= blacks;
    }
  }
  return FALSE;
}

/**
 * @brief Returns true if the `move` is legal for the game position x.
 *
 * @invariant Parameter `gpx` must be not `NULL`.
 * Parameter `move` must be a value in the range defined by
 * the #square_is_valid_move function.
 * Invariants are guarded by assertions.
 *
 * @param [in] gpx  the given game position x
 * @param [in] move the square where to put the new disk
 * @return          true if the move is legal
 */
gboolean
game_position_x_is_move_legal (const GamePositionX *const gpx,
                               const Square move)
{
  g_assert(gpx);
  g_assert(square_is_valid_move(move));

  Board board;
  board.blacks = gpx->blacks;
  board.whites = gpx->whites;

  return board_is_move_legal(&board, move, gpx->player);
}

/**
 * @brief Executes a game move.
 *
 * @details The moving player places a disc in the square identified by the `move` parameter.
 *          The `current` position is unchanged, the new position is assigned to the structure
 *          pointed by the `updated` parameter.
 *
 * @invariant Parameter `current` must be not `NULL`.
 * Parameter `updated` must be not `NULL`.
 * Parameter `move` must be a value in the range defined by
 * the #square_is_valid_move function.
 * Parameter `move` must be legal.
 * Invariants are guarded by assertions.
 *
 * @param [in]  current the given game position x
 * @param [in]  move    the square where to put the new disk
 * @param [out] updated the updated game position x as a rusult of the move
 */
void
game_position_x_make_move (const GamePositionX *const current,
                           const Square move,
                           GamePositionX *const updated)
{
  g_assert(current);
  g_assert(updated);
  g_assert(square_is_valid_move(move));
  g_assert(game_position_x_is_move_legal(current, move));

  if (move == pass_move) {
    game_position_x_pass(current, updated);
    return;
  }

  const Player p = current->player;
  const Player o = player_opponent(p);
  const SquareSet blacks = current->blacks;
  const SquareSet whites = current->whites;
  const SquareSet p_bit_board = (p == BLACK_PLAYER) ? blacks : whites;
  const SquareSet o_bit_board = (p == BLACK_PLAYER) ? whites : blacks;
  const int column = move % 8;
  const int row = move / 8;

  SquareSet new_bit_board[2];
  const SquareSet unmodified_mask = ~bitboard_mask_for_all_directions[move];
  new_bit_board[p] = p_bit_board & unmodified_mask;
  new_bit_board[o] = o_bit_board & unmodified_mask;

  int shift_distance;
  uint8_t p_bitrow;
  uint8_t o_bitrow;

  /* Axis HO. */
  const uint8_t right_shift_for_HO = 8 * row;
  p_bitrow = axis_transform_to_row_one(HO, p_bit_board >> right_shift_for_HO);
  o_bitrow = axis_transform_to_row_one(HO, o_bit_board >> right_shift_for_HO);
  p_bitrow = board_bitrow_changes_for_player(p_bitrow, o_bitrow, column);
  o_bitrow &= ~p_bitrow;
  new_bit_board[p] |= ((SquareSet) p_bitrow << right_shift_for_HO);
  new_bit_board[o] |= ((SquareSet) o_bitrow << right_shift_for_HO);

  /* Axis VE. */
  p_bitrow = axis_transform_to_row_one(VE, p_bit_board >> column);
  o_bitrow = axis_transform_to_row_one(VE, o_bit_board >> column);
  p_bitrow = board_bitrow_changes_for_player(p_bitrow, o_bitrow, row);
  o_bitrow &= ~p_bitrow;
  new_bit_board[p] |= axis_transform_back_from_row_one(VE, p_bitrow) << column;
  new_bit_board[o] |= axis_transform_back_from_row_one(VE, o_bitrow) << column;

  /* Axis DD. */
  shift_distance = axis_shift_distance(DD, column, row);
  p_bitrow = axis_transform_to_row_one(DD, bit_works_signed_left_shift(p_bit_board, shift_distance));
  o_bitrow = axis_transform_to_row_one(DD, bit_works_signed_left_shift(o_bit_board, shift_distance));
  p_bitrow = board_bitrow_changes_for_player(p_bitrow, o_bitrow, column);
  o_bitrow &= ~p_bitrow;
  new_bit_board[p] |= bit_works_signed_left_shift(axis_transform_back_from_row_one(DD, p_bitrow), -shift_distance);
  new_bit_board[o] |= bit_works_signed_left_shift(axis_transform_back_from_row_one(DD, o_bitrow), -shift_distance);

  /* Axis DU. */
  shift_distance = axis_shift_distance(DU, column, row);
  p_bitrow = axis_transform_to_row_one(DU, bit_works_signed_left_shift(p_bit_board, shift_distance));
  o_bitrow = axis_transform_to_row_one(DU, bit_works_signed_left_shift(o_bit_board, shift_distance));
  p_bitrow = board_bitrow_changes_for_player(p_bitrow, o_bitrow, column);
  o_bitrow &= ~p_bitrow;
  new_bit_board[p] |= bit_works_signed_left_shift(axis_transform_back_from_row_one(DU, p_bitrow), -shift_distance);
  new_bit_board[o] |= bit_works_signed_left_shift(axis_transform_back_from_row_one(DU, o_bitrow), -shift_distance);

  updated->player = o;
  updated->blacks = new_bit_board[0];
  updated->whites = new_bit_board[1];
  return;
}


/**
 * @cond
 */

/*
 * Internal functions.
 */

/*
 * Used to initialize the `bitrow_changes_for_player_array`.
 *
 * array a uint8_t array having the row changes for the given index value
 */
static void
board_initialize_bitrow_changes_for_player_array (uint8_t *array)
{
  for (int player_row_count = 0; player_row_count < 256; player_row_count++) {
    const uint8_t player_row = (uint8_t) player_row_count;
    for (int opponent_row_count = 0; opponent_row_count < 256; opponent_row_count++) {
      const uint8_t opponent_row = (uint8_t) opponent_row_count;
      const uint8_t filled_in_row = player_row | opponent_row;
      const uint8_t empties_in_row = ~filled_in_row;
      for (uint8_t move_position = 0; move_position < 8; move_position++) {
        const uint8_t move = 1 << move_position;
        const int array_index = player_row
                                | (opponent_row << 8)
                                | (move_position << 16);

        uint8_t player_row_after_move;

        /*
         * It checks two conditions that cannot happen because are illegal.
         * First player and opponent cannot have overlapping discs.
         * Second the move cannot overlap existing discs.
         * When either one of the two condition applies the result is set being equal
         * to the player row index. Otherwise when black and white do not overlap,
         * and the move is on an empy square it procede with the else block.
         */
        if (((player_row & opponent_row) != 0) || ((move & filled_in_row) != 0)) {
          player_row_after_move = player_row;
        } else {

          /* The square of the move is added to the player configuration of the row after the move. */
          player_row_after_move = player_row | move;

          /*
           * The potential bracketing disc on the right is the first player disc found moving
           * on the left starting from the square of the move.
           */
          const uint8_t potential_bracketing_disc_on_the_left = bit_works_highest_bit_set_8(player_row & (move - 1));

          /*
           * The left rank is the sequence of adiacent discs that start from the bracketing disc and end
           * with the move disc.
           */
          const uint8_t left_rank = bit_works_fill_in_between(potential_bracketing_disc_on_the_left | move);

          /*
           * If the rank contains empy squares, this is a fake flip, and it doesn't do anything.
           * If the rank is full, it cannot be full of anything different than opponent discs, so
           * it adds the discs to the after move player configuration.
           */
          if ((left_rank & empties_in_row) == 0x00) {
            player_row_after_move |= left_rank;
          }

          /* Here it does the same procedure computed on the left also on the right. */
          const uint8_t potential_bracketing_disc_on_the_right = bit_works_lowest_bit_set_8(player_row & ~(move - 1));
          const uint8_t right_rank = bit_works_fill_in_between(potential_bracketing_disc_on_the_right | move);
          if ((right_rank & empties_in_row) == 0x00) {
            player_row_after_move |= right_rank;
          }

          /*
           * It checks that the after move configuration is different from
           * the starting one for the player.
           * This case can happen because it never checked that
           * the bracketing piece was not adjacent to the move disc,
           * on such a case, on both side, the move is illegal, and it is recorded setting
           * the result configuation appropriately.
           */
          if (player_row_after_move == (player_row | move)) {
            player_row_after_move = player_row;
          }
        }

        /* Assigns the computed player row to the proper array position. */
        array[array_index] = player_row_after_move;

      }
    }
  }
}

/*
 * @brief Used to initialize the `shift_square_set_by_amount_mask_array`.
 *
 * @param array a SquareSet array having the board mask for the given index value
 */
static void
board_initialize_shift_square_set_by_amount_mask_array (SquareSet *array)
{
  const SquareSet full_board = 0xFFFFFFFFFFFFFFFF;
  for (Direction dir = NW; dir <= SE; dir++) {
    const int i_dir = dir;
    for (int amount = 0; amount < 8; amount++) {
      const int array_index = amount | (i_dir << 3);
      SquareSet mask = full_board;
      for (int i = 0; i < amount; i++) {
        mask = direction_shift_square_set(dir, mask);
      }
      array[array_index] = mask;
    }
  }
}

/*
 * @brief Returns a new #SquareSet value by shifting back the `squares` parameter
 * by a number of positions as given by the `amount` parameter.
 *
 * Amount must be in the 0..7 range, meaning that 0 is equal to no shift, 1 is
 * on position, and so on.
 *
 * It is safe to call this function only after a number of shift call equal to the amount value.
 * This is becouse the function doesn't mask after shifting.
 *
 * @invariant Parameter `dir` must belong to the #Direction enum.
 * The invariant is not guarded by an assertion.
 *
 * @invariant Parameter `amount` must be in the range 0..7 enum.
 * The invariant is not guarded by an assertion.
 *
 * @param [in] dir     the direction to shift to
 * @param [in] squares the squares set on the bitboard
 * @param [in] amount  the amount to shift
 * @return             the shifted squares
*/
static SquareSet
direction_shift_back_square_set_by_amount (const Direction dir,
                                           const SquareSet squares,
                                           const int amount)
{
  switch (dir) {
  case NW: return squares >> (9 * amount);
  case N:  return squares >> (8 * amount);
  case NE: return squares >> (7 * amount);
  case W:  return squares >> (1 * amount);
  case E:  return squares << (1 * amount);
  case SW: return squares << (7 * amount);
  case S:  return squares << (8 * amount);
  case SE: return squares << (9 * amount);
  default: abort();
  }
}

/**
 * @endcond
 */
