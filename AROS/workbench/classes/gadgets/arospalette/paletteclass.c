/*
    (C) 1995-97 AROS - The Amiga Replacement OS
    $Id$

    Desc:
    Lang: english
*/

#include <proto/utility.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/cghooks.h>
#include <intuition/gadgetclass.h>
#include <utility/tagitem.h>
#include <gadgets/arospalette.h>
#include <aros/asmcall.h>
#include <stdlib.h> /* abs() */
#include "arospalette_intern.h"

#define SDEBUG 0
#define DEBUG 0
#include <aros/debug.h>

#define AROSPaletteBase ((struct PaletteBase_intern *)(cl->cl_UserData))

/*********************
**  Palette::Set()  **
*********************/
STATIC IPTR palette_set(Class *cl, Object *o, struct opSet *msg)
{
    struct TagItem *tag, *tstate;
    IPTR retval = 0UL;
    struct PaletteData *data = INST_DATA(cl, o);
    
    EnterFunc(bug("Palette::Set()\n"));
    
    for (tstate = msg->ops_AttrList; (tag = NextTagItem(&tstate)); )
    {
    	IPTR tidata = tag->ti_Data;
    	
    	switch (tag->ti_Tag)
    	{
    	case AROSA_Palette_Depth:		/* [IU] */
    	    data->pd_Depth = (UBYTE)tidata;
    	    D(bug("Depth initialized to %d\n", tidata));

    	    /* Relayout the gadget */
    	    DoMethod(o, GM_LAYOUT, msg->ops_GInfo, FALSE);
    	    retval = 1UL;
    	    
    	    /* Check if the old selected fits into the new depth */
    	    if (data->pd_Color > (1 << data->pd_Depth) - 1)
    	    	data->pd_Color = 0;
    	    
    	    break;
    	    
    	case AROSA_Palette_Color:		/* [IS] */
    	    data->pd_Color = (UBYTE)tidata;
    	    data->pd_OldColor = data->pd_Color;
    	    retval = 1UL;
    	    
    	    D(bug("Color set to %d\n", tidata));    	    
    	    retval = 1UL;
    	    break;
    	    
    	case AROSA_Palette_ColorOffset:		/* [I] */
    	    data->pd_ColorOffset = (UBYTE)tidata;
    	    D(bug("ColorOffset initialized to %d\n", tidata));
    	    retval = 1;
    	    break;
    	    
    	case AROSA_Palette_IndicatorWidth:	/* [I] */
    	    data->pd_IndWidth = (UWORD)tidata;
    	    D(bug("Indicatorwidth set to %d\n", tidata));
    	    break;

    	case AROSA_Palette_IndicatorHeight:	/* [I] */
    	    data->pd_IndHeight = (UWORD)tidata;
    	    D(bug("Indicatorheight set to %d\n", tidata));
    	    break;
    	    
    	    
    	case AROSA_Palette_ColorTable:
    	    D(bug(" !!!!! ColorTable attribute not implemented (yet) !!!!!\n"));
    	    break;
    	    
    	case AROSA_Palette_NumColors:
    	    D(bug(" !!!!! NumColors attribute not implemented (yet) !!!!!\n"));
    	    break;

    	} /* switch (tag->ti_Tag) */
    	
    } /* for (each attr in attrlist) */
    
    ReturnPtr ("Palette::Set", IPTR, retval);
}

