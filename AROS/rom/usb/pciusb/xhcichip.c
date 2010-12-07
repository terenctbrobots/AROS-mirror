/*
    Copyright © 2010, The AROS Development Team. All rights reserved
    $Id$
*/

#include <proto/oop.h>
#include <hidd/pci.h>

#include "uhwcmd.h"

#if defined(USB3)

#undef HiddPCIDeviceAttrBase
#define HiddPCIDeviceAttrBase (hd->hd_HiddPCIDeviceAB)
#undef HiddAttrBase
#define HiddAttrBase (hd->hd_HiddAB)


static
AROS_UFH3(void, xhciResetHandler,
        AROS_UFHA(struct PCIController *, hc, A1),
        AROS_UFHA(APTR, unused, A5),
        AROS_UFHA(struct ExecBase *, SysBase, A6))
{
    AROS_USERFUNC_INIT

    ULONG timeout, temp;

    struct PCIUnit *hu = hc->hc_Unit;
    struct PCIDevice *hd = hu->hu_Device;

	/* Halt controller */
	temp = opreg_readl(XHCI_USBCMD);
	opreg_writel(XHCI_USBCMD, (temp & ~XHCF_CMD_RS));

    /* Spec says "16ms" if conditions are met... */
    timeout = 100;
    do {
        temp = opreg_readl(XHCI_USBSTS);
        if( (temp & XHCF_STS_HCH) ) {
            KPRINTF(1000, ("XHCI Controller halted!\n"));
            break;
        }
        uhwDelayMS(10, hu, hd);
    } while(--timeout);

    #ifdef DEBUG
    if(!timeout)
        KPRINTF(1000, ("XHCI Halting timed out, reset may result in undefined behavior!\n"));
    #endif

	/* Reset controller */
	temp = opreg_readl(XHCI_USBCMD);
	opreg_writel(XHCI_USBCMD, (temp | XHCF_CMD_HCRST));

    AROS_USERFUNC_EXIT
}

IPTR xhciExtCap(struct PCIController *hc, ULONG id, IPTR previous) {

    IPTR extcapoffset, extcap;
    ULONG cnt = XHCI_EXT_CAPS_MAX;

    if(!previous) {  
        extcapoffset = XHCV_xECP(capreg_readl(XHCI_HCCPARAMS));
        if (!extcapoffset) {
            return (IPTR) NULL;
        }
        extcap = (IPTR) hc->xhc_capregbase;
    } else {
        extcap = previous;
        extcapoffset = XHCV_EXT_CAPS_NEXT(READMEM32_LE(READMEM32_LE(extcap)));
    }

    do {
        extcap += extcapoffset;
        if(XHCV_EXT_CAPS_ID(READMEM32_LE(extcap)) == id) {
            KPRINTF(1000, ("XHCI Extended Capability found\n"));
            return extcap;
        }
        extcapoffset = XHCV_EXT_CAPS_NEXT(READMEM32_LE(READMEM32_LE(extcap)));
        cnt--;
    }while(extcapoffset & cnt);

    return (IPTR) NULL;
}

