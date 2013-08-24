#include <aros/kernel.h>
#include <aros/libcall.h>

#include "kernel_base.h"
#include "kernel_intern.h"
#include "apic.h"

AROS_LH0(unsigned int, KrnGetCPUNumber,
	 struct KernelBase *, KernelBase, 37, Kernel)
{
    AROS_LIBFUNC_INIT

    if (KernelBase && KernelBase->kb_PlatformData)
        return core_APIC_GetNumber(KernelBase->kb_PlatformData->kb_APIC);
    else
        return 0;

    AROS_LIBFUNC_EXIT
}
