#ifndef _ETASK_H
#define _ETASK_H

/*
    Copyright (C) 1995-2001 AROS - The Amiga Research OS
    $Id$

    Desc: Internal description of the ETask structure
    Lang: english
*/
#ifndef EXEC_TASKS_H
#   include <exec/tasks.h>
#endif

struct IntETask
{
    struct ETask iet_ETask;
#ifdef DEBUG_ETASK
    STRPTR	 iet_Me;
#endif
    APTR	 iet_RT;	/* Structure for resource tracking */
    APTR	 iet_Context;	/* Structure to store CPU registers */
    APTR         iet_AroscUserData; /* Structure to store shared clib's data */
};

#define GetIntETask(task)   ((struct IntETask *)(((struct Task *) \
				(task))->tc_UnionETask.tc_ETask))
#define IntETask(etask)	    ((struct IntETask *)(etask))

#endif /* _ETASK_H */