/*********************
**  Palette::New()  **
*********************/
STATIC Object *palette_new(Class *cl, Object *o, struct opSet *msg)
{

    struct opSet ops;
    struct TagItem tags[] =
    {
	{GA_RelSpecial, TRUE},
	{TAG_MORE, NULL}
    };

    EnterFunc(bug("Palette::New()\n"));
    
    tags[1].ti_Data = (IPTR)msg->ops_AttrList;
    
    ops.MethodID	= OM_NEW;
    ops.ops_GInfo	= NULL;
    ops.ops_AttrList	= &tags[0];
 
    o = (Object *)DoSuperMethodA(cl, o, (Msg)&ops);
    if (o)
    {
    	struct PaletteData *data = INST_DATA(cl, o);
    	
    	/* Set some defaults */
    	data->pd_Depth		= 1;
    	data->pd_Color		= 0;
    	data->pd_OldColor	= 0 ; /* = data->pd_OldColor */
    	data->pd_ColorOffset	= 0;
    	data->pd_IndWidth 	= 0;
    	data->pd_IndHeight	= 0;
    	
    	palette_set(cl, o, msg);
    
    }
    ReturnPtr ("Palette::New", Object *, o);
}

/************************
**  Palette::Render()  **
************************/
STATIC VOID palette_render(Class *cl, Object *o, struct gpRender *msg)
{
    struct PaletteData *data = INST_DATA(cl, o);
    
    UWORD *pens = msg->gpr_GInfo->gi_DrInfo->dri_Pens;
    struct RastPort *rp;
    
    EnterFunc(bug("Palette::Render()\n"));    
    
    D(bug("Window: %s\n", msg->gpr_GInfo->gi_Window->Title));
    
    rp = msg->gpr_RPort;
    
    
    switch (msg->gpr_Redraw)
    {
    	case GREDRAW_REDRAW:
    	    D(bug("Doing total redraw\n"));
    	    RenderPalette(data, pens, rp, AROSPaletteBase);
    
    	case GREDRAW_UPDATE:
     	    D(bug("Doing redraw update\n"));
    	    
    	    UpdateActiveColor(data, pens, rp, AROSPaletteBase);
    	
    	    if (data->pd_IndWidth || data->pd_IndHeight)
    	    {
    	    	struct IBox *ibox = &(data->pd_IndicatorBox);
    	    	SetAPen(msg->gpr_RPort, pens[data->pd_Color]);
    	    	RectFill(msg->gpr_RPort, ibox->Left, ibox->Top,
    	    	    ibox->Left + ibox->Width - 1, ibox->Top + ibox->Height - 1);

    	    }
    	    break;
    	    
    } /* switch (redraw method) */
    	
    ReturnVoid("Palette::Render");
}

/***************************
**  Palette::GadgetHit()  **
***************************/
STATIC IPTR palette_hittest(Class *cl, Object *o, struct gpHitTest *msg)
{

    IPTR retval = 0UL;
    struct PaletteData *data = INST_DATA(cl, o);
    
    EnterFunc(bug("Palette::HitTest()\n"));

    /* One might think that this method is not necessary to implement,
    ** but here is an example to show the opposite:
    ** Consider a 16 color palette with 8 rows and 2 cols.
    ** Gadget is 87 pix. heigh. Each row will then be 10 pix hight + 7 pix
    ** of "nowhere". To prevent anything from happening when this area is
    ** clicked, we rule it out here.
    */
    if (InsidePalette(&(data->pd_PaletteBox), msg->gpht_Mouse.X, msg->gpht_Mouse.Y))
    	retval = GMR_GADGETHIT;
    	
    ReturnInt ("Palette::HitTest", IPTR, retval);
}

/************************
**  Palette::GoActive  **
************************/
STATIC IPTR palette_goactive(Class *cl, Object *o, struct gpInput *msg)
{
    IPTR retval = 0UL;
    struct PaletteData *data = INST_DATA(cl, o);

    EnterFunc(bug("Palette::GoActive()\n"));

    if (msg->gpi_IEvent)
    {
    	UBYTE clicked_color;
    	
    	/* Set temporary active to the old active */
    	data->pd_ColorBackup = data->pd_Color;
    	
    	clicked_color = ComputeColor(data, msg->gpi_Mouse.X, msg->gpi_Mouse.Y, AROSPaletteBase);
    	
    	if (clicked_color != data->pd_Color)
    	{
    	    struct RastPort *rp;
    	    
    	    data->pd_Color = clicked_color;
    	
    	    if ((rp = ObtainGIRPort(msg->gpi_GInfo)))
    	    {
	    	DoMethod(o, GM_RENDER, msg->gpi_GInfo, rp, GREDRAW_UPDATE);
	    
	    	ReleaseGIRPort(rp);
	    }
	}
	
	retval = GMR_MEACTIVE;
    }
    else
    	retval = GMR_NOREUSE;
    ReturnInt("Palette::GoActive", IPTR, retval);
}



