/* Copyright 1986-1994 by Kai-Fu Lee, Sanjoy Mahajan, and Joe Keane.

   This file is part of Bill and is free software.  You can
   redistribute it and/or modify it under the terms of the GNU General
   Public License as published by the Free Software Foundation; either
   version 2 of the License (see the file COPYING), or (at your
   option) any later version.
 */

/* these are the numbers used by gen_values to generate the value array.
   It's use is:
   	x = Value[i][j][k] means  that
		if you are considering table i (0-8), and square type j (0-5),
		and a disc square k , it is worth x.
*/

#include <tables.h>

int             Value[NUM_TABLES][NUM_TYPES][MAX_PIECES] =
{
 {						/* diag3 */
  {   0, -200,    0},					/* unstable */
  {-120, -200, -120},					/* Semi_stable */
  {   0,    0,    0},					/* two-way center */
  {   0,    0,    0},					/* one-way center */
  {   0,   50,    0},					/* no-way center */
  {   0,    0,    0},					/* stable  */
  },
 {						/* diag4 */
  { -50,  -50,  -50,  -50},				/* unstable */
  {-100, -100, -100, -100},				/* Semi_stable */
  {   0,    0,    0,    0},				/* two-way center */
  {   0,   50,   50,    0},				/* one-way center */
  {   0,   70,   70,    0},				/* no-way center */
  {   0,  100,  100,    0},				/* stable  */
  },
 {						/* diag5 */
  { -60,  -80,  -80,  -80,  -60},			/* unstable */
  {-120, -160, -160, -160, -120},			/* Semi_stable */
  {   0,    0,  250,    0,    0},			/* two-way center */
  {   0,   50,  100,   50,    0},			/* one-way center */
  {   0,  100,  200,  100,    0},			/* no-way center */
  {   0,   70,  250,   70,    0},			/* stable  */
  },
 {						/* diag6 */
  { -80, -120, -100, -100, -120,  -80},			/* unstable */
  {-160, -240, -100, -100, -240, -160},			/* unstable */
  {   0,    0,  500,  500,    0,    0},			/* two-way center */
  {   0,   50,  250,  250,   50,    0},			/* one-way center */
  {   0,  140,  400,  400,  140,    0},			/* no-way center */
  {   0,  100,  550,  550,  100,    0},			/* stable  */
  },
 {						/* diag7 */
  {   0, -150,  -50,   50,   -50, -150,    0},		/* unstable */
  {   0, -300, -100,  100,  -100, -300,    0},		/* Semi_stable */
  {   0,    0,  350,  500,  350,    0,    0},		/* two-way center */
  {   0,   30,  150,  250,  150,   30,    0},		/* one-way center */
  {   0,  100,  250,  350,  250,  100,    0},		/* no-way center */
  {   0,   50,  250,  500,  250,   50,    0},		/* stable  */
  },
 {						/* diag8 */
  {   0, -600, -200, -300, -300, -200, -600,    0},	/* unstable */
  {   0,    0,  400,  600,  600,  400,    0,    0},	/* Semi_stable */
  {   0,    0,  400,  700,  700,  400,    0,    0},	/* two-way center */
  {   0,  100,  100,  300,  300,  100,  100,    0},	/* one-way center */
  {   0,  200,  400,  700,  700,  400,  200,    0},	/* no-way center */
  {   0,  400,  400, 1000, 1000,  400,  400,    0},	/* stable */
  },
 {						/* tb1 */
  {-100, -200, -120, -150, -150, -120, -200, -100},	/* unstable */
  {-200, -400, -240, -300, -300, -240, -400, -200},	/* Semi_stable */
  {   0,  100,  200,  350,  350,  200,  100,    0},	/* two-way center */
  {   0,   50,  120,  160,  160,  120,   50,    0},	/* one-way center */
  {   0,  140,  160,  220,  220,  160,  140,    0},	/* no-way center */
  {   0,   50,  100,  300,  300,  100,   50,    0},	/* stable  */
  },
 {						/* tb2 */
  {-200, -150, -120, -150, -150, -120, -150, -200},	/* unstable */
  {-400, -300, -200,  200,  200, -200, -300, -400},	/* Semi_stable */
  {   0,    0,  400,  600,  600,  400,    0,    0},	/* two-way center */
  {   0,  150,  300,  400,  400,  300,  150,    0},	/* one-way center */
  {   0,  200,  300,  400,  400,  300,  200,    0},	/* no-way center */
  {   0,   50,  250,  600,  600,  250,   50,    0},	/* stable */
  },
 {						/* tb3 */
  {-200, -150, -120, -100, -100, -120, -150, -200},	/* unstable */
  {-450, -350, -200,  300,  300, -200, -350, -450},	/* Semi_stable */
  {   0,    0,  500,  700,  700,  500,    0,    0},	/* two-way center */
  {   0,  150,  250,  350,  350,  250,  150,    0},	/* one-way center */
  {   0,  300,  420,  540,  540,  420,  300,    0},	/* no-way center */
  {   0,  200,  400,  900,  900,  400,  200,    0},	/* stable */
  },
};