BOOL xhciInit(struct PCIController *hc, struct PCIUnit *hu) {

    struct PCIDevice *hd = hu->hu_Device;

    ULONG cnt, timeout, temp;
    IPTR extcap;
    APTR memptr;
    volatile APTR pciregbase;

    struct TagItem pciActivateMemAndBusmaster[] =
    {
            { aHidd_PCIDevice_isMEM,    TRUE },
            { aHidd_PCIDevice_isMaster, TRUE },
            { TAG_DONE, 0UL },
    };

    /* Activate Mem and Busmaster as pciFreeUnit will disable them! (along with IO, but we don't have that...) */
    OOP_SetAttrs(hc->hc_PCIDeviceObject, (struct TagItem *) pciActivateMemAndBusmaster);

    OOP_GetAttr(hc->hc_PCIDeviceObject, aHidd_PCIDevice_Base0, (APTR) &pciregbase);
    KPRINTF(1000, ("XHCI MMIO address space (%p)\n",pciregbase));

    // Store capregbase in xhc_capregbase
    hc->xhc_capregbase = (APTR) pciregbase;
    KPRINTF(1000, ("XHCI xhc_capregbase(%p)\n",hc->xhc_capregbase));

    // Store opregbase in xhc_opregbase
    hc->xhc_opregbase = (APTR) ((ULONG) pciregbase + capreg_readb(XHCI_CAPLENGTH));
    KPRINTF(1000, ("XHCI xhc_opregbase (%p)\n",hc->xhc_opregbase));

    KPRINTF(1000, ("XHCI CAPLENGTH (%02x)\n",   capreg_readb(XHCI_CAPLENGTH)));
    KPRINTF(1000, ("XHCI Version (%04x)\n",     capreg_readw(XHCI_HCIVERSION)));
    KPRINTF(1000, ("XHCI HCSPARAMS1 (%08x)\n",  capreg_readl(XHCI_HCSPARAMS1)));
    KPRINTF(1000, ("XHCI HCSPARAMS2 (%08x)\n",  capreg_readl(XHCI_HCSPARAMS2)));
    KPRINTF(1000, ("XHCI HCSPARAMS3 (%08x)\n",  capreg_readl(XHCI_HCSPARAMS3)));
    KPRINTF(1000, ("XHCI HCCPARAMS (%08x)\n",   capreg_readl(XHCI_HCCPARAMS)));

    /*
        Chapter 4.20
        System software shall allocate the Scratchpad Buffer(s) before placing the xHC in Run mode (Run/Stop(R/S) = ‘1’).

        The following operations take place to allocate Scratchpad Buffers to the xHC:
        1) Software examines the Max Scratchpad Buffers field in the HCSPARAMS2 register.
        2) Software allocates a Scratchpad Buffer Array with Max Scratchpad Buffers entries.
        3) Software writes the base address of the Scratchpad Buffer Array to the DCBAA (Slot 0) entry.
        4) For each entry in the Scratchpad Buffer Array:
            a. Software allocates a PAGESIZE Scratchpad Buffer.
            b. Software writes the base address of the allocated Scratchpad Buffer to associated entry in the Scratchpad Buffer Array.
    */
    cnt = 0;
    temp = opreg_readl(XHCI_PAGESIZE)&0xffff;
    while((~temp&1) & temp){
        temp = temp>>1;
        cnt++;
    }
    hc->xhc_pagesize = 1<<(cnt+12);

    hc->xhc_scratchbufs = XHCV_SPB_Max(capreg_readl(XHCI_HCSPARAMS2));

    KPRINTF(1000, ("Max Scratchpad Buffers %lx\n",hc->xhc_scratchbufs));
    KPRINTF(1000, ("Pagesize 2^(n+12) = %lx\n",hc->xhc_pagesize));

    hc->hc_NumPorts = XHCV_MaxPorts(capreg_readl(XHCI_HCSPARAMS1));
    KPRINTF(1000, ("MaxSlots %lx\n",XHCV_MaxSlots(capreg_readl(XHCI_HCSPARAMS1))));
    KPRINTF(1000, ("MaxIntrs %lx\n",XHCV_MaxIntrs(capreg_readl(XHCI_HCSPARAMS1))));
    KPRINTF(1000, ("MaxPorts %lx\n",hc->hc_NumPorts));

    extcap = xhciExtCap(hc, XHCI_EXT_CAPS_LEGACY, 0);
    if(extcap) {
        KPRINTF(1000, ("XHCI LEGACY extended cap found\n"));

        temp = READMEM32_LE(extcap);
        if( (temp & XHCF_EC_BIOSOWNED) ){
           KPRINTF(1000, ("XHCI Controller owned by BIOS\n"));

           /* Spec says "no more than a second", we give it a little more */
           timeout = 250;

           WRITEMEM32_LE(extcap, (temp | XHCF_EC_OSOWNED) );
           do {
               temp = READMEM32_LE(extcap);
               if( !(temp & XHCF_EC_BIOSOWNED) ) {
                   KPRINTF(1000, ("BIOS gave up on XHCI. Pwned!\n"));
                   break;
               }
               uhwDelayMS(10, hu, hd);
            } while(--timeout);

            if(!timeout) {
                KPRINTF(1000, ("BIOS didn't release XHCI. Forcing and praying...\n"));
                WRITEMEM32_LE(extcap, (temp & ~XHCF_EC_BIOSOWNED) );
            }
        }
    }

    extcap = xhciExtCap(hc, XHCI_EXT_CAPS_PROTOCOL, 0);
    while(extcap) {
        KPRINTF(1000, ("XHCI PROTOCOL extended cap found\n"));
        extcap = xhciExtCap(hc, XHCI_EXT_CAPS_PROTOCOL, extcap);
        #if DEBUG
        if (!extcap) {
            KPRINTF(1000,("No more extended caps with id=PROTOCOL found\n"));
        }
        #endif
    }

    hc->hc_PCIMemSize = 1024;   //Arbitrary number
    hc->hc_PCIMemSize += (hc->xhc_scratchbufs * hc->xhc_pagesize);

    memptr = HIDD_PCIDriver_AllocPCIMem(hc->hc_PCIDriverObject, hc->hc_PCIMemSize);
    if(!memptr) {
        return FALSE;
    }

    hc->hc_PCIMem = (APTR) memptr;
    // PhysicalAddress - VirtualAdjust = VirtualAddress
    // VirtualAddress  + VirtualAdjust = PhysicalAddress
    hc->hc_PCIVirtualAdjust = ((ULONG) pciGetPhysical(hc, memptr)) - ((ULONG) memptr);
    KPRINTF(10, ("VirtualAdjust 0x%08lx\n", hc->hc_PCIVirtualAdjust));

    /* Reset controller */
    temp = opreg_readl(XHCI_USBCMD);
    opreg_writel(XHCI_USBCMD, (temp | XHCF_CMD_HCRST));

    /* FIXME: Wait until the Controller Not Ready (CNR) flag in the USBSTS is ‘0’ */

    hc->hc_CompleteInt.is_Node.ln_Type = NT_INTERRUPT;
    hc->hc_CompleteInt.is_Node.ln_Name = "XHCI CompleteInt";
    hc->hc_CompleteInt.is_Node.ln_Pri  = 0;
    hc->hc_CompleteInt.is_Data = hc;
    hc->hc_CompleteInt.is_Code = (void (*)(void)) &xhciCompleteInt;

    // install reset handler
    hc->hc_ResetInt.is_Code = xhciResetHandler;
    hc->hc_ResetInt.is_Data = hc;
    AddResetCallback(&hc->hc_ResetInt);

    // add interrupt
    hc->hc_PCIIntHandler.h_Node.ln_Name = "XHCI PCI (pciusb.device)";
    hc->hc_PCIIntHandler.h_Node.ln_Pri = 5;
    hc->hc_PCIIntHandler.h_Code = xhciIntCode;
    hc->hc_PCIIntHandler.h_Data = hc;
    HIDD_IRQ_AddHandler(hd->hd_IRQHidd, &hc->hc_PCIIntHandler, hc->hc_PCIIntLine);

    /* Program the Max Device Slots Enabled (MaxSlotsEn) field */
    /* FIXME: This field shall not be modified by software if the xHC is running (Run/Stop (R/S) = ‘1’) */
    opreg_writel(XHCI_CONFIG, ((opreg_readl(XHCI_CONFIG)&~XHCM_CONFIG_MaxSlotsEn) | hc->hc_NumPorts) );

    /* Program the Device Context Base Address Array Pointer (DCBAAP) */

    /* Define the Command Ring Dequeue Pointer by programming the Command Ring Control Register */

    return TRUE;
}

void xhciFree(struct PCIController *hc, struct PCIUnit *hu) {

    hc = (struct PCIController *) hu->hu_Controllers.lh_Head;
    while(hc->hc_Node.ln_Succ)
    {
        switch(hc->hc_HCIType)
        {
            case HCITYPE_XHCI:
            {
                KPRINTF(1000, ("Shutting down XHCI %08lx\n", hc));
                KPRINTF(1000, ("Shutting down XHCI done.\n"));
                break;
            }
        }

        hc = (struct PCIController *) hc->hc_Node.ln_Succ;
    }
}

#endif
