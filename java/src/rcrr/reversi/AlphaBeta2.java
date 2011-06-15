/*
 *  AlphaBeta2.java
 *
 *  Copyright (c) 2010, 2011 Roberto Corradini. All rights reserved.
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

package rcrr.reversi;

import java.util.List;

/**
 * The {@code AlphaBeta2} class implements {@code DecisionRule} and provides
 * an implementation of the search method applying the alpha-beta pruning
 * algorithm.
 * Compared with the AlphaBeta, this class provides some features that govern
 * the search ordering of the moves.
 * <p>
 * The alpha-beta family of algorithms is described by the wikipedia
 * page: <a href="http://en.wikipedia.org/wiki/Alpha-beta_pruning">alpha-beta pruning</a>.
 */
public final class AlphaBeta2 extends AbstractDecisionRule {

    /**
     * Class static factory.
     *
     * @return a new alphabeta instance
     */
    public static AlphaBeta2 getInstance() {
        return new AlphaBeta2();
    }

    /** Class constructor. */
    private AlphaBeta2() { };

    /**
     * Implemented by means of the alpha-beta algorithm.
     *
     * @param player     the player having the move
     * @param board      the board
     * @param achievable the upper bound
     * @param cutoff     the lower bound
     * @param ply        the search depth
     * @param ef         the evaluation function
     * @return a new search node
     */
    public SearchNode search(final Player player,
                             final Board board,
                             final int achievable,
                             final int cutoff,
                             final int ply,
                             final EvalFunction ef) {
        SearchNode node;
        final Player opponent = player.opponent();
        if (ply == 0) {
            node = SearchNode.valueOf(null, ef.eval(GamePosition.valueOf(board, player)));
        } else {
            List<Square> moves = board.legalMoves(player);
            if (moves.isEmpty()) {
                if (board.hasAnyLegalMove(opponent)) {
                    node = search(opponent, board, -cutoff, -achievable, ply - 1, ef).negated();
                } else {
                    node = SearchNode.valueOf(null, finalValue(board, player));
                }
            } else {
                node = SearchNode.valueOf(moves.get(0), achievable);
                outer: for (Square move : moves) {
                    Board board2 = board.makeMove(move, player);
                    int val = search(opponent, board2, -cutoff, -node.value(), ply - 1, ef).negated().value();
                    if (val > node.value()) {
                        node = SearchNode.valueOf(move, val);
                    }
                    if (node.value() >= cutoff) { break outer; }
                }
            }
        }
        return node;
    }

}