/* See my letter for how these are used. */

double weights[NUM_TABLES][NUM_TYPES][MAX_PIECES+1] =
{
{ /* diag3 */
    {0, 1.0, 2.2, 2.5, 2.7, 3.4, 4.0, 4.5, 5.0 },		/* unstable */
    {0, 1.0, 2.0, 2.2, 2.5, 3.0, 3.5, 3.7, 4.0 },		/* semi-stable */
    {0, 1.0, 2.0, 1.8, 1.5, 1.2, 1.0, 0.8, 0.4 },		/* two_way_center */
    {0, 1.0, 2.0, 1.8, 1.5, 1.2, 1.0, 0.8, 0.4 },		/* one_way_center */
    {0, 1.0, 2.0, 1.8, 1.5, 1.2, 1.0, 0.8, 0.4 },		/* no_way_center */
    {0, 1.0, 2.2, 2.5, 2.0, 1.3, 1.2, 1.1, 1.0 },		/* stable */
},
{ /* diag4 */
    {0, 1.0, 2.2, 2.5, 2.7, 3.4, 4.0, 4.5, 5.0 },		/* unstable */
    {0, 1.0, 2.0, 2.2, 2.5, 3.0, 3.5, 3.7, 4.0 },		/* semi-stable */
    {0, 1.0, 2.0, 1.8, 1.5, 1.2, 1.0, 0.8, 0.4 },		/* two_way_center */
    {0, 1.0, 2.0, 1.8, 1.5, 1.2, 1.0, 0.8, 0.4 },		/* one_way_center */
    {0, 1.0, 2.0, 1.8, 1.5, 1.2, 1.0, 0.8, 0.4 },		/* no_way_center */
    {0, 1.0, 2.2, 2.5, 2.0, 1.3, 1.2, 1.1, 1.0 },		/* stable */
},
{ /* diag5 */
    {0, 1.0, 2.2, 2.5, 2.7, 3.4, 4.0, 4.5, 5.0 },		/* unstable */
    {0, 1.0, 2.0, 2.2, 2.5, 3.0, 3.5, 3.7, 4.0 },		/* semi-stable */
    {0, 1.0, 1.7, 2.1, 2.5, 2.8, 3.0, 0.8, 0.4 },		/* two_way_center */
    {0, 1.0, 1.7, 2.1, 2.5, 1.2, 1.0, 0.8, 0.4 },		/* one_way_center */
    {0, 1.0, 1.7, 2.1, 2.5, 1.2, 1.0, 0.8, 0.4 },		/* no_way_center */
    {0, 1.0, 2.2, 2.5, 2.0, 1.3, 1.2, 1.1, 1.0 },		/* stable */
},
{ /* diag6 */
    {0, 1.0, 2.2, 2.5, 2.7, 3.4, 4.0, 4.5, 5.0 },		/* unstable */
    {0, 1.0, 2.0, 2.2, 2.5, 3.0, 3.5, 3.7, 4.0 },		/* semi-stable */
    {0, 1.0, 2.0, 2.5, 3.0, 2.8, 3.0, 0.8, 0.4 },		/* two_way_center */
    {0, 1.0, 2.0, 2.5, 3.0, 1.2, 1.0, 0.8, 0.4 },		/* one_way_center */
    {0, 1.0, 2.0, 2.5, 3.0, 1.2, 1.0, 0.8, 0.4 },		/* no_way_center */
    {0, 1.0, 2.2, 2.5, 2.0, 1.3, 1.2, 1.1, 1.0 },		/* stable */
},
{ /* diag7 */
    {0, 1.0, 2.2, 2.5, 2.7, 3.4, 4.0, 4.5, 5.0 },		/* unstable */
    {0, 1.0, 2.0, 2.2, 2.5, 3.0, 3.5, 3.7, 4.0 },		/* semi-stable */
    {0, 1.0, 2.0, 2.7, 3.2, 2.8, 3.0, 0.8, 0.4 },		/* two_way_center */
    {0, 1.0, 2.0, 2.7, 3.2, 1.2, 1.0, 0.8, 0.4 },		/* one_way_center */
    {0, 1.0, 2.0, 2.7, 3.2, 1.2, 1.0, 0.8, 0.4 },		/* no_way_center */
    {0, 1.0, 2.2, 2.5, 2.0, 1.3, 1.2, 1.1, 1.0 },		/* stable */
},
{ /* diag8 */
    {0, 1.0, 2.2, 2.5, 2.7, 3.4, 4.0, 4.5, 5.0 },		/* unstable */
    {0, 1.0, 2.0, 2.2, 2.5, 3.0, 3.5, 3.7, 4.0 },		/* semi-stable */
    {0, 1.0, 2.0, 3.0, 3.5, 1.2, 1.0, 0.8, 0.4 },		/* two_way_center */
    {0, 1.0, 2.0, 3.0, 3.5, 1.2, 1.0, 0.8, 0.4 },		/* one_way_center */
    {0, 1.0, 2.0, 3.0, 3.5, 1.2, 1.0, 0.8, 0.4 },		/* no_way_center */
    {0, 1.0, 2.0, 3.0, 4.0, 5.0, 5.0, 6.0, 6.0 },		/* stable */
},
{ /* tb1 */
    {0, 1.0, 2.2, 2.5, 2.7, 3.4, 4.0, 4.5, 5.0 },		/* unstable */
    {0, 1.0, 2.0, 2.2, 2.5, 3.0, 3.5, 3.7, 4.0 },		/* semi-stable */
    {0, 1.0, 2.0, 2.5, 2.0, 1.2, 1.0, 0.8, 0.4 },		/* two_way_center */
    {0, 1.0, 2.0, 2.5, 2.0, 1.2, 1.0, 0.8, 0.4 },		/* one_way_center */
    {0, 1.0, 2.0, 2.5, 2.0, 1.2, 1.0, 0.8, 0.4 },		/* no_way_center */
    {0, 1.0, 2.2, 2.5, 2.0, 1.3, 1.2, 1.1, 1.0 },		/* stable */
},
{ /* tb2 */
    {0, 1.0, 2.2, 2.5, 2.7, 3.4, 4.0, 4.5, 5.0 },		/* unstable */
    {0, 1.0, 2.0, 2.2, 2.5, 3.0, 3.5, 3.7, 4.0 },		/* semi-stable */
    {0, 1.0, 2.0, 2.2, 2.5, 1.2, 1.0, 0.8, 0.4 },		/* two_way_center */
    {0, 1.0, 2.0, 2.2, 2.5, 1.2, 1.0, 0.8, 0.4 },		/* one_way_center */
    {0, 1.0, 2.0, 2.2, 2.5, 1.2, 1.0, 0.8, 0.4 },		/* no_way_center */
    {0, 1.0, 2.2, 2.5, 2.0, 1.3, 1.2, 1.1, 1.0 },		/* stable */
},
{ /* tb3 */
    {0, 1.0, 2.2, 2.5, 2.7, 3.4, 4.0, 4.5, 5.0 },		/* unstable */
    {0, 1.0, 2.0, 2.2, 2.5, 3.0, 3.5, 3.7, 4.0 },		/* semi-stable */
    {0, 1.0, 2.0, 2.5, 3.2, 1.2, 1.0, 0.8, 0.4 },		/* two_way_center */
    {0, 1.0, 2.0, 2.5, 3.2, 1.2, 1.0, 0.8, 0.4 },		/* one_way_center */
    {0, 1.0, 2.0, 2.5, 3.2, 1.2, 1.0, 0.8, 0.4 },		/* no_way_center */
    {0, 1.0, 2.2, 2.5, 2.0, 1.6, 1.5, 1.4, 1.3 },		/* stable */
},
};
