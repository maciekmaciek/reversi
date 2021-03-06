/*
 *  BitBoard1.java
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
 *  ERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MUST  02111-1307, USA
 *  or visit the site <http://www.gnu.org/licenses/>.
 */

package rcrr.reversi.board;

import java.util.Map;
import java.util.List;

/**
 * A board concrete implementation in the bitboard family.
 * <p>
 * The board uses advanced tecniques for computing the isLegal method.
 * legalMoves relate on a pre computed set of likely moves that reduces the number of isLegal tests.
 * makeMoves is completely rewritten having a logic thta mimic the isLegal computation.
 * <p>
 * {@code BitBoard1} is immutable.
 * <p>
 * @see Square
 */
public class BitBoard1 extends BitBoard {

    /** It turns on or off the class logging for performances. */
    private static final boolean LOG = true;

    /** Collects the number of call to legalMoves method. */
    private static int callsTolegalMoves = 0;

    /** Collects the number of call to makeMove method. */
    private static int callsToMakeMove = 0;

    /** Collects the number of call to the class constructor. */
    private static int callsToConstructor = 0;

    /** Collects the number of call to isLegal method. */
    private static int callsToIsLegal = 0;

    /** Collects the number of likely moves in the legalMoves method. */
    private static int numberOflikelyMoves = 0;

    /** Collects the number of legal moves in the legalMoves method. */
    private static int numberOflegalMoves = 0;

    /** Caches the direction enum values in a local array. */
    private static final Direction[] DIRECTION_VALUES = Direction.values();

    /** Caches the square enum values in a local array. */
    private static final Square[] SQUARE_VALUES = Square.values();

    /** Caches the axis enum values in a local array. */
    private static final Axis[] AXIS_VALUES = Axis.values();

    /** A bitboard being all set with the exception of column A. */
    private static final long ALL_SQUARES_EXCEPT_COLUMN_A = 0xFEFEFEFEFEFEFEFEL;

    /** A bitboard being all set with the exception of column H. */
    private static final long ALL_SQUARES_EXCEPT_COLUMN_H = 0x7F7F7F7F7F7F7F7FL;

    /** Macic number 3. */
    private static final int MAGIC_NUMBER_3 = 3;

    /** Macic number 7. */
    private static final int MAGIC_NUMBER_7 = 7;

    /** Macic number 8. */
    private static final int MAGIC_NUMBER_8 = 8;

    /** Macic number 16. */
    private static final int MAGIC_NUMBER_16 = 16;

    /** Macic number 256. */
    private static final int MAGIC_NUMBER_256 = 256;

    /**
     * This array has sixtyfour entries. The index, having range 0-63, represent one of the squares
     * of the table. Each entry is a bitboard mask having set all the squares that are
     * reachable moving along the eigth directions, when starting from the square identified by
     * the index itself.
     * <p>
     * Values do not change.
     */
    private static final long[] BITBOARD_MASK_FOR_ALL_DIRECTIONS = new long[] {
        0x81412111090503FEL, 0x02824222120A07FDL, 0x0404844424150EFBL, 0x08080888492A1CF7L,
        0x10101011925438EFL, 0x2020212224A870DFL, 0x404142444850E0BFL, 0x8182848890A0C07FL,
        0x412111090503FE03L, 0x824222120A07FD07L, 0x04844424150EFB0EL, 0x080888492A1CF71CL,
        0x101011925438EF38L, 0x20212224A870DF70L, 0x4142444850E0BFE0L, 0x82848890A0C07FC0L,
        0x2111090503FE0305L, 0x4222120A07FD070AL, 0x844424150EFB0E15L, 0x0888492A1CF71C2AL,
        0x1011925438EF3854L, 0x212224A870DF70A8L, 0x42444850E0BFE050L, 0x848890A0C07FC0A0L,
        0x11090503FE030509L, 0x22120A07FD070A12L, 0x4424150EFB0E1524L, 0x88492A1CF71C2A49L,
        0x11925438EF385492L, 0x2224A870DF70A824L, 0x444850E0BFE05048L, 0x8890A0C07FC0A090L,
        0x090503FE03050911L, 0x120A07FD070A1222L, 0x24150EFB0E152444L, 0x492A1CF71C2A4988L,
        0x925438EF38549211L, 0x24A870DF70A82422L, 0x4850E0BFE0504844L, 0x90A0C07FC0A09088L,
        0x0503FE0305091121L, 0x0A07FD070A122242L, 0x150EFB0E15244484L, 0x2A1CF71C2A498808L,
        0x5438EF3854921110L, 0xA870DF70A8242221L, 0x50E0BFE050484442L, 0xA0C07FC0A0908884L,
        0x03FE030509112141L, 0x07FD070A12224282L, 0x0EFB0E1524448404L, 0x1CF71C2A49880808L,
        0x38EF385492111010L, 0x70DF70A824222120L, 0xE0BFE05048444241L, 0xC07FC0A090888482L,
        0xFE03050911214181L, 0xFD070A1222428202L, 0xFB0E152444840404L, 0xF71C2A4988080808L,
        0xEF38549211101010L, 0xDF70A82422212020L, 0xBFE0504844424140L, 0x7FC0A09088848281L
    };

