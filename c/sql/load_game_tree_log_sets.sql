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

--
-- File ../out/minimax_log-ffo-01-simplified-4.csv is obtained by running
-- the commands:
-- ./build/bin/endgame_solver -f db/gpdb-sample-games.txt -q ffo-01-simplified-4 -s minimax -l
-- mv out/minimax_log.csv out/minimax_log-ffo-01-simplified-4.csv
--
\! ./gt_load_file.sh ../out/minimax_log-ffo-01-simplified-4.csv;
SELECT gt_load_from_staging('T001','C_MINIMAX_SOLVER', 'Test data obtained by the C minimax solver on position ffo-01-simplified-4.');

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

--
-- Tests if the tree expansion performed by the PLSQL and C implementation are equal.
--
DO $$
DECLARE
  c_run_id              INTEGER;
  plsql_run_id          INTEGER;
  c_run_log_count       INTEGER;
  plsql_run_log_count   INTEGER;
  full_outer_join_count INTEGER;
BEGIN
  SELECT run_id INTO STRICT c_run_id     FROM game_tree_log_header WHERE run_label = 'T001';
  SELECT run_id INTO STRICT plsql_run_id FROM game_tree_log_header WHERE run_label = 'T002';
  SELECT COUNT(*) INTO STRICT c_run_log_count     FROM game_tree_log WHERE run_id = c_run_id;
  SELECT COUNT(*) INTO STRICT plsql_run_log_count FROM game_tree_log WHERE run_id = plsql_run_id;
  PERFORM p_assert(c_run_log_count = plsql_run_log_count, 'Values of c_run_log_count and plsql_run_log_count must be equal.');

  --
  -- The query prepares two temporary tables c_minimax_gtl and plsql_minimax_gtl.
  -- Then performs a full outer join matching for equality all the game tree fields.
  -- The record count is then saved to the variable full_outer_join_count.
  -- This value is equal to the two specific count when the records are all equal one by one.
  --
  WITH c_minimax_gtl AS (
    SELECT
      gtl.run_id            AS run_id,
      gtl.sub_run_id        AS sub_run_id,
      gtl.call_id           AS call_id,
      gtl.hash              AS hash,
      gtl.parent_hash       AS parent_hash,
      gtl.blacks            AS blacks,
      gtl.whites            AS whites,
      gtl.player            AS player
    FROM
      game_tree_log AS gtl
    WHERE
      run_id = c_run_id
    ORDER BY
      sub_run_id ASC, call_id ASC
  ), plsql_minimax_gtl AS (
    SELECT
      gtl.run_id            AS run_id,
      gtl.sub_run_id        AS sub_run_id,
      gtl.call_id           AS call_id,
      gtl.hash              AS hash,
      gtl.parent_hash       AS parent_hash,
      gtl.blacks            AS blacks,
      gtl.whites            AS whites,
      gtl.player            AS player
    FROM
      game_tree_log AS gtl
    WHERE
      run_id = plsql_run_id
    ORDER BY
      sub_run_id ASC, call_id ASC
  )
  SELECT
    COUNT(*)
  INTO STRICT full_outer_join_count
  FROM
    c_minimax_gtl AS ct FULL OUTER JOIN plsql_minimax_gtl AS pt
  ON (ct.sub_run_id  = pt.sub_run_id  AND
      ct.call_id     = pt.call_id     AND
      ct.hash        = pt.hash        AND
      ct.parent_hash = pt.parent_hash AND
      ct.blacks      = pt.blacks      AND
      ct.whites      = pt.whites      AND
      ct.player      = pt.player)
  ;
  PERFORM p_assert(c_run_log_count = full_outer_join_count, 'All the records belonging to T001 and T002 must be equal.');
END $$;


