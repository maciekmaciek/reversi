/* Copyright 1986-1994 by Kai-Fu Lee, Sanjoy Mahajan, and Joe Keane.

   This file is part of Bill and is free software.  You can
   redistribute it and/or modify it under the terms of the GNU General
   Public License as published by the Free Software Foundation; either
   version 2 of the License (see the file COPYING), or (at your
   option) any later version.
 */

#include <node.h>
#include <search.h>


/* Directions from each square.  Each group is for one square.
   Each triple within a group represent:
	Starting square
	Direction
	Ending square
 */

int   Dirs[1008] =
{
  8, 8, 64,
  1, 1, 8,
  9, 9, 72,

  9, 8, 65,
  2, 1, 8,
  10, 9, 64,

  10, 8, 66,
  3, 1, 8,
  1, -1, -1,
  9, 7, 23,
  11, 9, 56,

  11, 8, 67,
  4, 1, 8,
  2, -1, -1,
  10, 7, 31,
  12, 9, 48,

  12, 8, 68,
  5, 1, 8,
  3, -1, -1,
  13, 9, 40,
  11, 7, 39,

  13, 8, 69,
  6, 1, 8,
  4, -1, -1,
  14, 9, 32,
  12, 7, 47,

  14, 8, 70,
  5, -1, -1,
  13, 7, 55,

  15, 8, 71,
  6, -1, -1,
  14, 7, 63,

  16, 8, 64,
  9, 1, 16,
  17, 9, 71,

  17, 8, 65,
  10, 1, 16,
  18, 9, 72,

  18, 8, 66,
  11, 1, 16,
  9, -1, 7,
  17, 7, 31,
  19, 9, 64,

  19, 8, 67,
  12, 1, 16,
  10, -1, 7,
  18, 7, 39,
  20, 9, 56,

  20, 8, 68,
  13, 1, 16,
  11, -1, 7,
  21, 9, 48,
  19, 7, 47,

  21, 8, 69,
  14, 1, 16,
  12, -1, 7,
  22, 9, 40,
  20, 7, 55,

  22, 8, 70,
  13, -1, 7,
  21, 7, 63,

  23, 8, 71,
  14, -1, 7,
  22, 7, 64,

  24, 8, 64,
  8, -8, -8,
  17, 1, 24,
  9, -7, -5,
  25, 9, 70,

  25, 8, 65,
  9, -8, -7,
  18, 1, 24,
  10, -7, -4,
  26, 9, 71,

  26, 8, 66,
  10, -8, -6,
  19, 1, 24,
  17, -1, 15,
  11, -7, -3,
  25, 7, 39,
  27, 9, 72,
  9, -9, -9,

  27, 8, 67,
  11, -8, -5,
  20, 1, 24,
  18, -1, 15,
  26, 7, 47,
  12, -7, -2,
  28, 9, 64,
  10, -9, -8,

  28, 8, 68,
  12, -8, -4,
  21, 1, 24,
  19, -1, 15,
  29, 9, 56,
  11, -9, -7,
  27, 7, 55,
  13, -7, -1,

  29, 8, 69,
  13, -8, -3,
  22, 1, 24,
  20, -1, 15,
  30, 9, 48,
  12, -9, -6,
  28, 7, 63,
  14, -7, 0,

  30, 8, 70,
  14, -8, -2,
  21, -1, 15,
  13, -9, -5,
  29, 7, 64,

  31, 8, 71,
  15, -8, -1,
  22, -1, 15,
  14, -9, -4,
  30, 7, 65,

  32, 8, 64,
  16, -8, -8,
  25, 1, 32,
  17, -7, -4,
  33, 9, 69,

  33, 8, 65,
  17, -8, -7,
  26, 1, 32,
  18, -7, -3,
  34, 9, 70,

  34, 8, 66,
  18, -8, -6,
  27, 1, 32,
  25, -1, 23,
  19, -7, -2,
  33, 7, 47,
  35, 9, 71,
  17, -9, -1,

  35, 8, 67,
  19, -8, -5,
  28, 1, 32,
  26, -1, 23,
  34, 7, 55,
  20, -7, -1,
  36, 9, 72,
  18, -9, -9,

  36, 8, 68,
  20, -8, -4,
  29, 1, 32,
  27, -1, 23,
  37, 9, 64,
  19, -9, -8,
  35, 7, 63,
  21, -7, 0,

  37, 8, 69,
  21, -8, -3,
  30, 1, 32,
  28, -1, 23,
  20, -9, -7,
  38, 9, 56,
  36, 7, 64,
  22, -7, 8,

  38, 8, 70,
  22, -8, -2,
  29, -1, 23,
  21, -9, -6,
  37, 7, 65,

  39, 8, 71,
  23, -8, -1,
  30, -1, 23,
  22, -9, -5,
  38, 7, 66,

  40, 8, 64,
  24, -8, -8,
  33, 1, 40,
  41, 9, 68,
  25, -7, -3,

  41, 8, 65,
  25, -8, -7,
  34, 1, 40,
  42, 9, 69,
  26, -7, -2,

  42, 8, 66,
  26, -8, -6,
  35, 1, 40,
  33, -1, 31,
  43, 9, 70,
  25, -9, 7,
  27, -7, -1,
  41, 7, 55,

  43, 8, 67,
  27, -8, -5,
  36, 1, 40,
  34, -1, 31,
  26, -9, -1,
  44, 9, 71,
  28, -7, 0,
  42, 7, 63,

  44, 8, 68,
  28, -8, -4,
  37, 1, 40,
  35, -1, 31,
  29, -7, 8,
  43, 7, 64,
  27, -9, -9,
  45, 9, 72,

  45, 8, 69,
  29, -8, -3,
  38, 1, 40,
  36, -1, 31,
  44, 7, 65,
  30, -7, 16,
  28, -9, -8,
  46, 9, 64,

  46, 8, 70,
  30, -8, -2,
  37, -1, 31,
  45, 7, 66,
  29, -9, -7,

  47, 8, 71,
  31, -8, -1,
  38, -1, 31,
  46, 7, 67,
  30, -9, -6,

  48, 8, 64,
  32, -8, -8,
  41, 1, 48,
  49, 9, 67,
  33, -7, -2,

  49, 8, 65,
  33, -8, -7,
  42, 1, 48,
  50, 9, 68,
  34, -7, -1,

  50, 8, 66,
  34, -8, -6,
  43, 1, 48,
  41, -1, 39,
  33, -9, 15,
  51, 9, 69,
  35, -7, 0,
  49, 7, 63,

  51, 8, 67,
  35, -8, -5,
  44, 1, 48,
  42, -1, 39,
  34, -9, 7,
  52, 9, 70,
  36, -7, 8,
  50, 7, 64,

  52, 8, 68,
  36, -8, -4,
  45, 1, 48,
  43, -1, 39,
  37, -7, 16,
  51, 7, 65,
  35, -9, -1,
  53, 9, 71,

  53, 8, 69,
  37, -8, -3,
  46, 1, 48,
  44, -1, 39,
  52, 7, 66,
  38, -7, 24,
  36, -9, -9,
  54, 9, 72,

  54, 8, 70,
  38, -8, -2,
  45, -1, 39,
  53, 7, 67,
  37, -9, -8,

  55, 8, 71,
  39, -8, -1,
  46, -1, 39,
  54, 7, 68,
  38, -9, -7,

  40, -8, -8,
  49, 1, 56,
  41, -7, -1,

  41, -8, -7,
  50, 1, 56,
  42, -7, 0,

  42, -8, -6,
  51, 1, 56,
  49, -1, 47,
  41, -9, 23,
  43, -7, 8,

  43, -8, -5,
  52, 1, 56,
  50, -1, 47,
  42, -9, 15,
  44, -7, 16,

  44, -8, -4,
  53, 1, 56,
  51, -1, 47,
  45, -7, 24,
  43, -9, 7,

  45, -8, -3,
  54, 1, 56,
  52, -1, 47,
  46, -7, 32,
  44, -9, -1,

  46, -8, -2,
  53, -1, 47,
  45, -9, -9,

  47, -8, -1,
  54, -1, 47,
  46, -9, -8,

  48, -8, -8,
  57, 1, 64,
  49, -7, 0,

  49, -8, -7,
  58, 1, 64,
  50, -7, 8,

  50, -8, -6,
  59, 1, 64,
  57, -1, 55,
  49, -9, 31,
  51, -7, 16,

  51, -8, -5,
  60, 1, 64,
  58, -1, 55,
  50, -9, 23,
  52, -7, 24,

  52, -8, -4,
  61, 1, 64,
  59, -1, 55,
  53, -7, 32,
  51, -9, 15,

  53, -8, -3,
  62, 1, 64,
  60, -1, 55,
  54, -7, 40,
  52, -9, 7,

  54, -8, -2,
  61, -1, 55,
  53, -9, -1,

  55, -8, -1,
  62, -1, 55,
  54, -9, -9
};


