#    (C) 1995-96 AROS - The Amiga Replacement OS
#    $Id$
#    $Log$
#    Revision 1.5  1996/09/11 16:54:27  digulla
#    Always use __AROS_SLIB_ENTRY() to access shared external symbols, because
#    	some systems name an external symbol "x" as "_x" and others as "x".
#    	(The problem arises with assembler symbols which might differ)
#
#    Revision 1.4  1996/08/23 16:49:21	digulla
#    With some systems, .align 16 aligns to 64K instead of 16bytes. Therefore
#	I replaced it with .balign which does what we want.
#
#    Revision 1.3  1996/08/13 14:03:19	digulla
#    Added standard headers
#
#    Revision 1.2  1996/08/01 17:41:10	digulla
#    Added standard header for all files
#
#    Desc:
#    Lang:

#*****************************************************************************
#
#   NAME
#	__AROS_LH0(void, Exception,
#
#   LOCATION
#	struct ExecBase *, SysBase, 8, Exec)
#
#   FUNCTION
#
#   INPUTS
#
#   RESULT
#
#   NOTES
#
#   EXAMPLE
#
#   BUGS
#
#   SEE ALSO
#
#   INTERNALS
#
#   HISTORY
#
#******************************************************************************
	Disable     =	-100
	Enable	    =	-105
	ThisTask    =	284
	IDNestCnt   =	302
	tc_Flags    =	16
	tc_SigRecvd =	28
	tc_SigExcept=	32
	tc_ExceptData=	40
	tc_ExceptCode=	44
	TF_EXCEPT   =	32
	.text
	.balign 16
	.globl	_Exec_Exception
	.type	_Exec_Exception,@function
_Exec_Exception:
	/* Get SysBase amd pointer to current task */
	movl	4(%esp),%ebp
	movl	ThisTask(%ebp),%edi

	/* Clear exception flag */
	andb	$~TF_EXCEPT,tc_Flags(%edi)

	/* If the exception is raised out of Wait IDNestCnt may be >0 */
	movb	IDNestCnt(%ebp),%ebx
	/* Set it to a defined value */
	movb	$0,IDNestCnt(%ebp)

exloop: /* Get mask of signals causing the exception */
	movl	tc_SigExcept(%edi),%ecx
	andl	tc_SigRecvd(%edi),%ecx
	je	excend

	/* Clear bits */
	xorl	%ecx,tc_SigExcept(%edi)
	xorl	%ecx,tc_SigRecvd(%edi)

	/* Raise exception. Enable Interrupts */
	movl	tc_ExceptData(%edi),%eax
	leal	Enable(%ebp),%edx
	pushl	%ebp
	call	*%edx
	pushl	%ebp
	pushl	%eax
	pushl	%ecx
	movl	tc_ExceptCode(%edi),%edx
	call	*%edx
	leal	Disable(%ebp),%edx
	pushl	%ebp
	call	*%edx
	addl	$20,%esp

	/* Re-use returned bits */
	orl	%eax,tc_SigExcept(%edi)
	jmp	exloop

excend: /* Restore IDNestCnt and return */
	movb	%ebx,IDNestCnt(%ebp)
	ret
