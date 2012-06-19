/*
    Copyright © 2010, The AROS Development Team. All rights reserved
    $Id$
*/

#include <proto/exec.h>
#include <proto/oop.h>
#include <hidd/pci.h>

#include <devices/usb_hub.h>

#include "uhwcmd.h"

#ifdef AROS_USB30_CODE

#undef HiddPCIDeviceAttrBase
#define HiddPCIDeviceAttrBase (hd->hd_HiddPCIDeviceAB)
#undef HiddAttrBase
#define HiddAttrBase (hd->hd_HiddAB)

/* Make sure we clear (set as 0) HW register when assigning it as NULL(PTR) */
#define NULLPTR 0

static
AROS_UFH3(void, xhciResetHandler,
        AROS_UFHA(struct PCIController *, hc, A1),
        AROS_UFHA(APTR, unused, A5),
        AROS_UFHA(struct ExecBase *, SysBase, A6))
{
    AROS_USERFUNC_INIT

	/* Halt controller */
    #ifdef DEBUG
    if(!xhciHaltHC(hc))
        KPRINTF(1000, ("Halting HC failed, reset may result in undefined behavior!\n"));
    #else
    xhciHaltHC(hc);
    #endif

	/* Reset controller */
    xhciResetHC(hc);

    AROS_USERFUNC_EXIT
}

void xhciCompleteInt(struct PCIController *hc)
{
    KPRINTF(1, ("CompleteInt!\n"));

    KPRINTF(1, ("CompleteDone\n"));
}

void xhciIntCode(HIDDT_IRQ_Handler *irq, HIDDT_IRQ_HwInfo *hw)
{
    struct PCIController *hc = (struct PCIController *) irq->h_Data;
//    struct PCIDevice *base = hc->hc_Device;
//    struct PCIUnit *unit = hc->hc_Unit;

    ULONG intr, portn;

    intr = opreg_readl(XHCI_USBSTS);
    if(intr & XHCF_STS_EINT) {
        /* Clear (RW1C) Event Interrupt (EINT) */
        opreg_writel(XHCI_USBSTS, XHCF_STS_EINT);

        if(hc->hc_Online) {

            if(intr & XHCF_STS_HSE) {
                KPRINTF(1000, ("Host System Error (HSE)!\n"));
            }

            if(intr & XHCF_STS_PCD) {
                /* There are seven status change bits in the PORTSC register,
                        Connect Status Change (CSC)
                        Port Enabled/Disabled Change (PEC)
                        Warm Port Reset Change (WRC)
                        Over-current Change (OCC)
                        Port Reset Change (PRC)
                        Port Link State Change (PLC)
                        Port Config Error Change (CEC)
                */
                for (portn = 1; portn <= hc->xhc_NumPorts; portn++) {
                    if (opreg_readl(XHCI_PORTSC(portn)) & (XHCF_PS_CSC|XHCF_PS_PEC|XHCF_PS_OCC|XHCF_PS_WRC|XHCF_PS_PRC|XHCF_PS_PLC|XHCF_PS_CEC)) {
                            KPRINTF(1000,("port %d changed\n", portn));
                    }
                }
            }

            if(intr & XHCF_STS_SRE) {
                KPRINTF(1000, ("Host Controller Error (HCE)!\n"));
            }

        } // not online
    }
}

/* FIXME: Redefine these quick calls and the main function */
#define xhciCreateNormalTRB() xhciCreateTRB(hc, TRBTYPE_NORMAL)
#define xhciCreateIsoChTRB() xhciCreateTRB(hc, TRBTYPE_ISOCH)

APTR xhciCreateTRB(struct PCIController *hc, ULONG trb_type) {

    IPTR *TRB;

    TRB = AllocVecAligned(TRB_TEMPLATE_SIZE, 16);

    if(TRB) {
        return (APTR) TRB;
    }

    return NULLPTR;
}