/* Index into Dirs.  (Nth number means start at that number for the
   Nth square moves)
 */

int   Dir_Start[65] =
{
  0, 9, 18, 33, 48, 63, 78, 87,
  96, 105, 114, 129, 144, 159, 174, 183,
  192, 207, 222, 246, 270, 294, 318, 333,
  348, 363, 378, 402, 426, 450, 474, 489,
  504, 519, 534, 558, 582, 606, 630, 645,
  660, 675, 690, 714, 738, 762, 786, 801,
  816, 825, 834, 849, 864, 879, 894, 903,
  912, 921, 930, 945, 960, 975, 990, 999,
  1008
};
/* Displacement table for the edge table */

int   D1[4] =
{
  0, 1, 2, 2
}    ,
      D2[4] =
{
  0, 3, 6, 6
}    ,
      D3[4] =
{
  0, 9, 18, 18
}    ,
      D4[4] =
{
  0, 27, 54, 54
}    ,
      D5[4] =
{
  0, 81, 162, 162
}    ,
      D6[4] =
{
  0, 243, 486, 486
}    ,
      D7[4] =
{
  0, 729, 1458, 1458
}    ,
      D8[4] =
{
  0, 2187, 4374, 4374
}    ,
      D9[4] =
{
  0, 6561, 13122, 13122
}    ,
      D10[4] =
{
  0, 19683, 39366, 39366
};

