/*
    Copyright � 2017, The AROS Development Team. All rights reserved.
    $Id: $
*/

#include <exec/types.h>
#include <exec/libraries.h>
#include <dos/dosextens.h>
#include <aros/libcall.h>
#include <aros/debug.h>

#include <proto/exec.h>
#include <proto/timer.h>

#include <aros/symbolsets.h>
#include LC_LIBDEFS_FILE

#include <utility/date.h>


/*** Variables **************************************************************/
struct Device      *TimerBase;
struct timerequest *TimerIO;

/*** Library startup and shutdown *******************************************/
static int Graph_StartTimer(LIBBASETYPEPTR LIBBASE)
{
    TimerIO   = NULL;
    TimerBase = NULL;
    
    TimerIO = AllocMem(sizeof(struct timerequest), MEMF_CLEAR);
    if(TimerIO == NULL) goto error;
    
    TimerIO->tr_node.io_Message.mn_Length = sizeof(struct timerequest);
    
    if
    (
        0 == OpenDevice
        (
            "timer.device", UNIT_VBLANK, (struct IORequest *) TimerIO, 0
        )
    )
    {
        TimerBase = (struct Device *) TimerIO->tr_node.io_Device;
    }
    else
    {
        goto error;
    }
    
    return TRUE;

error:
    if (TimerBase != NULL) CloseDevice((struct IORequest *) TimerIO);
    if (TimerIO != NULL) FreeMem(TimerIO, sizeof(struct timerequest));
    
    return FALSE;
}

static int Graph_StopTimer(LIBBASETYPEPTR LIBBASE)
{
    if (TimerBase != NULL) CloseDevice((struct IORequest *) TimerIO);
    if (TimerIO != NULL) FreeMem(TimerIO, sizeof( struct timerequest ));
    return TRUE;
}

ADD2INITLIB(Graph_StartTimer, 1);
ADD2EXPUNGELIB(Graph_StopTimer, 1);
