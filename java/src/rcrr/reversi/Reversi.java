/*
 *  Reversi.java
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
import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;

import java.io.PrintStream;

import org.joda.time.Period;
import org.joda.time.Duration;

/**
 * The reversi class {@code Reversi} is the main entry point for the program.
 * <p>
 * Documentation has to be completed.
 * Testing has to be completed.
 */
public final class Reversi {

    /** Error code 1. */
    private static final int ERROR_CODE_1 = 1;

    /** Error code 2. */
    private static final int ERROR_CODE_2 = 2;

    /** Error code 3. */
    private static final int ERROR_CODE_3 = 3;

    /** Error code 4. */
    private static final int ERROR_CODE_4 = 4;

    /** Error code 5. */
    private static final int ERROR_CODE_5 = 5;

    /** Default game duration in minutes. */
    private static final int DEFAULT_GAME_DURATION_IN_MINUTES = 30;

    /** Class constructor. Not used so far. */
    private Reversi() {
        throw new UnsupportedOperationException();
    }

    /**
     * Plays a game returning the score.
     * <p>
     * Documentation has to be completed.
     * Testing has to be completed.
     * It has to be fully refactored.
     *
     * @param blStrategy   the black's strategy
     * @param whStrategy   the white's strategy
     * @param ps           the output print stream
     * @param gameDuration the game duration assigned to both players
     *
     * @return           the game score
     */
    public static int reversi(final Strategy blStrategy,
                              final Strategy whStrategy,
                              final PrintStream ps,
                              final Duration gameDuration) {

        /** Must be revised!!!! */
        final Actor black = Actor.valueOf("Black Actor", blStrategy);
        final Actor white = Actor.valueOf("White Actor", whStrategy);
        final Game game = Game.initialGame(black, white, gameDuration, ps);
        game.play();
        return game.countDiscDifference();
    }

    /**
     * Plays a series of 2*nPairs games, swapping sides. Returns a results map
     * having as key:
     * <ul>
     *   <li>{@code scores}</li>
     *   <li>{@code numberOfWins}</li>
     *   <li>{@code totalOfPointDiff}</li>
     * </ul>
     * <p>
     * The {@code scores} key references a {@code List<Integer>} value that contains
     * the game scores of this series.
     * <p>
     * The {@code numberOfWins} key references a {@code double} value where each win
     * count 1, a loss count -1, and a tie is valued 0.
     * <p>
     * The {@code totalOfPointDiff} key references a {@code long} value that sum all
     * the game scores.
     * <p>
     * All the results are calculated from the strategyOne's point of view.
     *
     * @param strategyOne  the first strategy
     * @param strategyTwo  the second strategy
     * @param nPairs       the number of paired game to play
     * @param gameDuration the game duration assigned to both players
     *
     * @return             a map hosting the scores, the number of wins,
     *                     the total of point difference
     */
    public static Map<String, Object> reversiSeries(final Strategy strategyOne,
                                                    final Strategy strategyTwo,
                                                    final int nPairs,
                                                    final Duration gameDuration) {
        final Map<String, Object> results = new HashMap<String, Object>();
        final List<Integer> scores = new ArrayList<Integer>();
        long totalOfPointDiff = 0;
        double numberOfWins = 0.;
        for (int i = 0; i < nPairs; i++) {

            scores.add(+ reversi(strategyOne, strategyTwo, new NullPrintStream(), gameDuration));
            scores.add(- reversi(strategyTwo, strategyOne, new NullPrintStream(), gameDuration));
        }
        for (int score : scores) {
            totalOfPointDiff += score;
            if (score > 0) { numberOfWins++; }
            else if (score == 0) { numberOfWins += 0.5; }
            else { }
        }
        results.put("scores", scores);
        results.put("totalOfPointDiff", totalOfPointDiff);
        results.put("numberOfWins", numberOfWins);
        return results;
    }

    private static Game.Builder randomInitialization(final Strategy strategyOne,
                                                     final Strategy strategyTwo,
                                                     final Duration gameDuration,
                                                     final int numberOfRandomMoves) {
        return new Game.Builder()
            .withActors(new ActorsPair.Builder()
                        .withActor(Player.BLACK, new Actor.Builder()
                                   .withStrategy(strategyOne)
                                   .build())
                        .withActor(Player.WHITE, new Actor.Builder()
                                   .withStrategy(strategyTwo)
                                   .build())
                        .build())
            .withPrintStream(new NullPrintStream())
            .withSequence(new GameSequence.Builder()
                          .withSnapshots(new GameSnapshot.Builder()
                                         .withClock(Clock.initialClock(gameDuration))
                                         .withPosition(Game.randomGame(numberOfRandomMoves).position())
                                         .withRegister(MoveRegister.empty())
                                         .build())
                          .build());
    }

    private static Game.Builder flipActors(final Game.Builder gb) {
        Actor one = gb.build().actors().get(Player.BLACK);
        Actor two = gb.build().actors().get(Player.WHITE);
        return gb.withActors(new ActorsPair.Builder()
                             .withActor(Player.BLACK, two)
                             .withActor(Player.WHITE, one)
                             .build());
    }

    /**
     * The main entry point for the Reversi Program.
     *
     * @param args an array having two elements: [black's strategy, white's strategy]
     */
    public static void main(final String[] args) {
        if (args == null || args.length != 2) {
            System.out.println("Argument list error: blackStrategy and whiteStrategy must be provided.");
            usage();
            System.exit(ERROR_CODE_1);
        }
        Strategy[] s = new Strategy[]{null, null};
        for (int i = 0; i < s.length; i++) {
            Object o = null;
            try {
                Class<?> c = Class.forName(args[i]);
                o = c.newInstance();
            } catch (ClassNotFoundException e) {
                System.out.println("Exception e: " + e);
                usage();
                System.exit(ERROR_CODE_2);
            } catch (InstantiationException e) {
                System.out.println("Exception e: " + e);
                usage();
                System.exit(ERROR_CODE_3);
            } catch (IllegalAccessException e) {
                System.out.println("Exception e: " + e);
                usage();
                System.exit(ERROR_CODE_4);
            }
            try {
                s[i] = (Strategy) o;
            } catch (ClassCastException e) {
                System.out.println("Exception e: " + e);
                usage();
                System.exit(ERROR_CODE_5);
            }
        }
        reversi(s[0], s[1], System.out,
                Period.minutes(DEFAULT_GAME_DURATION_IN_MINUTES).toStandardDuration());
    }

    /**
     * Print the usage message.
     */
    private static void usage() {
        System.out.println("usage: java rcrr.reversi.Reversi blackStrategy whiteStrategy");
        System.out.println("\t Where blackStrategy and whiteStrategy are two classes");
        System.out.println("\t that implements the rcrr.reversi.Strategy interface.");
    }

}
