/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1994 Anthony Dekker
 *
 * The X Consortium, and any party obtaining a copy of these files from
 * the X Consortium, directly or indirectly, is granted, free of charge, a
 * full and unrestricted irrevocable, world-wide, paid up, royalty-free,
 * nonexclusive right and license to deal in this software and
 * documentation files (the "Software"), including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons who receive
 * copies from any such party to do so, with the only requirement being
 * that this copyright notice remain intact.  This license includes without
 * limitation a license to do the foregoing actions under any patents of
 * the party supplying this software to the X Consortium.
 */

/*
 * Neural-Net quantization algorithm based on work of Anthony Dekker
 */

/*
 *  color.h - header for routines using pixel color values.
 *
 *     12/31/85
 *
 *  Two color representations are used, one for calculation and
 *  another for storage.  Calculation is done with three floats
 *  for speed.  Stored color values use 4 bytes which contain
 *  three single byte mantissas and a common exponent.
 */

#define  N_RED		0
#define  N_GRN		1
#define  N_BLU		2

typedef unsigned char  BYTE;	/* 8-bit unsigned integer */
typedef BYTE  COLR[4];		/* red, green, blue, exponent */

#define MIN_NEU_SAMPLES	600	/* min number of samples (npixels/samplefac) needed for network */

/*
 *  random.h - header file for random(3) and urand() function.
 *
 *     10/1/85
 */

#ifdef	BSD
extern long  random();

#define	 frandom()	(random()*(1./2147483648.))

#else /* not BSD */

extern long  lrand48();
extern double  drand48();
#define	 random()	lrand48()
#define	 frandom()	drand48()

#endif	/* not BSD */