int   WD1[4] =
{
  1, 0, 2, 2
}    ,
      WD2[4] =
{
  3, 0, 6, 6
}    ,
      WD3[4] =
{
  9, 0, 18, 18
}    ,
      WD4[4] =
{
  27, 0, 54, 54
}    ,
      WD5[4] =
{
  81, 0, 162, 162
}    ,
      WD6[4] =
{
  243, 0, 486, 486
}    ,
      WD7[4] =
{
  729, 0, 1458, 1458
}    ,
      WD8[4] =
{
  2187, 0, 4374, 4374
}    ,
      WD9[4] =
{
  6561, 0, 13122, 13122
}    ,
      WD10[4] =
{
  19683, 0, 39366, 39366
};


/* next_st contains the displacements for Next_Arr */

int   Next_St[MAX_PIECES + 1] =
{
  0, 3, 8, 13, 18, 23, 28, 33,
  36, 41, 49, 57, 65, 73, 81, 89,
  94, 99, 107, 115, 123, 131, 139, 147,
  152, 157, 165, 173, 181, 189, 197, 205,
  210, 215, 223, 231, 239, 247, 255, 263,
  268, 273, 281, 289, 297, 305, 313, 321,
  326, 331, 339, 347, 355, 363, 371, 379,
  384, 387, 392, 397, 402, 407, 412, 417,
  420,
};

/* List of neighbors for each square */