/*****************************
**  Palette::HandleInput()  **
*****************************/
STATIC IPTR palette_handleinput(Class *cl, Object *o, struct gpInput *msg)
{
    struct PaletteData *data = INST_DATA(cl, o);
    IPTR retval = 0UL;
    struct InputEvent *ie = msg->gpi_IEvent;
    
    EnterFunc(bug("Palette::HandleInput\n"));
    
    retval = GMR_MEACTIVE;
    
    if (ie->ie_Class == IECLASS_RAWMOUSE)
    {
    	
    	switch (ie->ie_Code)
    	{
    	    case SELECTUP: {
		/* If the button was released outside the gadget, then
		** go back to old state
		*/

		D(bug("IECLASS_RAWMOUSE: SELECTUP\n"));
		     
		if (!InsidePalette(&(data->pd_PaletteBox), msg->gpi_Mouse.X, msg->gpi_Mouse.Y))
    	    	{
    	    	    struct RastPort *rp;
    	    	     	
    	    	    /* Left released outside of gadget area, go back
    	    	    ** to old state
    	    	    */
    	    	    data->pd_Color = data->pd_ColorBackup;
    	    	    D(bug("Left released outside gadget\n"));

    	    	    if ((rp = ObtainGIRPort(msg->gpi_GInfo)))
    	    	    {
    	    	     	DoMethod(o, GM_RENDER, msg->gpi_GInfo, rp, GREDRAW_UPDATE);
    	    	     	    
    	    	     	ReleaseGIRPort(rp);
    	    	    }
    	    	}
    	    	else
    	    	{
    	    	    D(bug("Left released inside gadget, color=%d\n", data->pd_Color));
    	    	    *(msg->gpi_Termination) = data->pd_Color;
    	    	    retval = GMR_VERIFY;
    	    	}
    	    	
    	    	retval |= GMR_NOREUSE;
    	    } break;
    	    	
    	    case IECODE_NOBUTTON: {

    	    	UBYTE over_color;
    	    
    	    	D(bug("IECLASS_POINTERPOS\n"));
    	    	
    	    	if (InsidePalette(&(data->pd_PaletteBox), msg->gpi_Mouse.X, msg->gpi_Mouse.Y))
    	    	{
    	    	
    	    	    over_color = ComputeColor(data, msg->gpi_Mouse.X, msg->gpi_Mouse.Y, AROSPaletteBase);
    	    
    	   	    if (over_color != data->pd_Color)
    	    	    {
    	    	    	struct RastPort *rp;

    	    	    	data->pd_Color = over_color;
    	    	    	if ((rp = ObtainGIRPort(msg->gpi_GInfo)))
    	    	    	{
	    	    	    DoMethod(o, GM_RENDER, msg->gpi_GInfo, rp, GREDRAW_UPDATE);
	    
	    	    	    ReleaseGIRPort(rp);
	    	    	}
    	    	    } /* if (mouse is over a different color) */
    	    	    
    	    	} /* if (mouse is over gadget) */
    	    
    	    	retval = GMR_MEACTIVE;
    	    
    	    } break;
    	

    	    case MENUDOWN: {

    	    	/* Right pushed on gadget, go back to old state */
    	    	
    	    	struct RastPort *rp;
    	    	
    	    	data->pd_Color = data->pd_ColorBackup;
    	    	D(bug("Right mouse pushed \n"));

    	    	if ((rp = ObtainGIRPort(msg->gpi_GInfo)))
    	    	{
    	    	    DoMethod(o, GM_RENDER, msg->gpi_GInfo, rp, GREDRAW_UPDATE);
    	    	     	    
    	    	    ReleaseGIRPort(rp);
    	    	}

    	    	retval = GMR_REUSE;
    	    } break;
    	    	
    	} /* switch (ie->ie_Code) */
    	
    } /* if (mouse event) */
    
    ReturnInt("Palette::HandleInput", IPTR, retval);
}

