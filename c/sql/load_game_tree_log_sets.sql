--
-- load_game_tree_log_sets.sql
--
-- This file is part of the reversi program
-- http://github.com/rcrr/reversi
--
-- Author: Roberto Corradini mailto:rob_corradini@yahoo.it
-- Copyright 2014 Roberto Corradini. All rights reserved.
--
--
-- License:
--
-- This program is free software; you can redistribute it and/or modify it
-- under the terms of the GNU General Public License as published by the
-- Free Software Foundation; either version 3, or (at your option) any
-- later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program; if not, write to the Free Software
-- Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
-- or visit the site <http://www.gnu.org/licenses/>.
--
--
-- This script has been tested with PostgreSQL.
-- Start psql by running: psql -U reversi -w -d reversi -h localhost
-- Load the file by running the command: \i load_rab_solver.sql
--
--
-- This script loads game tree sets into the db, game_tree_log_header, and game_tree_log tables.
--

SET search_path TO reversi;

--
-- File ../out/rab_solver_log-ffo-01-simplified-4_n3.csv is obtained by running
-- the commands:
-- ./build/bin/endgame_solver -f db/gpdb-sample-games.txt -q ffo-01-simplified-4 -s rab -l -n 3
-- mv out/rab_solver_log.csv out/rab_solver_log-ffo-01-simplified-4_n3.csv
--
\! ./gt_load_file.sh ../out/rab_solver_log-ffo-01-simplified-4_n3.csv;
SELECT gt_load_from_staging('T000', 'C_RAB_SOLVER', 'Test data obtained by the C rab solver on position ffo-01-simplified-4.');
VACUUM(FULL, ANALYZE) game_tree_log;
SELECT gt_check_rab('T000', 0);
SELECT gt_check_rab('T000', 1);
SELECT gt_check_rab('T000', 2);



-- Compares MINIMAX tree expansion.

--
-- File ../out/minimax_log-ffo-01-simplified-4.csv is obtained by running
-- the commands:
-- ./build/bin/endgame_solver -f db/gpdb-sample-games.txt -q ffo-01-simplified-4 -s minimax -l
-- mv out/minimax_log.csv out/minimax_log-ffo-01-simplified-4.csv
--
\! ./gt_load_file.sh ../out/minimax_log-ffo-01-simplified-4.csv;
SELECT gt_load_from_staging('T001','C_MINIMAX_SOLVER', 'Test data obtained by the C minimax solver on position ffo-01-simplified-4.');
VACUUM(FULL, ANALYZE) game_tree_log;
SELECT p_assert(gt_check('T001', 0) = (20040, 6879, 6879, 8875, 0, 13161), 'The game tree T001/0 has not been loaded as expected.');

--
-- Loads into the db the game tree generated by SQL_MINIMAX_SOLVER on position ffo-01-simplified-4.
--
SELECT
  id,
  description,
  game_position_pp(gp),
  game_position_solve(gp, 'SQL_MINIMAX_SOLVER', TRUE, 'T002', 'Test data obtained by the PL/SQL minimax solver on position ffo-01-simplified-4.')
FROM
  game_position_test_data WHERE id = 'ffo-01-simplified-4';
VACUUM(FULL, ANALYZE) game_tree_log;
SELECT p_assert(gt_check('T002', 0) = (20040, 6879, 6879, 8875, 0, 13161), 'The game tree T002/0 has not been loaded as expected.');

--
-- Tests if the tree expansion performed by the PLSQL and C minimax implementations are equal.
--
DO $$
DECLARE
  gt_compare_result_record RECORD;
BEGIN
  gt_compare_result_record := gt_compare('T001', 'T002');
  PERFORM p_assert(gt_compare_result_record.are_equal = TRUE, 'Game tree set T001 and T002 must be equal.');
END $$;



-- Compares ALPHA-BETA tree expansion.

--
-- File ../out/ab_log-ffo-01-simplified-4.csv is obtained by running
-- the commands:
-- ./build/bin/endgame_solver -f db/gpdb-sample-games.txt -q ffo-01-simplified-4 -s ab -l
-- mv out/ab_solver_log.csv out/ab_solver_log-ffo-01-simplified-4.csv
--
\! ./gt_load_file.sh ../out/ab_solver_log-ffo-01-simplified-4.csv;
SELECT gt_load_from_staging('T003','C_ALPHABETA_SOLVER', 'Test data obtained by the C alpha-beta solver on position ffo-01-simplified-4.');
VACUUM(FULL, ANALYZE) game_tree_log;
SELECT p_assert(gt_check('T003', 0) = (3367, 1829, 1829, 2128, 0, 1538), 'The game tree T003/0 has not been loaded as expected.');

--
-- Loads into the db the game tree generated by SQL_ALPHABETA_SOLVER on position ffo-01-simplified-4.
--
SELECT
  id,
  description,
  game_position_pp(gp),
  game_position_solve(gp, 'SQL_ALPHABETA_SOLVER', TRUE, 'T004', 'Test data obtained by the PL/SQL alpha-beta solver on position ffo-01-simplified-4.')
FROM
  game_position_test_data WHERE id = 'ffo-01-simplified-4';
VACUUM(FULL, ANALYZE) game_tree_log;
SELECT p_assert(gt_check('T004', 0) = (3367, 1829, 1829, 2128, 0, 1538), 'The game tree T004/0 has not been loaded as expected.');

--
-- Tests if the tree expansion performed by the PLSQL and C alpha-beta implementations are equal.
--
DO $$
DECLARE
  gt_compare_result_record RECORD;
BEGIN
  gt_compare_result_record := gt_compare('T003', 'T004');
  PERFORM p_assert(gt_compare_result_record.are_equal = TRUE, 'Game tree set T003 and T004 must be equal.');
END $$;