int   Next_Arr[NEXT_MAX] =
{
  8, 9, 1,
  9, 10, 8, 0, 2,
  10, 11, 9, 1, 3,
  11, 12, 10, 2, 4,
  12, 13, 11, 3, 5,
  13, 14, 12, 4, 6,
  14, 15, 13, 5, 7,
  15, 14, 6,

  0, 16, 1, 17, 9,
  1, 17, 0, 2, 18, 16, 8, 10,
  2, 18, 1, 3, 19, 17, 9, 11,
  3, 19, 2, 4, 20, 18, 10, 12,
  4, 20, 3, 5, 21, 19, 11, 13,
  5, 21, 4, 6, 22, 20, 12, 14,
  6, 22, 5, 7, 23, 21, 13, 15,
  7, 23, 6, 22, 14,

  8, 24, 9, 25, 17,
  9, 25, 8, 10, 26, 24, 16, 18,
  10, 26, 9, 11, 27, 25, 17, 19,
  11, 27, 10, 12, 28, 26, 18, 20,
  12, 28, 11, 13, 29, 27, 19, 21,
  13, 29, 12, 14, 30, 28, 20, 22,
  14, 30, 13, 15, 31, 29, 21, 23,
  15, 31, 14, 30, 22,

  16, 32, 17, 33, 25,
  17, 33, 16, 18, 34, 32, 24, 26,
  18, 34, 17, 19, 35, 33, 25, 27,
  19, 35, 18, 20, 36, 34, 26, 28,
  20, 36, 19, 21, 37, 35, 27, 29,
  21, 37, 20, 22, 38, 36, 28, 30,
  22, 38, 21, 23, 39, 37, 29, 31,
  23, 39, 22, 38, 30,

  24, 40, 25, 41, 33,
  25, 41, 24, 26, 42, 40, 32, 34,
  26, 42, 25, 27, 43, 41, 33, 35,
  27, 43, 26, 28, 44, 42, 34, 36,
  28, 44, 27, 29, 45, 43, 35, 37,
  29, 45, 28, 30, 46, 44, 36, 38,
  30, 46, 29, 31, 47, 45, 37, 39,
  31, 47, 30, 46, 38,

  32, 48, 33, 49, 41,
  33, 49, 32, 34, 50, 48, 40, 42,
  34, 50, 33, 35, 51, 49, 41, 43,
  35, 51, 34, 36, 52, 50, 42, 44,
  36, 52, 35, 37, 53, 51, 43, 45,
  37, 53, 36, 38, 54, 52, 44, 46,
  38, 54, 37, 39, 55, 53, 45, 47,
  39, 55, 38, 54, 46,

  40, 56, 41, 57, 49,
  41, 57, 40, 42, 58, 56, 48, 50,
  42, 58, 41, 43, 59, 57, 49, 51,
  43, 59, 42, 44, 60, 58, 50, 52,
  44, 60, 43, 45, 61, 59, 51, 53,
  45, 61, 44, 46, 62, 60, 52, 54,
  46, 62, 45, 47, 63, 61, 53, 55,
  47, 63, 46, 62, 54,

  48, 49, 57,
  49, 48, 50, 56, 58,
  50, 49, 51, 57, 59,
  51, 50, 52, 58, 60,
  52, 51, 53, 59, 61,
  53, 52, 54, 60, 62,
  54, 53, 55, 61, 63,
  55, 54, 62,
};


/* Random 31 bit positive numbers used for hashing.  Second 64 numbers are for Black in each position.  Last 64 numbers are for White in each position. */

