#if !defined(AROSMESA_INTERN_H)
#define AROSMESA_INTERN_H

#include "mtypes.h"
#include <GL/AROSMesa.h>

/* TrueColor (RGBA) to ColorFrmt Macros */
#define     TC_ARGB(r, g, b, a)           ((((((a << 8) | r) << 8) | g) << 8) | b)
#define     TC_RGBA(r, g, b, a)           ((((((r << 8) | g) << 8) | b) << 8) | a)
#define     PACK_8B8G8R( r, g, b )        (((((b << 8) | g) << 8) | r) << 8)

/*
 * Convert Mesa window coordinate(s) to AROS screen coordinate(s):
 */

#define FIXx(x) (amesa->left + (x))

#define FIXy(y) (amesa->RealHeight - amesa->bottom - (y))

#define FIXxy(x,y) ((amesa->RealWidth * FIXy(y) + FIXx(x)))



/* Structs */



/* AROS render buffer */
struct arosmesa_renderbuffer
{
    struct gl_renderbuffer      Base;               /* Base class - must be first */
    UBYTE *                     imageline;          /* One Line for WritePixelRow renders */
    struct RastPort *           rp;                 /* RastPort to draw over */
    GLboolean                   ownsrastport;       /* GL_TRUE is rastport is owned (can be deleted) by renderbuffer */
};

typedef struct arosmesa_renderbuffer * AROSMesaRenderBuffer;

#define GET_GL_RB_PTR(arosmesa_rb) &arosmesa_rb->Base
#define GET_AROS_RB_PTR(gl_rb) (AROSMesaRenderBuffer)gl_rb

/* AROS frame buffer */
struct arosmesa_framebuffer
{
    struct gl_framebuffer Base;     /* Base class - must be first */
};

typedef struct arosmesa_framebuffer * AROSMesaFrameBuffer;

#define GET_GL_FB_PTR(arosmesa_fb) &arosmesa_fb->Base
#define GET_AROS_FB_PTR(gl_fb) (AROSMesaFrameBuffer)gl_fb

/* AROS visual */
struct arosmesa_visual
{
    GLvisual Base;                  /* Base class - must be first */
    /* TODO use dbflag from GLvisual */
    GLboolean               db_flag;          /* double buffered?   */
};

typedef struct arosmesa_visual * AROSMesaVisual;

#define GET_GL_VIS_PTR(arosmesa_vis) &arosmesa_vis->Base
#define GET_AROS_VIS_PTR(gl_vis) (AROSMesaVisual)gl_vis

/* AROS context */

struct arosmesa_context
{
    GLcontext               Base;                   /* GLcontext* - the core library context */

    AROSMesaVisual          visual;                 /* the visual context */
    AROSMesaFrameBuffer     framebuffer;            /* frame buffer */
    AROSMesaRenderBuffer    front_rb;               /* front render buffer */
    AROSMesaRenderBuffer    back_rb;                /* back render buffer */

    ULONG                    clearpixel;            /* pixel for clearing the color buffers */

    /* etc... */
    struct Window           *window;                /* Intuition window */
    struct RastPort         *visible_rp;            /* Visible rastport */
    struct Screen           *Screen;                /* Current screen*/

    GLuint                  depth;                  /* bits per pixel (1, 8, 24, etc) */

    GLuint                  width, height;          /* drawable area */
    GLuint                  top, bottom;            /* offsets due to window border */
    GLuint                  left, right;            /* offsets due to window border */
    GLuint                  RealWidth,RealHeight;   /* the drawingareas real size*/
    GLuint                  FixedWidth,FixedHeight; /* The internal buffer real size speeds up the drawing a bit*/

    /* Internal Functions */
    void (*SwapBuffer)(struct arosmesa_context *);  /* Use this when AROSMesaSwapBuffers is called */
};

/* typedef struct arosmesa_context * AROSMesaContext; */ /* Defined in AROSMesa.h */

#define GET_GL_CTX_PTR(arosmesa_ctx) &arosmesa_ctx->Base
#define GET_AROS_CTX_PTR(gl_ctx) (AROSMesaContext)gl_ctx

#endif /* AROSMESA_INTERN_H */