IPTR xhciSearchExtCap(struct PCIController *hc, ULONG id, IPTR extcap) {

    IPTR extcapoff = (IPTR) 0;
    ULONG cnt = XHCI_EXT_CAPS_MAX;

    KPRINTF(100,("search for ext cap with id(%ld)\n", id));

    if(extcap) {
        KPRINTF(100, ("continue search from %p\n", extcap));
        extcap = (IPTR) XHCV_EXT_CAPS_NEXT(READMEM32_LE(extcap));
    } else {  
        extcap = (IPTR) hc->xhc_capregbase + XHCV_xECP(capreg_readl(XHCI_HCCPARAMS));
        KPRINTF(100, ("search from the beginning %p\n", extcap));
    }

    do {
        extcap += extcapoff;
        if((XHCV_EXT_CAPS_ID(READMEM32_LE(extcap)) == id)) {
            KPRINTF(100, ("found matching ext cap %lx\n", extcap));
            return (IPTR) extcap;
        }
        #if DEBUG
        if(extcap)
            KPRINTF(100, ("skipping ext cap with id(%ld)\n", XHCV_EXT_CAPS_ID(READMEM32_LE(extcap))));
        #endif
        extcapoff = (IPTR) XHCV_EXT_CAPS_NEXT(READMEM32_LE(extcap));
        cnt--;
    } while(cnt & extcapoff);

    KPRINTF(100, ("not found!\n"));
    return (IPTR) 0;
}

BOOL xhciHaltHC(struct PCIController *hc) {

    struct PCIUnit *hu = hc->hc_Unit;

    ULONG timeout, temp;

    /* Halt the controller by clearing Run/Stop bit */
    temp = opreg_readl(XHCI_USBCMD);
    opreg_writel(XHCI_USBCMD, (temp & ~XHCF_CMD_RS));

    /*
        The xHC shall halt within 16 ms. after software clears the Run/Stop bit if certain conditions have been met.
        The HCHalted (HCH) bit in the USBSTS register indicates when the xHC has finished its
        pending pipelined transactions and has entered the stopped state. 
    */
    timeout = 250;  //FIXME: arbitrary value of 2500ms
    do {
        temp = opreg_readl(XHCI_USBSTS);
        if( (temp & XHCF_STS_HCH) ) {
            KPRINTF(1000, ("controller halted!\n"));
            return TRUE;
        }
        uhwDelayMS(10, hu);
    } while(--timeout);

    KPRINTF(1000, ("halt failed!\n"));
    return FALSE;
}

BOOL xhciResetHC(struct PCIController *hc) {

    struct PCIUnit *hu = hc->hc_Unit;

    ULONG timeout, temp;

    /* Reset controller by setting HCRST-bit */
    temp = opreg_readl(XHCI_USBCMD);
    opreg_writel(XHCI_USBCMD, (temp | XHCF_CMD_HCRST));

    /*
        Controller clears HCRST bit when reset is done, wait for it and the CNR-bit to be cleared
    */
    timeout = 250;  //FIXME: arbitrary value of 2500ms
    do {
        temp = opreg_readl(XHCI_USBCMD);
        if( !(temp & XHCF_CMD_HCRST) ) {
            /* Wait for CNR-bit to clear */
            timeout = 250;  //FIXME: arbitrary value of 2500ms
            do {
                temp = opreg_readl(XHCI_USBSTS);
                if( !(temp & XHCF_STS_CNR) ) {
                    KPRINTF(1000, ("reset succeeded!\n"));
                    return TRUE;
                }
                uhwDelayMS(10, hu);
            } while(--timeout);
            return FALSE;
        }
        uhwDelayMS(10, hu);
    } while(--timeout);

    KPRINTF(1000, ("reset failed!\n"));
    return FALSE;
}