    /**
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
     */
    private static final byte[] BITROW_CHANGES_FOR_PLAYER_ARRAY = initializeBitrowChangesForPlayerArray();

    /**
     * Returns info for performance statistics.
     *
     * @return a string having class performance statistics
     */
    public static String printLog() {
        String ret = "callsTolegalMoves=" + callsTolegalMoves + ", callsToMakeMove=" + callsToMakeMove
            + ", callsToConstructor=" + callsToConstructor + ", numberOflikelyMoves=" + numberOflikelyMoves
            + ", numberOflegalMoves=" + numberOflegalMoves + ", callsToIsLegal=" + callsToIsLegal;
        return ret;
    }

    /**
     * Base static factory for the class.
     * <p>
     * {@code squares} must be not null, and must have an entry for every board square.
     * Given that the map cannot have duplicate keys, its size must be equal to the number
     * of class instances defined by the {@code Square} enum.
     *
     * @param  squares the map of squares
     * @return         a new board having as state the given square map
     * @throws NullPointerException     if parameter {@code squares} is null
     * @throws IllegalArgumentException if the {@code squares} is not complete
     */
    static Board valueOf(final Map<Square, SquareState> squares) {
        BoardUtils.checkForConsistencyTheSquareMap(squares);
        return new BitBoard1(BoardUtils.mapToBitboard(squares));
    }

    /**
     * Static factory for the class.
     * <p>
     * {@code bitboard} must be not null, and must have a size equal to
     * two. Overlapping bit set are not valid.
     * Precondition on the {@code bitboard} parameter are not enforced.
     *
     * @param  bitboard the bitboard field
     * @return         a new board having as state the given bitboard array
     */
    static Board valueOf(final long[] bitboard) {
        return new BitBoard1(bitboard);
    }

    /**
     * Returns an 8-bit row representation of the player pieces after applying the move.
     *
     * @param playerRow    8-bit bitboard corrosponding to player pieces
     * @param opponentRow  8-bit bitboard corrosponding to opponent pieces
     * @param movePosition square to move
     * @return             the new player's row index after making the move
     */
    private static int bitrowChangesForPlayer(final int playerRow, final int opponentRow, final int movePosition) {
        final int arrayIndex = playerRow | (opponentRow << MAGIC_NUMBER_8) | (movePosition << MAGIC_NUMBER_16);
        return (int) BITROW_CHANGES_FOR_PLAYER_ARRAY[arrayIndex] & BYTE_MASK_FOR_INT;
    }