int   Random[2][MAX_PIECES] =
{
  {
    1562284138, 1158430050, 34927307, 792404178, 881994086, 1333136605, 961766950, 1901014988,
    1627532446, 647263449, 152063403, 1627005512, 1424172407, 815006462, 810282487, 1192434389,
    692529779, 646390930, 412226454, 1189819632, 21717234, 1050856969, 1688740215, 819520889,
    1025791845, 1708966931, 562252446, 330900074, 1581307336, 2061858504, 774674672, 660483803,
    478177749, 804919668, 385350155, 1842811160, 680877596, 1215586749, 1276715185, 2054631710,
    1111686003, 1403523588, 1320413263, 987501569, 600140207, 558950001, 1204314856, 1669572854,
    1518007235, 1014036145, 324087291, 353562421, 424698588, 526836005, 1551321441, 1914213887,
    253493927, 136994432, 158171522, 44893033, 1526420726, 1338910055, 658577024, 2140560646,
  },
  {
    1791002422, 1175967575, 965758773, 834580511, 894684027, 1986713019, 175961372, 1107311135,
    1476489818, 1058926477, 599915188, 1657557970, 1767372385, 2097147828, 1736827075, 1668479305,
    999207159, 506739432, 1722126447, 1244634058, 685896980, 157412742, 1597929033, 1621014342,
    2106843772, 896366691, 929639997, 1315232297, 913638585, 973375265, 1809736424, 1096047571,
    1567339645, 1580091020, 1013615973, 2083090676, 950572217, 2086207157, 845952624, 1249167848,
    2043615377, 1536283003, 1970337557, 1193417015, 2002025055, 1419333399, 1090440600, 1895951953,
    392241599, 256620935, 969654658, 1958662856, 278290022, 1179568861, 951446927, 806843004,
    2135491239, 549792187, 671724199, 1990076384, 272525149, 380852928, 1511350362, 1910640276,
  },
};

/* These are the XOR's of random numbers that represent Black and White on the same position. */

int   Random_Xor[MAX_PIECES] =
{
  937301852, 52161077, 998555134, 512069837, 29483549, 958191974, 858755898, 860820947,
  956522180, 428448596, 718386975, 37122458, 1035260694, 1282141514, 1473121588, 610395036,
  315202180, 951262842, 2117567481, 214216506, 699045350, 935535503, 999708478, 1346785855,
  1085781273, 1353825392, 384459939, 1574828099, 1748357489, 1088549353, 1173551640, 1711961352,
  1105976744, 1909743608, 714962798, 301883884, 272485029, 875339528, 2121602753, 805566710,
  999154658, 138003327, 1002643290, 2113801526, 1418793968, 1976150374, 121024368, 310527655,
  1293668988, 859524918, 714796153, 1638678013, 163986618, 1495804920, 1690477806, 1108338563,
  1884618752, 686782267, 560214821, 1949508745, 1254437803, 1501396903, 2102687450, 242504594,
};


/* Type1_Time[n] is the estimate of how much time must be left on the clock for a type1 engame search with n empty squres to be feasible.  It gets modified in compute_nps () in utils.c accoring to approximate nodes we can search / second.  These numbers assume about 1000 nps. */

int   Type1_Time[MAX_PIECES] =
{
  0,	    1000,     1000,	1000,	  1000,	    1000,     1000,	1000,
  1000,	    1200,     1500,	2500,	  4500,	    8500,     50000,	110000,
  250000,   650000,   2000000,	5000000,  15000000, INFINITY, INFINITY,	INFINITY,
  INFINITY, INFINITY, INFINITY,	INFINITY, INFINITY, INFINITY, INFINITY,	INFINITY,
  INFINITY, INFINITY, INFINITY,	INFINITY, INFINITY, INFINITY, INFINITY,	INFINITY,
  INFINITY, INFINITY, INFINITY,	INFINITY, INFINITY, INFINITY, INFINITY,	INFINITY,
  INFINITY, INFINITY, INFINITY,	INFINITY, INFINITY, INFINITY, INFINITY,	INFINITY,
  INFINITY, INFINITY, INFINITY,	INFINITY, INFINITY, INFINITY, INFINITY,	INFINITY,
};

/* Type2_Time[n] is the estimate of how much time must be left on the clock for a type2 engame search with n empty squres to be feasible. */

