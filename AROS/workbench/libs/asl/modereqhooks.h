#ifndef MODEREQHOOKS_H
#define MODEREQHOOKS_H

/*
    (C) 1997 AROS - The Amiga Research OS
    $Id$

    Desc: File requester specific defs.
    Lang: english
*/

#ifndef LAYOUT_H
#    include "layout.h"
#endif
#ifndef INTUITION_CLASSES_H
#   include <intuition/classes.h>
#endif
#ifndef DOS_EXALL_H
#   include <dos/exall.h>
#endif
#ifndef DOS_DOSEXTENS_H
#   include <dos/dosextens.h>
#endif

/* Options */

#define SREQ_MIN_VISIBLELINES 	 	3
#define SREQ_COOL_BUTTONS 		1

#define SREQ_FIRST_OBJECT(x) ((x)->Listview)
#define SREQ_LAST_OBJECT(x) ((x)->AutoScrollGadget)

struct SMUserData
{
    Object			*Listview;	
    Object			*OKBut;
    Object			*CancelBut;
    Object			*OverscanLabel;
    Object			*WidthLabel;
    Object			*HeightLabel;
    Object			*DepthLabel;
    Object			*AutoScrollLabel;
    Object			*OverscanGadget;
    Object			*WidthGadget;
    Object			*HeightGadget;
    Object			*DepthGadget;
    Object			*AutoScrollGadget;
    
    struct ScrollerGadget 	ScrollGad;
    struct List			ListviewList;
    struct Hook			ListviewHook;
    UWORD 			ButWidth;
    UWORD 			ButHeight;    
    STRPTR			colorarray[33];
    UBYTE			colortext[150];
    UBYTE 			Flags;
	
};

/* Has the gadgetry been layouted before ? */
#define SMFLG_LAYOUTED 		(1 << 0)

/* Menu IDs */

#define SMMEN_LASTMODE		1
#define SMMEN_NEXTMODE		2 
#define SMMEN_PROPERTYLIST	3 
#define SMMEN_RESTORE		4 
#define SMMEN_OK		5 
#define SMMEN_CANCEL		6 

#endif /* MODEREQHOOKS_H */
