#ifndef AROS_MACHINE_H
#define AROS_MACHINE_H
/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    NOTE: This file must compile *without* any other header !

    Desc: machine.h include file for Linux/i386 and FreeBSD/i386
    Lang: english
*/

#define EnableSetFunction	1

/* Information generated by machine.c */
#define AROS_STACK_GROWS_DOWNWARDS 1 /* Stack direction */
#define AROS_BIG_ENDIAN 	   0 /* Big or little endian */
#define AROS_SIZEOFULONG	   4 /* Size of an ULONG */
#define AROS_WORDALIGN		   2 /* Alignment for WORD */
#define AROS_LONGALIGN		   4 /* Alignment for LONG */
#define AROS_PTRALIGN		   4 /* Alignment for PTR */
#define AROS_IPTRALIGN		   4 /* Alignment for IPTR */
#define AROS_DOUBLEALIGN	   4 /* Alignment for double */
#define AROS_WORSTALIGN 	   4 /* Worst case alignment */

#define SIZEOF_FPU_STATE	108  /* 108 bytes are needed to store FPU */
#define SIZEOF_ALL_REGISTERS	(15*4 + SIZEOF_FPU_STATE)  /* Size of iet_Context */

#define AROS_GET_SYSBASE	struct ExecBase * SysBase = *(struct ExecBase **)0x4;

/* ??? */
#define SP_OFFSET 0

/*
    One entry in a libraries' jumptable. For assembler compatibility, the
    field jmp should contain the code for an absolute jmp to a 32bit
    address. There are also a couple of macros which you should use to
    access the vector table from C.
*/
struct FullJumpVec
{
    unsigned char jmp;
    unsigned char vec[4];
};
#define __AROS_SET_FULLJMP(v,a)         (struct FullJumpVec *)v->jmp = 0xE9,        \
					(*(ULONG*)(v)->vec=(ULONG)(a)-(ULONG)(v)-5))
struct JumpVec
{
    unsigned char vec[4];
};

#define __AROS_SET_VEC(v,a)             (*(ULONG*)(v)->vec=(ULONG)(a))
#define __AROS_GET_VEC(v)               ((APTR)(*(ULONG*)(v)->vec))

/* Use these to acces a vector table */
#define LIB_VECTSIZE			(sizeof (struct JumpVec))
#define __AROS_GETJUMPVEC(lib,n)        (&((struct JumpVec *)lib)[-(n)])
#define __AROS_GETVECADDR(lib,n)        (__AROS_GET_VEC(__AROS_GETJUMPVEC(lib,n)))
#define __AROS_SETVECADDR(lib,n,addr)   (__AROS_SET_VEC(__AROS_GETJUMPVEC(lib,n),(APTR)(addr)))
#define __AROS_INITVEC(lib,n)           __AROS_SETVECADDR(lib,n,_aros_not_implemented)

/*
   Code to use to generate stub functions.
   It must be *printed* with a function like printf in a file
   to be compiled with gcc.

   - The first parameter is the function name,
   - The second parameter is the basename,
   - The third parameter is the library vector to be called.
     It's value must be computed by the stub generator with this code:
     &(__AROS_GETJUMPVEC(0, n+1)->vec), where n is the library vector position in
     the library vectors list.

*/

#define STUBCODE                                       \
		"#define EMITSTUB(fname, bname, vec) " \
		".globl fname ; "                      \
		"fname : "                             \
		"movl bname , %%eax; "                 \
		"jmp *vec(%%eax);\n"                   \
		"EMITSTUB(%s, %s, %d) "

/*
   We want to activate the execstubs and preserve all registers
   when calling obtainsemaphore, obtainsemaphoreshared, releasesemaphore,
   getcc, permit, forbid, enable, disable
*/
#undef UseExecstubs
#define UseExecstubs 1

/* For debugging only: Pass errnos from the emulated OS. dos/Fault() will
   recognise them */
#undef PassThroughErrnos
#define PassThroughErrnos 0x40000000

