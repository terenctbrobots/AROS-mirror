/*
    (C) 1995 AROS - The Amiga Replacement OS
    $Id$    $Log

    Desc:
    Lang: english
*/
#include "exec_intern.h"
#include <exec/resident.h>
#include <proto/exec.h>

/*****************************************************************************

    NAME */

	AROS_LH1(struct Resident *, FindResident,

/*  SYNOPSIS */
	AROS_LHA(UBYTE *, name, A1),

/*  LOCATION */
	struct ExecBase *, SysBase, 16, Exec)

/*  FUNCTION
	Search for a Resident module in the system resident list.

    INPUTS
	name - pointer to the name of a Resident module to find

    RESULT
	pointer to the Resident module (struct Resident *), or null if
	not found.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY
	29-10-95    digulla automatically created from
			    exec_lib.fd and clib/exec_protos.h

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct ExecBase *,SysBase)

    ULONG *list;

    list = SysBase->ResModules;

    if(list)
    {
	while(*list)
	{
	    /*
		If bit 31 is set, this doesn't point to a Resident module, but
		to another list of modules.
	    */
	    if(*list & 0x80000000) list = (ULONG *)(*list & 0x7fffffff);

	    if(!(strcmp( ((struct Resident *)*list)->rt_Name, name)) )
	    {
		return (struct Resident *)*list;
	    }

	    list++;
	}
    }

    return NULL;
    AROS_LIBFUNC_EXIT
} /* FindResident */
