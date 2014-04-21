--
-- test_function_definition.sql
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
-- Load the file by running the command: \i test_function_definition.sql
--
--
-- This script creates the functions used by the reversi program.
--

SET search_path TO reversi;



--
-- Tests the game_position_empties function.
--
CREATE OR REPLACE FUNCTION test_game_position_empties() RETURNS VOID AS $$
DECLARE
  fixture  RECORD;
  gp       game_position;
  computed square_set;
  expected square_set;
BEGIN
  SELECT * INTO STRICT fixture FROM game_position_test_data WHERE id = 'empty';
  gp := fixture.gp;
  computed := game_position_empties(gp);
  expected := square_set_from_string('xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx');
  PERFORM p_assert(expected = computed, 'The empty board must have 64 empties.');
END;
$$ LANGUAGE plpgsql;
