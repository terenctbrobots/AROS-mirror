/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$

    Desc: ANSI C function abs()
    Lang: english
*/

/*****************************************************************************

    NAME */
#include <stdlib.h>

	int abs (

/*  SYNOPSIS */
	int j)

/*  FUNCTION
	Compute the absolute value of j.

    INPUTS
	j - A signed integer

    RESULT
	The absolute value of j.

    NOTES

    EXAMPLE
	// returns 1
	abs (1);

	// returns 1
	abs (-1);

    BUGS

    SEE ALSO
	labs(), fabs()

    INTERNALS

    HISTORY
	12.12.1996 digulla created

******************************************************************************/
{
    return (j < 0) ? -j : j;
} /* abs */

