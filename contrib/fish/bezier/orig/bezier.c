
/*
 *  BEZIER.C
 *
 *  Matthew Dillon.
 *  Public Domain (no Copyrights whatsoever)
 *
 *  -Assumes AZTEC compilation,  +L (32 bit ints), with all AMIGA symbols
 *  precompiled.  Additionally expects certain typedefs and routines
 *  found in MY.LIB, as well as some MY.LIB #include files.
 *
 *  An experienced programmer can remove the MY.LIB dependancies
 *  (openlibs() call), and figure out what typedefs have been assumed if
 *  he wishes to compile the program.  You can also simply extract the
 *  Bezier functions for your own use.
 */

/* !!!
 #include <typedefs.h>
*/ 
#include <aros/oldprograms.h>
#include <intuition/intuition.h>
#include <graphics/gfx.h>
typedef struct Window WIN;
typedef struct NewWindow NW;
typedef struct IntuiMessage IMESS;
typedef struct RastPort RP;
typedef struct Gadget GADGET;
struct GfxBase * GfxBase;
struct IntuitionBase * IntuitionBase;
/* !!!
 #include <xmisc.h>
*/

#define     SHIFTS  9
#define     ONE     (1<<SHIFTS)

typedef struct PropInfo XPI;
typedef struct Image	IM;

/* !!!
 extern IMESS *GetMsg();
*/

#define MYGADGETS   (WINDOWSIZING|WINDOWDRAG|WINDOWDEPTH|WINDOWCLOSE)

NW Nw = {
    64, 64, 320, 100,
    0, 1,
    NEWSIZE|MOUSEBUTTONS|MOUSEMOVE|CLOSEWINDOW|GADGETDOWN|GADGETUP,
    MYGADGETS|REPORTMOUSE|ACTIVATE|NOCAREREFRESH,
    0, 0, (UBYTE *)"Bezier", NULL, NULL,
    32, 64, -1, -1, WBENCHSCREEN
};

#define INTUITION_LIB 1
#define GRAPHICS_LIB 2
int openlibs(int dummy)
{
  GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0);
  IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library",0);
}

void closelibs(int dummy)
{
  if (GfxBase)
    CloseLibrary((struct Library *)GfxBase);
  if (IntuitionBase)
    CloseLibrary((struct Library *)IntuitionBase);
}

WIN *Win;
RP  *Rp;
short Ux, Uy, Lx, Ly;
short Step = 128;

main(ac, av)
char *av[];
{
    register IMESS *mess;
    short notdone = 1;
    short pt = -1;
    short ptarray[4][2];

    short gy, gg = 0;
    XPI *po;

    init_gadgets(&Nw, &po);
    exiterr(!openlibs(INTUITION_LIB|GRAPHICS_LIB), "unable to open libs");
    exiterr(!(Win = (struct Window *)OpenWindow(&Nw)), "unable to open window");
    Rp = Win->RPort;
    SetAPen(Rp, 1);
    setpoint(ptarray, 0, 32, 32);
    setpoint(ptarray, 1, 40, 40);
    setpoint(ptarray, 2, 50, 50);
    setpoint(ptarray, 3, 60, 60);
    setbounds(ptarray);
    while (notdone) {
	short mx, my, mm = 0;
	WaitPort(Win->UserPort);
	while (mess = (struct IntuiMessage *)GetMsg(Win->UserPort)) {
	    switch(mess->Class) {
	    case CLOSEWINDOW:
		notdone = 0;
		break;
	    case NEWSIZE:
		setbounds(ptarray);
		break;
	    case GADGETUP:
	    case GADGETDOWN:
		{
		    gg = mess->Class;
		    gy = po->VertPot / 256;
		}
		break;
	    case MOUSEBUTTONS:
		switch(mess->Code) {
		case SELECTDOWN:
		    pt = getpoint(ptarray, mess->MouseX, mess->MouseY);
		    break;
		case SELECTUP:
		    pt = -1;
		    break;
		}
		break;
	    case MOUSEMOVE:
		if (gg == GADGETDOWN) {
		    gy = po->VertPot / 256;
		    break;
		}
		mm = 1;
		mx = mess->MouseX;
		my = mess->MouseY;
		break;
	    default:
		break;
	    }
	    ReplyMsg((struct Message *)	mess);
	}
	if (mm && pt >= 0) {
	    movepoint(ptarray, pt, mx, my);
	    drawcurve(ptarray);
	}
	if (gg) {
	    char buf[32];
	    if (gg == GADGETUP)
		gg = 0;
	    if (gy + 1 >= 0)
		Step = gy + 1;
	    sprintf(buf, "gran: %4ld/%ld", Step, ONE);
	    drawcurve(ptarray);
	    Move(Rp, Ux + 1, Uy + 16);
	    Text(Rp, buf, strlen(buf));
	}
    }
    exiterr(1, NULL);
}