/* Macros to test/set failure of AllocEntry() */
#define AROS_ALLOCENTRY_FAILED(memType) \
	((struct MemList *)((IPTR)(memType) | 0x80ul<<(sizeof(APTR)-1)*8))
#define AROS_CHECK_ALLOCENTRY(memList) \
	(!((IPTR)(memList) & 0x80ul<<(sizeof(APTR)-1)*8))

/*
    Find the next valid alignment for a structure if the next x bytes must
    be skipped.
*/
#define AROS_ALIGN(x)        (((x)+AROS_WORSTALIGN-1)&-AROS_WORSTALIGN)

/* Prototypes */
extern void _aros_not_implemented ();
extern void aros_not_implemented ();

/*
    How much stack do we need ? Lots :-) ?
    Not so much, I think (schulz) ;-))
*/

#define AROS_STACKSIZE	40960

/* How to map function arguments to CPU registers */
/*
    The i386 processor doesn't have enough registers to map the m68k
    register set onto them - so simply use the compiler's calling
    convention. The library base is mapped to the last argument so that
    it can be ignored by the function.
*/

/* What to do with the library base in header, prototype and call */
#define __AROS_LH_BASE(basetype,basename)   basetype basename
#define __AROS_LP_BASE(basetype,basename)   void *
#define __AROS_LC_BASE(basetype,basename)   basename
#define __AROS_LD_BASE(basetype,basename)   basetype

/* How to transform an argument in header, opt prototype, call and forced
   prototype. */
#define __AROS_LHA(type,name,reg)     type name
#define __AROS_LPA(type,name,reg)     type
#define __AROS_LCA(type,name,reg)     name
#define __AROS_LDA(type,name,reg)     type
#define __AROS_UFHA(type,name,reg)    type name
#define __AROS_UFPA(type,name,reg)    type
#define __AROS_UFCA(type,name,reg)    name
#define __AROS_UFDA(type,name,reg)    type
#define __AROS_LHAQUAD(type,name,reg1,reg2)     type name
#define __AROS_LPAQUAD(type,name,reg1,reg2)     type
#define __AROS_LCAQUAD(type,name,reg1,reg2)     name

/* Prefix for library function in header, prototype and call */
#define __AROS_LH_PREFIX    /* eps */
#define __AROS_LP_PREFIX    /* eps */
#define __AROS_LC_PREFIX    /* eps */
#define __AROS_LD_PREFIX    /* eps */
#define __AROS_UFH_PREFIX   /* eps */
#define __AROS_UFP_PREFIX   /* eps */
#define __AROS_UFC_PREFIX   /* eps */
#define __AROS_UFD_PREFIX   /* eps */

/* if this is defined, all AROS_LP*-macros will expand to nothing. */
#define __AROS_USE_MACROS_FOR_LIBCALL

#define __UFC3R(t,n,t1,n1,r1,t2,n2,r2,t3,n3,r3,p) \
({\
    long _n1 = (long)(n1);\
    long _n2 = (long)(n2);\
    long _n3 = (long)(n3);\
    long _re;\
    __asm__ __volatile__(\
	"movl   %5,%%eax\n\t"\
	"pushl  %%eax\n\t"\
	"movl   %4,%%eax\n\t"\
	"pushl  %%eax\n\t"\
	"movl   %3,%%eax\n\t"\
	"pushl  %%eax\n\t"\
	"movl   %%esp,%1\n\t"\
	"movl   %2,%%eax\n\t"\
	"call   *%%eax\n\t"\
	"lea    12(%%esp),%%esp\n\t"\
	"movl   %%eax,%0"\
	: "=g"(_re), "=m"(*(APTR *)p)\
	: "ad"(n), "g"(_n1), "g"(_n2), "g"(_n3)\
	: "cc", "memory", "%eax" );\
    (t)_re;\
})
#define AROS_UFC3R(t,n,a1,a2,a3,p,ss) __UFC3R(t,n,a1,a2,a3,p)

#endif /* AROS_MACHINE_H */