/************************
**  Palette::Layout()  **
************************/
STATIC VOID palette_layout(Class *cl, Object *o, struct gpLayout *msg)
{
    
    /* The palette gadget has been resized and we need to update our layout */
    
    struct PaletteData *data = INST_DATA(cl, o);
    struct IBox *pbox = &(data->pd_PaletteBox), *indbox = &(data->pd_IndicatorBox);
    
    UWORD cols, rows;

    WORD factor1, factor2, ratio;
    UWORD fault, smallest_so_far;
    
    UWORD *largest, *smallest;
    
    WORD leftover_width, leftover_height;
    
    EnterFunc(bug("Palette::Layout()\n"));
    
    if (!msg->gpl_GInfo)
    	return; /* We MUST have a GInfo to get screen aspect ratio */
    
    /* Delete the old gadget box */
    if (!msg->gpl_Initial)
    {

    	struct RastPort *rp;    
    
    	if ((rp = ObtainGIRPort(msg->gpl_GInfo)))
    	{
    	    SetAPen(rp, msg->gpl_GInfo->gi_DrInfo->dri_Pens[BACKGROUNDPEN]);
    	    
    	    RectFill(rp, pbox->Left, pbox->Top, 
    	    	pbox->Left + pbox->Width - 1, pbox->Top + pbox->Height - 1);
    	}
    }

    /* get the IBox surrounding the whole palette */
    GetGadgetIBox(o, msg->gpl_GInfo, pbox);
    
    D(bug("Got palette ibox: (%d, %d, %d, %d)\n",
    	pbox->Left, pbox->Top, pbox->Width, pbox->Height));
    	
    /* If we have an indicator box then account for this */
    if (data->pd_IndHeight)
    {
    	indbox->Top 	= (pbox->Top += data->pd_IndHeight);
    	indbox->Left	= pbox->Left;
    	indbox->Width	= pbox->Width;
    	indbox->Height	= data->pd_IndHeight;
    }
    else if (data->pd_IndWidth)
    {
    	indbox->Left	= (pbox->Left += data->pd_IndWidth);
    	indbox->Top	= pbox->Top;
    	indbox->Width	= data->pd_IndWidth;
    	indbox->Height	= pbox->Height;
    }

    
    /* Compute initial aspect ratio */
    if (pbox->Width > pbox->Height)
    {
    	cols = pbox->Width / pbox->Height;
    	rows = 1;
    	largest  = &cols;
    	smallest = &rows;
    }
    else
    {
    	rows = pbox->Height / pbox->Width;
    	cols = 1;
    	largest  = &rows;
    	smallest = &cols;
    }

    D(bug("Biggest aspect: %d\n", *largest));
    
    ratio = *largest;
    
    smallest_so_far = 0xFFFF;
    
    factor1 = 1 << data->pd_Depth;
    factor2 = 1;
    
    while (factor1 >= factor2)
    {

    	D(bug("trying aspect %dx%d\n", factor1, factor2));

    	fault = abs(ratio - (factor1 / factor2));
    	D(bug("Fault: %d, smallest fault so far: %d\n", fault, smallest_so_far));

    	if (fault < smallest_so_far)
    	{
    	     *largest  = factor1;
    	     *smallest = factor2;
    	     
    	     smallest_so_far = fault;
    	}

	factor1 >>= 1;
	factor2 <<= 1;
    		
    }
    
    data->pd_NumCols = (UBYTE)cols;
    data->pd_NumRows = (UBYTE)rows;
    
    data->pd_ColWidth  = pbox->Width  / data->pd_NumCols;
    data->pd_RowHeight = pbox->Height / data->pd_NumRows;
    
    D(bug("cols=%d, rows=%d\n", data->pd_NumCols, data->pd_NumRows));
    D(bug("colwidth=%d, rowheight=%d\n", data->pd_ColWidth, data->pd_RowHeight));
    
    /* Adjust the pbox's and indbox's height according to leftovers */
    
    leftover_width  = pbox->Width  % data->pd_NumCols;
    leftover_height = pbox->Height % data->pd_NumRows;

    pbox->Width  -= leftover_width;
    pbox->Height -= leftover_height;

    if (data->pd_IndHeight)
    	indbox->Width -= leftover_width;
    else if (data->pd_IndWidth)
    	indbox->Height -= leftover_height;
    
    ReturnVoid("Palette::Layout");
}