BOOL xhciInit(struct PCIController *hc, struct PCIUnit *hu) {

    struct PCIDevice *hd = hu->hu_Device;

    ULONG cnt, timeout, temp;
    IPTR extcap;
    APTR memptr = NULLPTR;

    volatile APTR pciregbase;

    struct TagItem pciActivateMemAndBusmaster[] =
    {
            { aHidd_PCIDevice_isIO,     FALSE },
            { aHidd_PCIDevice_isMEM,    TRUE },
            { aHidd_PCIDevice_isMaster, TRUE },
            { TAG_DONE, 0UL },
    };

//    memptr = AllocVecAligned(2000, 0x100);
//    KPRINTF(1000,("AllocVedAligned %p\n", memptr));
//    FreeVecAligned(memptr);

    /* Activate Mem and Busmaster as pciFreeUnit will disable them! (along with IO, but we don't have that...) */
    OOP_SetAttrs(hc->hc_PCIDeviceObject, (struct TagItem *) pciActivateMemAndBusmaster);

    OOP_GetAttr(hc->hc_PCIDeviceObject, aHidd_PCIDevice_Base0, (APTR) &pciregbase);
//    KPRINTF(1000, ("XHCI MMIO address space (%p)\n",pciregbase));

    // Store capregbase in xhc_capregbase
    hc->xhc_capregbase = (APTR) pciregbase;
    KPRINTF(1000, ("xhc_capregbase (%p)\n",hc->xhc_capregbase));

    // Store opregbase in xhc_opregbase
    hc->xhc_opregbase = (APTR) (pciregbase + capreg_readb(XHCI_CAPLENGTH));
    KPRINTF(1000, ("xhc_opregbase (%p)\n",hc->xhc_opregbase));

    // Store doorbellbase in xhc_doorbellbase
    hc->xhc_doorbellbase = (APTR) (hc->xhc_capregbase + capreg_readl(XHCI_DBOFF));
    KPRINTF(1000, ("xhc_doorbellbase (%p)\n",hc->xhc_doorbellbase));

    // Store runtimebase in xhc_runtimebase
    hc->xhc_runtimebase = (APTR) (hc->xhc_capregbase + capreg_readl(XHCI_RTSOFF));
    KPRINTF(1000, ("xhc_runtimebase (%p)\n",hc->xhc_runtimebase));

//    KPRINTF(1000, ("XHCI CAPLENGTH (%02x)\n",   capreg_readb(XHCI_CAPLENGTH)));
//    KPRINTF(1000, ("XHCI Version (%04x)\n",     capreg_readw(XHCI_HCIVERSION)));
//    KPRINTF(1000, ("XHCI HCSPARAMS1 (%08x)\n",  capreg_readl(XHCI_HCSPARAMS1)));
//    KPRINTF(1000, ("XHCI HCSPARAMS2 (%08x)\n",  capreg_readl(XHCI_HCSPARAMS2)));
//    KPRINTF(1000, ("XHCI HCSPARAMS3 (%08x)\n",  capreg_readl(XHCI_HCSPARAMS3)));
//    KPRINTF(1000, ("XHCI HCCPARAMS (%08x)\n",   capreg_readl(XHCI_HCCPARAMS)));

    /*
        This field defines the page size supported by the xHC implementation.
        This xHC supports a page size of 2^(n+12) if bit n is Set. For example,
        if bit 0 is set, the xHC supports 4k byte page sizes.
    */
    cnt = 12;
    temp = opreg_readl(XHCI_PAGESIZE)&0xffff;
    while((~temp&1) & temp){
        temp = temp>>1;
        cnt++;
    }
    hc->xhc_pagesize = 1<<(cnt);
    KPRINTF(1000, ("Pagesize 2^(n+12) = 0x%lx\n", hc->xhc_pagesize));

    /* Testing scratchpad allocations */
//    hc->xhc_scratchpads = 4;
    hc->xhc_scratchpads = XHCV_SPB_Max(capreg_readl(XHCI_HCSPARAMS2));
    KPRINTF(1000, ("Max Scratchpad Buffers %ld\n",hc->xhc_scratchpads));

    hc->xhc_NumPorts = XHCV_MaxPorts(capreg_readl(XHCI_HCSPARAMS1));
    KPRINTF(1000, ("MaxPorts %ld\n",hc->xhc_NumPorts));

    /*
        We don't yeat know how many we have each of them, xhciParseSupProtocol takes care of that
    */
    hc->xhc_NumPorts20 = 0;
    hc->xhc_NumPorts30 = 0;

    /*
        Number of Device Slots (MaxSlots). This field specifies the maximum number of Device
        Context Structures and Doorbell Array entries this host controller can support. Valid values are
        in the range of 1 to 255. The value of ‘0’ is reserved, fail gracefully on it
    */
    hc->xhc_maxslots = XHCV_MaxSlots(capreg_readl(XHCI_HCSPARAMS1));

    if(hc->xhc_maxslots == 0){
        KPRINTF(1000, ("MaxSlots count is 0, failing!\n"));
        return FALSE;
    }

    KPRINTF(1000, ("MaxSlots %ld\n",hc->xhc_maxslots));


    /*
        If the Number of Interrupters (MaxIntrs) field is greater than 1, then Interrupter Mapping shall be supported
        If Interrupt Pin support is enabled, then only Interrupter 0 is enabled and any other Interrupters are disabled (This is the case with Aros)
    */
    KPRINTF(1000, ("MaxIntrs %ld\n",XHCV_MaxIntrs(capreg_readl(XHCI_HCSPARAMS1))));

    /* 64 byte or 32 byte context data structures? */
    if(capreg_readl(XHCI_HCCPARAMS) & XHCF_CSZ) {
        hc->xhc_contextsize64=TRUE; 
    }

    /* xHCI Extended Capabilities, search for USB Legacy Support */
    extcap = xhciSearchExtCap(hc, XHCI_EXT_CAPS_LEGACY, 0);
    if(extcap) {

        temp = READMEM32_LE(extcap);
        if( (temp & XHCF_BIOSOWNED) ){
           KPRINTF(1000, ("controller owned by BIOS\n"));

           /* Spec says "no more than a second", we give it a little more */
           timeout = 250;

           WRITEMEM32_LE(extcap, (temp | XHCF_OSOWNED) );
           do {
               temp = READMEM32_LE(extcap);
               if( !(temp & XHCF_BIOSOWNED) ) {
                   KPRINTF(1000, ("BIOS gave up on XHCI. Pwned!\n"));
                   break;
               }
               uhwDelayMS(10, hu);
            } while(--timeout);

            if(!timeout) {
                KPRINTF(1000, ("BIOS didn't release XHCI. Forcing and praying...\n"));
                WRITEMEM32_LE(extcap, (temp & ~XHCF_BIOSOWNED) );
            }
        }
    }

    /* XHCI spec says that there is at least one "Supported Protocol" capability, fail if none is found as this is used for port logic */
    extcap = xhciSearchExtCap(hc, XHCI_EXT_CAPS_PROTOCOL, 0);
    if(extcap) {
        KPRINTF(1000, ("Supported Protocol found!\n"));
        xhciParseSupProtocol(hc, extcap);

        /* Parse rest, if any...*/
        do {
            extcap = xhciSearchExtCap(hc, XHCI_EXT_CAPS_PROTOCOL, extcap);
            if(extcap) {
                KPRINTF(1000, ("More Supported Protocols found!\n"));
                xhciParseSupProtocol(hc, extcap);
            }
        } while (extcap);
    }else{
        KPRINTF(1000, ("No Supported Protocol found, failing!\n"));
        return FALSE;
    }

    /*
        If no USB2.0 ports were found but max port count is greater than USB3.0 count assume the overhead to be USB2.0
    */
    if( (hc->xhc_NumPorts < (hc->xhc_NumPorts20 + hc->xhc_NumPorts30)) ) {
        KPRINTF(1000, ("Too many ports in Supported Protocol!\n"));
        return FALSE;
    }else if ( (hc->xhc_NumPorts > (hc->xhc_NumPorts20 + hc->xhc_NumPorts30)) ) {
        hc->xhc_NumPorts20 = (hc->xhc_NumPorts - hc->xhc_NumPorts30);
    }
    KPRINTF(1000, ("Number of USB2.0 ports %ld\n", hc->xhc_NumPorts20 ));
    KPRINTF(1000, ("Number of USB3.0 ports %ld\n", hc->xhc_NumPorts30 ));

    if(xhciHaltHC(hc)) {
        if(xhciResetHC(hc)) {

//            for(cnt = 1; cnt <=hc->xhc_NumPorts; cnt++) {
//                temp = opreg_readl(XHCI_PORTSC(cnt));
//                KPRINTF(1000, ("Attached device's speed on port #%d is %d (PORTSC %lx)\n",cnt, XHCV_PS_SPEED(temp), temp ));
//            }

            hc->hc_PCIMemSize = 1024;   //Arbitrary number

            /* CHECKME: Removed this memory allocation as it was not used in any way (at least for now) */
//            memptr = HIDD_PCIDriver_AllocPCIMem(hc->hc_PCIDriverObject, hc->hc_PCIMemSize);
//            hc->hc_PCIMem = (APTR) memptr;
            hc->hc_PCIMem = NULL;

//            if(memptr) {
            {
//                PhysicalAddress - VirtualAdjust = VirtualAddress
//                VirtualAddress  + VirtualAdjust = PhysicalAddress
//                hc->hc_PCIVirtualAdjust = pciGetPhysical(hc, memptr) - (APTR)memptr;
//                KPRINTF(10, ("VirtualAdjust 0x%08lx\n", hc->hc_PCIVirtualAdjust));

                hc->hc_CompleteInt.is_Node.ln_Type = NT_INTERRUPT;
                hc->hc_CompleteInt.is_Node.ln_Name = "XHCI CompleteInt";
                hc->hc_CompleteInt.is_Node.ln_Pri  = 0;
                hc->hc_CompleteInt.is_Data = hc;
                hc->hc_CompleteInt.is_Code = (void (*)(void)) &xhciCompleteInt;

                /* add reset handler */
                hc->hc_ResetInt.is_Code = xhciResetHandler;
                hc->hc_ResetInt.is_Data = hc;
                AddResetCallback(&hc->hc_ResetInt);

                /* add interrupt handler */
                hc->hc_PCIIntHandler.h_Node.ln_Name = "XHCI PCI (pciusb.device)";
                hc->hc_PCIIntHandler.h_Node.ln_Pri = 5;
                hc->hc_PCIIntHandler.h_Code = xhciIntCode;
                hc->hc_PCIIntHandler.h_Data = hc;
                HIDD_IRQ_AddHandler(hd->hd_IRQHidd, &hc->hc_PCIIntHandler, hc->hc_PCIIntLine);

                /* Clears (RW1C) Host System Error(HSE), Event Interrupt(EINT), Port Change Detect(PCD) and Save/Restore Error(SRE) */
                temp = opreg_readl(XHCI_USBSTS);
                opreg_writel(XHCI_USBSTS, temp);

                /* After reset all notifications should be automatically disabled but ensure anyway, preserve reserved bits */
                opreg_writel(XHCI_DNCTRL, ( (opreg_readl(XHCI_DNCTRL) & ~XHCM_DNCTRL) | 0) );

                /* Program the Max Device Slots Enabled (MaxSlotsEn) field, preserve reserved bits */
                /*
                    Max Device Slots Enabled (MaxSlotsEn) – RW. Default = ‘0’. This field specifies the maximum
                    number of enabled Device Slots. Valid values are in the range of 0 to MaxSlots. Enabled Devices
                    Slots are allocated contiguously. e.g. A value of 16 specifies that Device Slots 1 to 16 are active.
                    A value of ‘0’ disables all Device Slots. A disabled Device Slot shall not respond to Doorbell
                    Register references.
                    This field shall not be modified by software if the xHC is running (Run/Stop (R/S) = ‘1’).
                */
                opreg_writel(XHCI_CONFIG, ( (opreg_readl(XHCI_CONFIG) & ~XHCM_CONFIG_MaxSlotsEn) | hc->xhc_maxslots) );


                /*
                    Scratchpad array is 64 byte aligned as is Device Context Base array, neither can cross page boundary
                */

                hc->xhc_dcbaa = AllocVecAligned( ((hc->xhc_maxslots + 1)*sizeof(UQUAD)) , hc->xhc_pagesize);
                if(hc->xhc_dcbaa) {
                    KPRINTF(1000, ("Device context base array at %p\n", hc->xhc_dcbaa));

                    if( !(hc->xhc_scratchpads) ) {
                        /* Host controller does not use scratchpads (This is the case OnMyHW™) */
                        hc->xhc_scratchpadarray = NULLPTR;

                    }else{
                        hc->xhc_scratchpadarray = AllocVecAligned( (hc->xhc_scratchpads*sizeof(UQUAD) ), hc->xhc_pagesize);
                        if( !(hc->xhc_scratchpadarray) ){
                            KPRINTF(1000, ("Unable to allocate scratchpad array, failing!\n"));
                            return FALSE;
                        }
                        KPRINTF(1000, ("Allocated scratchpad buffer array at %p\n", hc->xhc_scratchpadarray));

                        for(temp = 0; temp<hc->xhc_scratchpads; temp++){
                            memptr = AllocVecAligned(hc->xhc_pagesize, hc->xhc_pagesize);
                            if(memptr){
                                hc->xhc_scratchpadarray[temp] = (UQUAD) ((IPTR) memptr);
                                KPRINTF(1000, ("hc->xhc_scratchpadarray[%d] = %0lx:%0lx\n", temp, (ULONG) ((UQUAD)(hc->xhc_scratchpadarray[temp])>>32), (ULONG) hc->xhc_scratchpadarray[temp]));
                            }else{
                                FreeVecAligned( (APTR) hc->xhc_dcbaa );
                                for(temp = 0; temp<hc->xhc_scratchpads; temp++){
                                    if( ((UQUAD) hc->xhc_scratchpadarray[temp]) ){
                                        FreeVecAligned( (APTR) ((UQUAD) hc->xhc_scratchpadarray[temp]) );
                                    }
                                }
                                return FALSE;
                            }
                        }
                    }
                    hc->xhc_dcbaa[0] = (UQUAD) ((IPTR) hc->xhc_scratchpadarray);

                    /* DCBAA[0]                     QUAD pointer scratchpad array or NULLPTR */
                    /* DCBAA[1] - DCBAA[maxslots]   QUAD pointer to device slots */
                    KPRINTF(1000, ("DCBAA[0] = %0lx:%0lx\n", (IPTR) ((UQUAD)(hc->xhc_dcbaa[0])>>32), (IPTR) hc->xhc_dcbaa[0]));
                    for(temp = 1; temp<=(hc->xhc_maxslots); temp++){
                        KPRINTF(1000, ("DCBAA[%d] = %0lx:%0lx\n", temp, (IPTR) ((UQUAD)(hc->xhc_dcbaa[temp])>>32), (IPTR) hc->xhc_dcbaa[temp]));
                    }
                    opreg_writeq(XHCI_DCBAAP, (UQUAD) ((IPTR)hc->xhc_dcbaa) );

                    /*
                        Event Ring Segment Table Size – RW. Default = ‘0’. This field identifies the number of valid
                        Event Ring Segment Table entries in the Event Ring Segment Table pointed to by the Event
                        Ring Segment Table Base Address register. The maximum value supported by an xHC
                        implementation for this register is defined by the ERST Max field in the HCSPARAMS2 register
                    */
                    KPRINTF(1000, ("ERSTSZ %lx\n", IRS_readl(0, XHCI_ERSTSZ)));     //16bit
                    KPRINTF(1000, ("ERSTBA %lx:%lx\n", (ULONG) ((UQUAD) (IRS_readq(0, XHCI_ERSTBA))>>32), (ULONG) IRS_readq(0, XHCI_ERSTBA)));
                    KPRINTF(1000, ("ERDP %lx:%lx\n", (ULONG) ((UQUAD) (IRS_readq(0, XHCI_ERDP))>>32), (ULONG) IRS_readq(0, XHCI_ERDP)));

                    /* Define the Command Ring Dequeue Pointer by programming the Command Ring Control Register */

                    /* Set interrupt enable(IE) bit for interrupter 0 */
                    KPRINTF(1000, ("IMAN[0] %lx\n", IRS_readl(0,XHCI_IMAN)));
                    IRS_writel(0, XHCI_IMAN, ((IRS_readl(0,XHCI_IMAN)) | XHCF_IE) );
                    KPRINTF(1000, ("IMAN[0] %lx\n", IRS_readl(0,XHCI_IMAN)));

                    /* Set Run/Stop(R/S), Interrupter Enable(INTE) and Host System Error Enable(HSEE) */
                    KPRINTF(1000, ("USBCMD %lx\n", opreg_readl(XHCI_USBCMD)));
                    opreg_writel(XHCI_USBCMD, (XHCF_CMD_RS | XHCF_CMD_INTE | XHCF_CMD_HSEE) );
                    KPRINTF(1000, ("USBCMD %lx\n", opreg_readl(XHCI_USBCMD)));

                    KPRINTF(1000, ("xhciInit returns TRUE...\n"));
                    return TRUE;
                }
                KPRINTF(1000, ("Unable to allocate device context base array, failing!\n"));
            }
        }
    }

    KPRINTF(1000, ("xhciInit returns FALSE...\n"));
    return FALSE;
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
                xhciHaltHC(hc);
                uhwDelayMS(50, hu);
                SYNC;
                KPRINTF(1000, ("Shutting down XHCI done.\n"));
                break;
            }
        }

        hc = (struct PCIController *) hc->hc_Node.ln_Succ;
    }
}