exiterr(n, str)
char *str;
{
    if (n) {
	if (str)
	    puts(str);
	if (Win)
	    CloseWindow(Win);
        closelibs(-1);
	exit(1);
    }
}

setbounds(a)
register long *a;
{
    Ux = Win->BorderLeft;
    Uy = Win->BorderTop;
    Lx = Win->Width - Win->BorderRight;
    Ly = Win->Height- Win->BorderBottom;
    drawcurve(a);
}

setpoint(a, pt, x, y)
register short a[4][2];
{
    a[pt][0] = x;
    a[pt][1] = y;
    drawpoints(a, pt, pt + 1);
}

getpoint(a, x, y)
register short a[4][2];
{
    register short i, bi;
    register long r, br;

    for (i = bi = 0, br = 0x7FFFFFFF; i < 4; ++i) {
	r = (x-a[i][0])*(x-a[i][0]) + (y-a[i][1])*(y-a[i][1]);
	if (r < br) {
	    bi = i;
	    br = r;
	}
    }
    return(bi);
}

movepoint(a, pt, x, y)
register short a[4][2];
{
    SetAPen(Rp, 0);
    drawpoints(a, pt, pt + 1);
    SetAPen(Rp, 1);
    setpoint(a, pt, x, y);
}

#define S10(x)	 ((x) >> SHIFTS)
#define S20(x)	 ((x) >> (2*SHIFTS))

/*
 *  So I can use integer arithmatic, I am defining 512 as 1 (as far
 *  as the mathematics go), which means that I must divide any power
 *  multiplication by 512^(n-1).  E.G. .5^2 = .25 ... to make 256^2
 *  equal 128, I must divide by 512^1
 */

drawcurve(a)
register short a[4][2];
{
    long  m1[4];	/* t matrix		    */
    long  mr[4];	/* partial result matrix    */
    long  corr[2];
    register short t, i;
    char  lastpt = 0;

    SetAPen(Rp, 0);
    RectFill(Rp, Ux, Uy, Lx - 1, Ly - 1);
    SetAPen(Rp, 1);
    drawpoints(a, 0, 4);

    Move(Rp, a[0][0], a[0][1]);
    for (t = 0; t <= ONE; t += Step) {	   /*  t = 0 to 1      */
oncemore:
	m1[3] = ONE;
	m1[2] = t;
	m1[1] = t * t;
	m1[0] = m1[1] * t;

	mr[0] = -S20(m1[0]  ) + S10(3*m1[1]) - 3*m1[2] + m1[3];
	mr[1] =  S20(3*m1[0]) - S10(6*m1[1]) + 3*m1[2];
	mr[2] = -S20(3*m1[0]) + S10(3*m1[1]);
	mr[3] =  S20(m1[0]  );
	for (i = 0; i < 2; ++i) {
	    corr[i] = (mr[0] * a[0][i] + mr[1] * a[1][i] +
		      mr[2] * a[2][i] + mr[3] * a[3][i]) >> SHIFTS;
	}
	Draw(Rp, corr[0], corr[1]);
    }
    if (lastpt == 0 && t - Step < ONE) {
	lastpt = 1;
	t = ONE;
	goto oncemore;
    }
}

drawpoints(a, is, ie)
register short a[4][2];
{
    register short i;
    for (i = is; i < ie; ++i) {
	Move(Rp, a[i][0] - 2, a[i][1]);
	Draw(Rp, a[i][0] + 2, a[i][1]);
	Move(Rp, a[i][0], a[i][1] - 2);
	Draw(Rp, a[i][0], a[i][1] + 2);
    }
}

/*
 *  GADGET ROUTINES!
 */


#define NG(nn)	&Gadgets[nn+1]
#define G_YGLOB 1
#define G_XGLOB 2

XPI Props[] = {
    { AUTOKNOB|FREEVERT , 0, 0, 0x1FFF, 0x1FFF }
};

IM Images[] = {
    { 0,0,2,1,1, NULL, 1, 0, NULL },
};

GADGET Gadgets[] = {
    {
	NULL, -15, 11, 15, -19, GADGIMAGE|GADGHCOMP|GRELRIGHT|GRELHEIGHT,
	GADGIMMEDIATE|RIGHTBORDER|RELVERIFY,PROPGADGET,
	(APTR)&Images[0],NULL,NULL,0,(APTR)&Props[0], G_YGLOB, 0
    },
};

GADGET *Gc;
long GUx, GUy;

init_gadgets(nw, ppo)
NW *nw;
XPI **ppo;
{
    nw->FirstGadget = &Gadgets[0];
    *ppo = &Props[0];
}