int   Type2_Time[MAX_PIECES] =
{
  0,        1000,     1000,     1000,     1000,     1000,     1000,     1000,
  1200,     1500,     3000,     6500,     22500,    60000,    180000,   360000,
  800000,   2000000,  5000000,  15000000, INFINITY, INFINITY, INFINITY, INFINITY,
  INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY,
  INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY,
  INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY,
  INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY,
  INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY,
};


/* Time_Raw_Fraction contains what percentage of time can be spent on a move.  This is converted to Time_Fraction when we know how far the opening book got. */

double  Time_Raw_Fraction[MAX_MOVES / 2] =
{   0.001, 0.001, 0.024,  0.025, 0.026, 0.027, 0.028, 0.029, 0.030, 0.032,
    0.034, 0.036, 0.038,  0.04,  0.04,  0.041, 0.041, 0.042, 0.042, 0.043,
    0.043, 0.042, 0.042,  0.04,  0.02,  0.02,  0.125, 0.01,  0.008, 0.005,
    0.005, 0.0045,0.004,  0.003, 0.0025,0.002, 0.001, 0.001, 0.001, 0.001
};

/*  0.001, 0.001, 0.01, 0.02, 0.022, 0.024, 0.026, 0.028, 0.03, 0.032,
  0.034, 0.036, 0.038, 0.04, 0.0415, 0.043, 0.0445, 0.046, 0.0475, 0.049,
  0.0505, 0.052, 0.0535, 0.075, 0.075, 0.035, 0.007, 0.006, 0.0055, 0.005,
  0.004, 0.004, 0.004, 0.002, 0.002, 0.002, 0.001, 0.001, 0.001, 0.001
*/


/* some printing names */

char  Color_Name[3][10] =
{
  "BLACK", "WHITE", "EMPTY",
};

char  Print_Name[4] =
{
  '*', 'o', '.', '.',
};


int X_Square[MAX_PIECES] =
{
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 0, 0, 0, 0, 2, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 3, 0, 0, 0, 0, 4, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
};

int C_Square[MAX_PIECES] =
{
  0, 1, 0, 0, 0, 0, 2, 0,
  3, 0, 0, 0, 0, 0, 0, 4,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  5, 0, 0, 0, 0, 0, 0, 6,
  0, 7, 0, 0, 0, 0, 8, 0,
};

int C_Corner[9] =
{
  0, 0, 7, 0, 7, 56, 63, 56, 63,
};

int C_B[9] =
{
  0, 2, 5, 16, 23, 40, 47, 58, 61,
};

int C_A[9] =
{
  0, 3, 4, 24, 31, 32, 39, 59, 60,
};

int Corner[5] =
{
  0, 0, 7, 56, 63,
},
  C1[5] =
{
  0, 1, 6, 48, 55,
},
  C2[5] =
{
  0, 8, 15, 57, 62,
};

int   Edge_Turn_Weight = 500;

int Disp[2][12][4] =
{
  /* for Black */
  {
    /* displacements */
    { 0, 1, 2, 2, },
    { 0, 3, 6, 6, },
    { 0, 9, 18, 18, },
    { 0, 27, 54, 54, },
    { 0, 81, 162, 162, },
    { 0, 243, 486, 486, },
    { 0, 729, 1458, 1458, },
    { 0, 2187, 4374, 4374, },
    { 0, 6561, 13122, 13122, },
    { 0, 19683, 39366, 39366, },
    /* occupied */
    { 1, 1, 0, 0, },
    { 2, 2, 0, 0, },
  },
  /* for White */
  {
    { 1, 0, 2, 2, },
    { 3, 0, 6, 6, },
    { 9, 0, 18, 18, },
    { 27, 0, 54, 54, },
    { 81, 0, 162, 162, },
    { 243, 0, 486, 486, },
    { 729, 0, 1458, 1458, },
    { 2187, 0, 4374, 4374, },
    { 6561, 0, 13122, 13122, },
    { 19683, 0, 39366, 39366, },
    { 1, 1, 0, 0, },
    { 2, 2, 0, 0, },
  },
};