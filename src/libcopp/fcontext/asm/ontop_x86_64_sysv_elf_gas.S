/*
            Copyright Oliver Kowalke 2009.
   Distributed under the Boost Software License, Version 1.0.
      (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

/****************************************************************************************
 *                                                                                      *
 *  ----------------------------------------------------------------------------------  *
 *  |    0    |    1    |    2    |    3    |    4     |    5    |    6    |    7    |  *
 *  ----------------------------------------------------------------------------------  *
 *  |   0x0   |   0x4   |   0x8   |   0xc   |   0x10   |   0x14  |   0x18  |   0x1c  |  *
 *  ----------------------------------------------------------------------------------  *
 *  |        R12        |         R13       |         R14        |        R15        |  *
 *  ----------------------------------------------------------------------------------  *
 *  ----------------------------------------------------------------------------------  *
 *  |    8    |    9    |   10    |   11    |    12    |    13   |    14   |    15   |  *
 *  ----------------------------------------------------------------------------------  *
 *  |   0x20  |   0x24  |   0x28  |  0x2c   |   0x30   |   0x34  |   0x38  |   0x3c  |  *
 *  ----------------------------------------------------------------------------------  *
 *  |        RBX        |         RBP       |         RIP        |       EXIT        |  *
 *  ----------------------------------------------------------------------------------  *
 *                                                                                      *
 ****************************************************************************************/

.text
.globl copp_ontop_fcontext
.type copp_ontop_fcontext,@function
.align 16
copp_ontop_fcontext:
    pushq  %rbp  /* save RBP */
    pushq  %rbx  /* save RBX */
    pushq  %r15  /* save R15 */
    pushq  %r14  /* save R14 */
    pushq  %r13  /* save R13 */
    pushq  %r12  /* save R12 */

    /* store RSP (pointing to context-data) in RAX */
    movq  %rsp, %rax

    /* restore RSP (pointing to context-data) from RDI */
    movq  %rdi, %rsp

    popq  %r12  /* restrore R12 */
    popq  %r13  /* restrore R13 */
    popq  %r14  /* restrore R14 */
    popq  %r15  /* restrore R15 */
    popq  %rbx  /* restrore RBX */
    popq  %rbp  /* restrore RBP */

    /* preserve ontop-function in R8 */
    movq  %rdx, %r8

    /* return transfer_t from jump */
    /* RAX == fctx, RDX == data */
    movq  %rsi, %rdx
    /* pass transfer_t as first arg in context function */
    /* RDI == fctx, RSI == data */
    movq  %rax, %rdi

    /* keep return-address on stack */

    /* indirect jump to context */
    jmp  *%r8
.size copp_ontop_fcontext,.-copp_ontop_fcontext

/* Mark that we don't need executable stack.  */
.section .note.GNU-stack,"",%progbits
