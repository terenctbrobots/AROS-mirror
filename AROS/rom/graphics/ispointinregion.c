/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Graphics function AndRegionRegion()
    Lang: english
*/
#include "graphics_intern.h"
#include <graphics/regions.h>
#include "intregions.h"

/*****************************************************************************

    NAME */
#include <proto/graphics.h>

	AROS_LH3(BOOL, IsPointInRegion,

/*  SYNOPSIS */
	AROS_LHA(struct Region *, Reg, A0),
	AROS_LHA(WORD,            x,   D0),
	AROS_LHA(WORD,            y,   D1),

/*  LOCATION */
	struct GfxBase *, GfxBase, 190, Graphics)

/*  FUNCTION
	Checks if the point (x, y) is contained in the region Reg

    INPUTS
	region1 - pointer to a region structure
	x       - The point's 'x' coord
        y       - The point's 'y' coord

    RESULT
	TRUE if the point is contained, FALSE otherwise

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
	XorRegionRegion(), OrRegionRegion()

    INTERNALS

    HISTORY

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    struct RegionRectangle *rr;

    if (!_IsPointInRect(Bounds(Reg), x, y))
	return FALSE;

    for
    (
        rr = Reg->RegionRectangle;
	rr;
        rr = rr->Next
    )
    {
        if (y > MaxY(rr)) continue;
        if (y < MinY(rr)) return FALSE;
        if (x < MinX(rr)) continue;
        if (x > MaxX(rr)) continue;

        return TRUE;
    }

    return FALSE;

    AROS_LIBFUNC_EXIT
}
