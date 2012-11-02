/*
 *  Axis.java
 *
 *  Copyright (c) 2012 Roberto Corradini. All rights reserved.
 *
 *  This file is part of the reversi program
 *  http://github.com/rcrr/reversi
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 3, or (at your option) any
 *  later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 *  or visit the site <http://www.gnu.org/licenses/>.
 */

package rcrr.reversi.board;

/**
 * The axes are the lines that pass throw a square. A general
 * square has four axes.
 */
public enum Axis {

    /**
     * Horizontal axis (W-E).
     */
    HO {
        @Override
        public int transformToRowOne(final long squares) {
            return 0;
        }

        @Override
        public long transformBackFromRowOne(final int bitrow) {
            return 0L;
        }
    },

    /**
     * Vertical axis (N-S).
     */
    VE {
        @Override
        public int transformToRowOne(final long squares) {
            return transformColumnAInRow0(squares);
        }

        @Override
        public long transformBackFromRowOne(final int bitrow) {
            return reTransformRow0BackToColumnA(bitrow);
        }
    },

    /**
     * Diagonal Down axis (NW-SE), A1-H8.
     */
    DD {
        @Override
        public int transformToRowOne(final long squares) {
            return transformDiagonalA1H8InRow0(squares);
        }

        @Override
        public long transformBackFromRowOne(final int bitrow) {
            return reTransformRow0BackToDiagonalA1H8(bitrow);
        }
    },

    /**
     * Diagonal Up axis (NE-SW), A8-H1.
     */
    DU {
        @Override
        public int transformToRowOne(final long squares)
        {
            return transformDiagonalH1A8InRow0(squares);
        }

        @Override
        public long transformBackFromRowOne(final int bitrow) {
            return reTransformRow0BackToDiagonalH1A8(bitrow);
        }
    };

    /** The null instance. */
    public static final Axis NULL = null;

    /** The number of axes. */
    public static final int NUMBER_OF = values().length;

    /** Used for masking a byte when using integer values. */
    private static final int BYTE_MASK_FOR_INT = 0xFF;

    /** A bitboard being set on column A. */
    private static final long COLUMN_A = 0x0101010101010101L;

    /** Macic number 7. */
    private static final int MAGIC_NUMBER_7 = 7;

    /** Macic number 8. */
    private static final int MAGIC_NUMBER_8 = 8;

    /** Macic number 14. */
    private static final int MAGIC_NUMBER_14 = 14;

    /** Macic number 16. */
    private static final int MAGIC_NUMBER_16 = 16;

    /** Macic number 28. */
    private static final int MAGIC_NUMBER_28 = 28;

    /** Macic number 32. */
    private static final int MAGIC_NUMBER_32 = 32;

    /** A bitboard being set on diagonal A1-H8. */
    private static final long DIAGONAL_A1_H8 = 0x8040201008040201L;

    /** A bitboard being set on diagonal H1-A8. */
    private static final long DIAGONAL_H1_A8 = 0x0102040810204080L;

    /** A bitboard being set on diagonal H1-A8. */
    private static final long SQUARES_B1_F1_A2_E2 = 0x1122;

    /**
     * Returns a long having the bits along the colomn A set to the corresponding ones
     * on the {@code bitrow} parameter.
     * Bits ranging from 8 to 31 in the {@code bitrow} parameter must be 0.
     * Bit value corresponding to square A1 is moved to A1, B1 to A2, ... , H1 to A8.
     *
     * @param bitrow bits 0 to 7 represents row one, bits forom position 8 to 31 must be 0
     * @return       a bitboard having column A set as the bitboard parameter,
     *               all other position are set to zero
     */
    private static long reTransformRow0BackToColumnA(final int bitrow) {
        int tmp = bitrow;
        tmp |= tmp << MAGIC_NUMBER_7;
        tmp |= tmp << MAGIC_NUMBER_14;
        final long bitboard = (long) tmp | ((long) tmp << MAGIC_NUMBER_28);
        return bitboard & COLUMN_A;
    }

    /**
     * Returns a long having the bits along the diagonal A1-H8 set to the corresponding ones
     * on the {@code bitrow} parameter.
     * Bits ranging from 8 to 31 in the {@code bitrow} parameter must be 0.
     * Bit value corresponding to square A1 is moved to A1, B1 to B2, ... , H1 to H8.
     *
     * @param bitrow bits 0 to 7 represents row one, bits forom position 8 to 31 must be 0
     * @return       a bitboard having diagonal A1-H8 set as the bitboard parameter,
     *               all other position are set to zero
     */
    private static long reTransformRow0BackToDiagonalA1H8(final int bitrow) {
        int tmp = bitrow;
        tmp |= tmp << MAGIC_NUMBER_8;
        long bitboard = (long) tmp | ((long) tmp << MAGIC_NUMBER_16);
        bitboard |= bitboard << MAGIC_NUMBER_32;
        return bitboard & DIAGONAL_A1_H8;
    }