    /**
     * Used to initialize the BITROW_CHANGES_FOR_PLAYER_ARRAY.
     *
     * @return a byte array having the row changes for the given index value
     */
    private static byte[] initializeBitrowChangesForPlayerArray() {

        final byte[] arrayResult = new byte[MAGIC_NUMBER_256 * MAGIC_NUMBER_256 * MAGIC_NUMBER_8];
        for (int playerRow = 0; playerRow < MAGIC_NUMBER_256; playerRow++) {
            for (int opponentRow = 0; opponentRow < MAGIC_NUMBER_256; opponentRow++) {
                final int filledInRow = playerRow | opponentRow;
                final int emptiesInRow = ~(filledInRow) & BYTE_MASK_FOR_INT;
                for (int movePosition = 0; movePosition < MAGIC_NUMBER_8; movePosition++) {
                    final int move = 1 << movePosition;
                    final int arrayResultIndex = playerRow
                        | (opponentRow << MAGIC_NUMBER_8)
                        | (movePosition << MAGIC_NUMBER_16);

                    int playerRowAfterMove;

                    /**
                     * It checks two conditions that cannot happen because are illegal.
                     * First player and opponent cannot have overlapping discs.
                     * Second the move cannot overlap existing discs.
                     * When either one of the two condition applies the result is set being equal
                     * to the player row index. Otherwise when black and white do not overlap,
                     * and the move is on an empy square it procede with the else block.
                     **/
                    if (((playerRow & opponentRow) != 0) || ((move & filledInRow) != 0)) {
                        playerRowAfterMove = playerRow;
                    } else {

                        /** The square of the move is added to the player configuration of the row after the move. */
                        playerRowAfterMove = playerRow | move;

                        /**
                         * The potential bracketing disc on the right is the first player disc found moving
                         * on the left starting from the square of the move.
                         */
                        final int potentialBracketingDiscOnTheLeft = BitWorks.highestBitSet(playerRow & (move - 1));

                        /**
                         * The left rank is the sequence of adiacent discs that start from the bracketing disc and end
                         * with the move disc.
                         */
                        final int leftRank = BitWorks.fillInBetween(potentialBracketingDiscOnTheLeft | move);

                        /**
                         * If the rank contains empy squares, this is a fake flip, and it doesn't do anything.
                         * If the rank is full, it cannot be full of anything different than opponent discs, so
                         * it adds the discs to the after move player configuration.
                         */
                        if ((leftRank & emptiesInRow) == 0) {
                            playerRowAfterMove |= leftRank;
                        }

                        /** Here it does the same computed on the left also on the right. */
                        final int potentialBracketingDiscOnTheRight = BitWorks.lowestBitSet(playerRow & ~(move - 1));
                        final int rightRank = BitWorks.fillInBetween(potentialBracketingDiscOnTheRight | move);
                        if ((rightRank & emptiesInRow) == 0) {
                            playerRowAfterMove |= rightRank;
                        }

                        /**
                         * It checks that the after move configuration is different from
                         * the starting one for the player.
                         * This case can happen because it never checked that
                         * the bracketing piece was not adjacent to the move disc,
                         * on such a case, on both side, the move is illegal, and it is recorded setting
                         * the result configuation appropriately.
                         */
                        if (playerRowAfterMove == (playerRow | move)) {
                            playerRowAfterMove = playerRow;
                        }
                    }

                    /** Assigns the computed player row index to the proper array position. */
                    arrayResult[arrayResultIndex] = (byte) playerRowAfterMove;

                }
            }
        }

        return arrayResult;
    }

    /**
     * Returns all the cells surrounding the the squares given as parameter.
     * It gives back also the original squares.
     *
     * @param squares the set of given squares
     * @return        the union of given and surrounding squares
     */
    private static long neighbors(final long squares) {
        long neighbors = squares;
        neighbors |= (neighbors >>> MAGIC_NUMBER_8);
        neighbors |= (neighbors >>> 1) & ALL_SQUARES_EXCEPT_COLUMN_H;
        neighbors |= (neighbors <<  1) & ALL_SQUARES_EXCEPT_COLUMN_A;
        neighbors |= (neighbors <<  MAGIC_NUMBER_8);
        return neighbors;
    }

    /**
     * Lazily initialized, cached legalMoves.
     * In case of a multi-threaded use must be applied a ReadWriteLock on this field.
     */
    private final transient long[] legalMovesCache = new long[] {0L, 0L};