/* paletteclass boopsi dispatcher
 */
AROS_UFH3(STATIC IPTR, dispatch_paletteclass,
    AROS_UFHA(Class *,  cl,  A0),
    AROS_UFHA(Object *, o,   A2),
    AROS_UFHA(Msg,      msg, A1)
)
{
    IPTR retval = 0UL;
    
    switch(msg->MethodID)
    {
	case OM_NEW:
	    retval = (IPTR)palette_new(cl, o, (struct opSet *)msg);
	    break;
	
	case GM_RENDER:
	    palette_render(cl, o, (struct gpRender *)msg);
	    break;
	    
	case GM_LAYOUT:
	    palette_layout(cl, o, (struct gpLayout *)msg);
	    break;
	    
	case GM_HITTEST:
	    retval = palette_hittest(cl, o, (struct gpHitTest *)msg);
	    break;
	    
	case GM_GOACTIVE:
	    retval = palette_goactive(cl, o, (struct gpInput *)msg);
	    break;

	case GM_HANDLEINPUT:
	    retval = palette_handleinput(cl, o, (struct gpInput *)msg);
	    break;

	case OM_SET:
	case OM_UPDATE:
	    retval = DoSuperMethodA(cl, o, msg);
	    retval += (IPTR)palette_set(cl, o, (struct opSet *)msg);
	    /* If we have been subclassed, OM_UPDATE should not cause a GM_RENDER
	     * because it would circumvent the subclass from fully overriding it.
	     * The check of cl == OCLASS(o) should fail if we have been
	     * subclassed, and we have gotten here via DoSuperMethodA().
	     */
	    if (    retval
	    	 && (msg->MethodID == OM_UPDATE)
	    	 && (cl == OCLASS(o))	)
	    {
	    	struct GadgetInfo *gi = ((struct opSet *)msg)->ops_GInfo;
	    	if (gi)
	    	{
		    struct RastPort *rp = ObtainGIRPort(gi);
		    if (rp)
		    {


		        DoMethod(o, GM_RENDER, gi, rp, GREDRAW_UPDATE);
		        ReleaseGIRPort(rp);

		    } /* if */
	    	} /* if */
	    } /* if */

	    break;

	    
	default:
	    retval = DoSuperMethodA(cl, o, msg);
	    break;
	    
    } /* switch */

    return (retval);
}  /* dispatch_paletteclass */


#undef AROSPaletteBase

/****************************************************************************/

/* Initialize our palette class. */
struct IClass *InitPaletteClass (struct PaletteBase_intern * AROSPaletteBase)
{
    struct IClass *cl = NULL;

    /* This is the code to make the paletteclass...
     */
    if ((cl = MakeClass(AROSPALETTECLASS, GADGETCLASS, NULL, sizeof(struct PaletteData), 0)))
    {
	cl->cl_Dispatcher.h_Entry    = (APTR)AROS_ASMSYMNAME(dispatch_paletteclass);
	cl->cl_Dispatcher.h_SubEntry = NULL;
	cl->cl_UserData 	     = (IPTR)AROSPaletteBase;

	AddClass (cl);
    }

    return (cl);
}