    /**
     * Returns a long having the bits along the diagonal H1-A8 set to the corresponding ones
     * on the {@code bitrow} parameter.
     * Bits ranging from 8 to 31 in the {@code bitrow} parameter must be 0.
     * Bit value corresponding to square A1 is moved to A8, B1 to B7, ... , H1 to H1.
     *
     * @param bitrow bits 0 to 7 represents row one, bits forom position 8 to 31 must be 0
     * @return       a bitboard having diagonal H1-A8 set as the bitboard parameter,
     *               all other position are set to zero
     */
    private static long reTransformRow0BackToDiagonalH1A8(final int bitrow) {
        int tmp = bitrow;
        tmp |= tmp << MAGIC_NUMBER_8;
        tmp |= (tmp & SQUARES_B1_F1_A2_E2) << MAGIC_NUMBER_16;
        final long bitboard = (long) tmp | ((long) tmp << MAGIC_NUMBER_32);
        return bitboard & DIAGONAL_H1_A8;
    }

    /**
     * Returns an int having the bits from position 8 to position 31 set to zero,
     * and the bits from position 0 to position 7, corresponding to Row One in the board,
     * copied from the Column A of the {@code bitboard} parameter.
     * Bit value corresponding to square A1 is moved to A1, A2 to B1, ... , A8 to H1.
     *
     * @param bitboard the bitboard representation for one color
     * @return         colum a copied to row one, all other position are set to zero
     */
    private static int transformColumnAInRow0(final long bitboard) {
        long tmp = bitboard;
        tmp &= COLUMN_A;
        tmp |= tmp >> MAGIC_NUMBER_28;
        tmp |= tmp >> MAGIC_NUMBER_14;
        tmp |= tmp >> MAGIC_NUMBER_7;
        return (int) tmp & BYTE_MASK_FOR_INT;
    }

    /**
     * Returns an int having the bits from position 8 to position 31 set to zero,
     * and the bits from position 0 to position 7, corresponding to Row One in the board,
     * copied from the Diagonal A1-H8 of the {@code bitboard} parameter.
     * Bit value corresponding to square A1 is moved to A1, B2 to B1, ... , H8 to H1.
     *
     * @param bitboard the bitboard representation for one color
     * @return         diagonal A1-H8 copied to row one, all other position are set to zero
     */
    private static int transformDiagonalA1H8InRow0(final long bitboard) {
        long tmp = bitboard;
        tmp &= DIAGONAL_A1_H8;
        tmp |= tmp >> MAGIC_NUMBER_32;
        tmp |= tmp >> MAGIC_NUMBER_16;
        tmp |= tmp >> MAGIC_NUMBER_8;
        return (int) tmp & BYTE_MASK_FOR_INT;
    }

    /**
     * Returns an int having the bits from position 8 to position 31 set to zero,
     * and the bits from position 0 to position 7, corresponding to Row One in the board,
     * copied from the Diagonal H1-A8 of the {@code bitboard} parameter.
     * Bit value corresponding to square A8 is moved to A1, B7 to B1, ... , H1 to H1.
     *
     * @param bitboard the bitboard representation for one color
     * @return         diagonal H1-A8 copied to row one, all other position are set to zero
     */
    private static int transformDiagonalH1A8InRow0(final long bitboard) {
        long tmp = bitboard;
        tmp &= DIAGONAL_H1_A8;
        tmp |= tmp >> MAGIC_NUMBER_32;
        tmp |= tmp >> MAGIC_NUMBER_16;
        tmp |= tmp >> MAGIC_NUMBER_8;
        return (int) tmp & BYTE_MASK_FOR_INT;
    }

    /**
     * Returns an int having the bits from position 8 to position 31 set to zero,
     * and the bits from position 0 to position 7, corresponding to Row One in the board,
     * transformed from:
     * <ul>
     *   <li>ROW 1 for the HO axis.</li>
     *   <li>COLUMN A for the VE axis.</li>
     *   <li>DIAGONAL A1-H8 for the DD axis.</li>
     *   <li>DIAGONAL A8-H1 for the DU axis.</li>
     * </ul>
     *
     * @param squares the set of board squares packed in a long field
     * @return        the transformed file
     */
    public abstract int transformToRowOne(final long squares);

    public abstract long transformBackFromRowOne(final int bitrow);

}