void xhciParseSupProtocol(struct PCIController *hc, IPTR extcap) {

    ULONG temp1, temp2;

    temp1 = READMEM32_LE(extcap);
    KPRINTF(1000, ("Version %l.%l\n", XHCV_SPFD_RMAJOR(temp1), XHCV_SPFD_RMINOR(temp1) ));

    temp2 = READMEM32_LE(extcap + XHCI_SPPORT);
//    KPRINTF(1000, ("CPO %ld\n", XHCV_SPPORT_CPO(temp2) ));
//    KPRINTF(1000, ("CPCNT %ld\n", XHCV_SPPORT_CPCNT(temp2) ));
//    KPRINTF(1000, ("PD %ld\n", XHCV_SPPORT_PD(temp2) ));
//    KPRINTF(1000, ("PSIC %ld\n", XHCV_SPPORT_PSIC(temp2) ));

    /*
        FIXME:
            -We might not get at all "USB2 Supported Protocol", in that case make a wild assumption on # USB2.0 ports
            -Check if the name string is "USB " (=0x20425355)
            -Map USB specifications to their respective ports (USB2.0/USB3.0) 
    */

    if(XHCV_SPFD_RMAJOR(temp1) == 2) {
        hc->xhc_NumPorts20 = ((XHCV_SPPORT_CPCNT(temp2) - XHCV_SPPORT_CPO(temp2) + 1));
    }
    if(XHCV_SPFD_RMAJOR(temp1) == 3) {
        hc->xhc_NumPorts30 = ((XHCV_SPPORT_CPCNT(temp2) - XHCV_SPPORT_CPO(temp2) + 1));
    }
}