    /** Flag for tracking that the legalMovesCache field has been computed. */
    private final transient boolean[] hasLegalMovesBeenComputed = new boolean[] {false, false};

    /**
     * Class constructor.
     * <p>
     * {@code bitboard} must be not null, and must have a size equal to
     * two. Overlapping bit set to one are not allowed.
     *
     * @param  bitboard the bitboard field
     */
    BitBoard1(final long[] bitboard) {
        super(bitboard);
        if (LOG) { callsToConstructor++; }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean isLegal(final Square move, final Player player) {
        if (LOG) { callsToIsLegal++; }
        isLegalInvariantsAreSatisfied(move, player);
        return isLegal(1L << move.ordinal(), player.ordinal());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public List<Square> legalMoves(final Player player) {
        if (LOG) { callsTolegalMoves++; }
        if (player == null) { throw new NullPointerException("Parameter player must be not null."); }
        return new SquareList(legalMoves(player.ordinal()));
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Board makeMove(final Square move, final Player player) {
        if (LOG) { callsToMakeMove++; }
        makeMoveInvariantsAreSatisfied(move, player);
        return valueOf(makeMoveImpl(move, player.ordinal()));
    }

    /**
     * Returns the value of the hasLegalMovesBeenComputed field for the {@code player}.
     *
     * @param player the player associated vith the legal move list
     * @return       true if the cached value has been already computed
     */
    boolean hasLegalMovesBeenComputed(final int player) { return hasLegalMovesBeenComputed[player]; }

    /**
     * Returns the boolean value telling if the move, done by the
     * specified player, is legal.
     * <p>
     * Parameter {@code move} must have one bit set.
     * Parameter {@code player} must be either {@code 0} or {@code 1}.
     *
     * @param move   the square where to put the new disk
     * @param player the player moving
     * @return       true if the move is legal, otherwise false
     */
    boolean isLegal(final long move, final int player) {

        if ((move & empties()) == 0L) { return false; }

        if (hasLegalMovesBeenComputed(player)) {
            return (move & legalMovesCache(player)) != 0L;
        }

        final long playerBitboard = bitboard(player);
        final long opponentBitboard = bitboard(opponent(player));

        final int[] xy = BitWorks.bitscanMS1BtoBase8(move);
        final int column = xy[0];
        final int row    = xy[1];

        /**
         * The following commented loop is the code as it should be.
         * Unluckly it is 70% slover than the proposed code.
         * Reasons are:
         * - The loop introduces overhead.
         * - The axis methods are fair for DD and DU, introduce a bit of inefficiency for VE,
         *   are more penalizing for HO, where the explosion of the code locally enable elisions and semplifications.
         *
         * Optimizations are:
         * - Method moveOrdinalPositionInBitrow is "inlined".
         * - Method shiftDistance is "inlined" for the HO and VE axes.
         * - Method signedLeftShift is "inlined" for the HO and VE axes.
         */

        /**
        for (final Axis axis : AXIS_VALUES) {
            final int moveOrdPosition = axis.moveOrdinalPositionInBitrow(column, row);
            final int shiftDistance   = axis.shiftDistance(column, row);
            final int playerBitrow    = axis.transformToRowOne(BitWorks.signedLeftShift(playerBitboard,   shiftDistance));
            final int opponentBitrow  = axis.transformToRowOne(BitWorks.signedLeftShift(opponentBitboard, shiftDistance));
            if (bitrowChangesForPlayer(playerBitrow, opponentBitrow, moveOrdPosition) != playerBitrow) {
                return true;
            }
        }
        */

        int playerBitrow;
        int opponentBitrow;
        int shiftDistance;

        final int rightShiftForHO = MAGIC_NUMBER_8 * row;
        playerBitrow   = Axis.HO.transformToRowOne(playerBitboard   >>> rightShiftForHO);
        opponentBitrow = Axis.HO.transformToRowOne(opponentBitboard >>> rightShiftForHO);
        if (bitrowChangesForPlayer(playerBitrow, opponentBitrow, column) != playerBitrow) {
            return true;
        }

        playerBitrow   = Axis.VE.transformToRowOne(playerBitboard   >>> column);
        opponentBitrow = Axis.VE.transformToRowOne(opponentBitboard >>> column);
        if (bitrowChangesForPlayer(playerBitrow, opponentBitrow, row) != playerBitrow) {
            return true;
        }

        shiftDistance  = Axis.DD.shiftDistance(column, row);
        playerBitrow   = Axis.DD.transformToRowOne(BitWorks.signedLeftShift(playerBitboard,   shiftDistance));
        opponentBitrow = Axis.DD.transformToRowOne(BitWorks.signedLeftShift(opponentBitboard, shiftDistance));
        if (bitrowChangesForPlayer(playerBitrow, opponentBitrow, column) != playerBitrow) {
            return true;
        }

        shiftDistance  = Axis.DU.shiftDistance(column, row);
        playerBitrow   = Axis.DU.transformToRowOne(BitWorks.signedLeftShift(playerBitboard,   shiftDistance));
        opponentBitrow = Axis.DU.transformToRowOne(BitWorks.signedLeftShift(opponentBitboard, shiftDistance));
        if (bitrowChangesForPlayer(playerBitrow, opponentBitrow, column) != playerBitrow) {
            return true;
        }

        /** If no capture on the four directions happens, return false. */
        return false;
    }

    /**
     * Returns the value of the legalMovesCache field for the player.
     *
     * @param player the player associated vith the legal move list
     * @return       the legal move list for the player
     */
    long legalMovesCache(final int player) { return legalMovesCache[player]; }

    /**
     * Returns a new bitboard long array to reflect move by player.
     * <p>
     * A null value for player is not allowed, but not checked.
     * A null value for move is not allowed, but not checked.
     * The method does not check if the move is legal.
     *
     * @param  move   the board square where to put the disk
     * @param  player the disk color to put on the board
     * @return        a new bitboard array reflecting the move made
     */
    long[] makeMoveImpl(final Square move, final int player) {

        final int opponent = opponent(player);
        final int column = move.column().ordinal();
        final int row = move.row().ordinal();

        final long playerBitboard   = bitboard(player);
        final long opponentBitboard = bitboard(opponent);

        final long[] newbitboard = new long[2];
        final long unmodifiedMask = ~BITBOARD_MASK_FOR_ALL_DIRECTIONS[move.ordinal()];
        newbitboard[player]   = playerBitboard   & unmodifiedMask;
        newbitboard[opponent] = opponentBitboard & unmodifiedMask;

        /**
         * See the comment in the isLegal method. The same considerations apply here.
         */

        /**
        for (final Axis axis : AXIS_VALUES) {
            final int moveOrdPosition = axis.moveOrdinalPositionInBitrow(column, row);
            final int shiftDistance   = axis.shiftDistance(column, row);
            int playerBitrow    = axis.transformToRowOne(BitWorks.signedLeftShift(playerBitboard,   shiftDistance));
            int opponentBitrow  = axis.transformToRowOne(BitWorks.signedLeftShift(opponentBitboard, shiftDistance));
            playerBitrow = bitrowChangesForPlayer(playerBitrow, opponentBitrow, moveOrdPosition);
            opponentBitrow &= ~playerBitrow;
            newbitboard[player]   |= BitWorks.signedLeftShift(axis.transformBackFromRowOne(playerBitrow),   -shiftDistance);
            newbitboard[opponent] |= BitWorks.signedLeftShift(axis.transformBackFromRowOne(opponentBitrow), -shiftDistance);
        }
        */

        int playerBitrow;
        int opponentBitrow;
        int shiftDistance;

        final int rightShiftForHO = MAGIC_NUMBER_8 * row;
        playerBitrow    = Axis.HO.transformToRowOne(playerBitboard   >>> rightShiftForHO);
        opponentBitrow  = Axis.HO.transformToRowOne(opponentBitboard >>> rightShiftForHO);
        playerBitrow = bitrowChangesForPlayer(playerBitrow, opponentBitrow, column);
        opponentBitrow &= ~playerBitrow;
        newbitboard[player]   |= ((long) playerBitrow   << (MAGIC_NUMBER_8 * row));
        newbitboard[opponent] |= ((long) opponentBitrow << (MAGIC_NUMBER_8 * row));

        playerBitrow   = Axis.VE.transformToRowOne(playerBitboard   >>> column);
        opponentBitrow = Axis.VE.transformToRowOne(opponentBitboard >>> column);
        playerBitrow = bitrowChangesForPlayer(playerBitrow, opponentBitrow, row);
        opponentBitrow &= ~playerBitrow;
        newbitboard[player]   |= Axis.VE.transformBackFromRowOne(playerBitrow)   << column;
        newbitboard[opponent] |= Axis.VE.transformBackFromRowOne(opponentBitrow) << column;

        shiftDistance  = Axis.DD.shiftDistance(column, row);
        playerBitrow   = Axis.DD.transformToRowOne(BitWorks.signedLeftShift(playerBitboard,   shiftDistance));
        opponentBitrow = Axis.DD.transformToRowOne(BitWorks.signedLeftShift(opponentBitboard, shiftDistance));
        playerBitrow = bitrowChangesForPlayer(playerBitrow, opponentBitrow, column);
        opponentBitrow &= ~playerBitrow;
        newbitboard[player]   |= BitWorks.signedLeftShift(Axis.DD.transformBackFromRowOne(playerBitrow),   -shiftDistance);
        newbitboard[opponent] |= BitWorks.signedLeftShift(Axis.DD.transformBackFromRowOne(opponentBitrow), -shiftDistance);

        shiftDistance  = Axis.DU.shiftDistance(column, row);
        playerBitrow   = Axis.DU.transformToRowOne(BitWorks.signedLeftShift(playerBitboard,   shiftDistance));
        opponentBitrow = Axis.DU.transformToRowOne(BitWorks.signedLeftShift(opponentBitboard, shiftDistance));
        playerBitrow = bitrowChangesForPlayer(playerBitrow, opponentBitrow, column);
        opponentBitrow &= ~playerBitrow;
        newbitboard[player]   |= BitWorks.signedLeftShift(Axis.DU.transformBackFromRowOne(playerBitrow),   -shiftDistance);
        newbitboard[opponent] |= BitWorks.signedLeftShift(Axis.DU.transformBackFromRowOne(opponentBitrow), -shiftDistance);

        return newbitboard;
    }

    /**
     * Implements the legal moves call by testing the likely moves one by one by calling the isLegal method.
     *
     * @param player the player that has to move
     * @return       legal moves for the player
     */
    private long legalMoves(final int player) {
        long result = 0L;
        if (hasLegalMovesBeenComputed(player)) {
            result = legalMovesCache(player);
        } else {
            for (long likelyMoves = likelyMoves(player);
                 likelyMoves != 0L;
                 likelyMoves = BitWorks.unsetLowestBit(likelyMoves)) {
                final long move = BitWorks.lowestBitSet(likelyMoves);
                if (isLegal(move, player)) { result |= move; }
            }
            setLegalMovesCache(player, result);
            setHasLegalMovesBeenComputed(player, true);
        }
        return result;
    }

    /**
     * Returns a set of likely moves. It is guaranted that the legal moves set
     * is fully contained by the likely move set.
     * On avarage likely moves are 1.7 times the number of legal moves.
     *
     * @param player the player that has to move
     * @return       the set of likely moves
     */
    private long likelyMoves(final int player) {
        return neighbors(bitboard(opponent(player))) & empties();
    }

    /**
     * Sets the value of the hasLegalMovesBeenComputed field for the {@code player}.
     *
     * @param player the player associated vith the legal move list
     * @param value  the value to be assigned to the field
     */
    void setHasLegalMovesBeenComputed(final int player, final boolean value) {
        hasLegalMovesBeenComputed[player] = value;
    }

    /**
     * Sets the value of the legalMovesCache field for the {@code player}.
     *
     * @param player the player associated vith the legal move list
     * @param value  the legal move list for the player
     */
    void setLegalMovesCache(final int player, final long value) {
        legalMovesCache[player] = value;
    }

}
