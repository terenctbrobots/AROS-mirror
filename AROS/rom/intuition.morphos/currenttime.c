/*
    (C) 1995-98 AROS - The Amiga Research OS
    $Id$
 
    Desc: Get the current time
    Lang: english
*/
#include <devices/timer.h>
#include <proto/timer.h>
#include "intuition_intern.h"


/*****************************************************************************
 
    NAME */
#include <proto/intuition.h>

AROS_LH2(void, CurrentTime,

         /*  SYNOPSIS */
         AROS_LHA(ULONG *, seconds, A0),
         AROS_LHA(ULONG *, micros, A1),

         /*  LOCATION */
         struct IntuitionBase *, IntuitionBase, 14, Intuition)

/*  FUNCTION
    Copies the current time into the argument pointers.
 
    INPUTS
    seconds - ptr to ULONG varaible which will contain the current
        seconds after function call
    micros - ptr to ULONG varaible which will contain the current
        microseconds after function call
 
    RESULT
    Copies the time values to the memory the arguments point to
    Return value is not set.
 
    NOTES
    Makes use of timer.library/timer.device
 
    EXAMPLE
 
    BUGS
 
    SEE ALSO
    timer.device/TR_GETSYSTIME
 
    INTERNALS
 
    HISTORY
 
*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct IntuitionBase *,IntuitionBase)

    struct timeval tv;

    GetSysTime(&tv);

    if (seconds) *seconds = tv.tv_secs;
    if (micros)  *micros = tv.tv_micro;

    AROS_LIBFUNC_EXIT
} /* CurrentTime */