/*
    Allocate aligned memory (call with ALIGNMENT = PAGESIZE for onpage memory, will result in PAGESIZE overhead memory usage)
*/
APTR AllocVecAligned(ULONG bytesize, ULONG alignment) {
//    KPRINTF(1000, ("AllocVecAligned %ld, %ld\n", bytesize, alignment ));

    IPTR temp, *ret;

    /*
        Allocate aligned memory by ourself as OS doesn't seem to give us any support for aligned allocations (ONPAGE,ALIGNMENT)
        -We only support alignment of sizeof(IPTR) and up since we store the original ptr, sizeof(IPTR) is platform dependant
    */
    if(alignment<sizeof(IPTR)) {
        alignment = sizeof(IPTR);
    }

    temp = (IPTR) AllocVec( bytesize+alignment, (MEMF_PUBLIC | MEMF_CLEAR) );
    if(temp) {
//        KPRINTF(1000, ("got memory @ %p with alignment %ld\n", temp, alignment ));

        /*
            If by coincidence we get aligned memory, we still add the alignment size to the pointer and align it to the next possible aligned address
            as we need memory below our allocation.
        */
        ret = (APTR)((IPTR)(temp+alignment) & ~(alignment-1));
//        KPRINTF(1000, ("final memory @ %p\n", ret ));

        /*
            Store our original memory pointer below our aligned memory (we have allocated memory there)
        */
        ret[-1] = temp;
        return ret;
    }
    return NULLPTR;
}

void FreeVecAligned(APTR memory) {
    IPTR *ptr = memory;
//    KPRINTF(1000, ("FreeVecAligned @ %p\n", ptr[-1] ));
    FreeVec((APTR)ptr[-1]);
}

#endif
