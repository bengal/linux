/*
 *  This file contains the power_save function for Power7 CPUs.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 */

#include <linux/threads.h>
#include <asm/processor.h>
#include <asm/page.h>
#include <asm/cputable.h>
#include <asm/thread_info.h>
#include <asm/ppc_asm.h>
#include <asm/asm-offsets.h>
#include <asm/ppc-opcode.h>
#include <asm/hw_irq.h>
#include <asm/kvm_book3s_asm.h>
#include <asm/opal.h>

#undef DEBUG

/* Idle state entry routines */

#define	IDLE_STATE_ENTER_SEQ(IDLE_INST)				\
	/* Magic NAP/SLEEP/WINKLE mode enter sequence */	\
	std	r0,0(r1);					\
	ptesync;						\
	ld	r0,0(r1);					\
1:	cmp	cr0,r0,r0;					\
	bne	1b;						\
	IDLE_INST;						\
	b	.

	.text

/*
 * Pass requested state in r3:
 * 	0 - nap
 * 	1 - sleep
 *
 * To check IRQ_HAPPENED in r4
 * 	0 - don't check
 * 	1 - check
 */
_GLOBAL(power7_powersave_common)
	/* Use r3 to pass state nap/sleep/winkle */
	/* NAP is a state loss, we create a regs frame on the
	 * stack, fill it up with the state we care about and
	 * stick a pointer to it in PACAR1. We really only
	 * need to save PC, some CR bits and the NV GPRs,
	 * but for now an interrupt frame will do.
	 */
	mflr	r0
	std	r0,16(r1)
	stdu	r1,-INT_FRAME_SIZE(r1)
	std	r0,_LINK(r1)
	std	r0,_NIP(r1)

#ifndef CONFIG_SMP
	/* Make sure FPU, VSX etc... are flushed as we may lose
	 * state when going to nap mode
	 */
	bl	discard_lazy_cpu_state
#endif /* CONFIG_SMP */

	/* Hard disable interrupts */
	mfmsr	r9
	rldicl	r9,r9,48,1
	rotldi	r9,r9,16
	mtmsrd	r9,1			/* hard-disable interrupts */

	/* Check if something happened while soft-disabled */
	lbz	r0,PACAIRQHAPPENED(r13)
	cmpwi	cr0,r0,0
	beq	1f
	cmpwi	cr0,r4,0
	beq	1f
	addi	r1,r1,INT_FRAME_SIZE
	ld	r0,16(r1)
	mtlr	r0
	blr

1:	/* We mark irqs hard disabled as this is the state we'll
	 * be in when returning and we need to tell arch_local_irq_restore()
	 * about it
	 */
	li	r0,PACA_IRQ_HARD_DIS
	stb	r0,PACAIRQHAPPENED(r13)

	/* We haven't lost state ... yet */
	li	r0,0
	stb	r0,PACA_NAPSTATELOST(r13)

	/* Continue saving state */
	SAVE_GPR(2, r1)
	SAVE_NVGPRS(r1)
	mfcr	r4
	std	r4,_CCR(r1)
	std	r9,_MSR(r1)
	std	r1,PACAR1(r13)

_GLOBAL(power7_enter_nap_mode)
#ifdef CONFIG_KVM_BOOK3S_HV_POSSIBLE
	/* Tell KVM we're napping */
	li	r4,KVM_HWTHREAD_IN_NAP
	stb	r4,HSTATE_HWTHREAD_STATE(r13)
#endif
	cmpwi	cr0,r3,1
	beq	2f
	IDLE_STATE_ENTER_SEQ(PPC_NAP)
	/* No return */
2:	IDLE_STATE_ENTER_SEQ(PPC_SLEEP)
	/* No return */

_GLOBAL(power7_idle)
	/* Now check if user or arch enabled NAP mode */
	LOAD_REG_ADDRBASE(r3,powersave_nap)
	lwz	r4,ADDROFF(powersave_nap)(r3)
	cmpwi	0,r4,0
	beqlr
	li	r3, 1
	/* fall through */

_GLOBAL(power7_nap)
	mr	r4,r3
	li	r3,0
	b	power7_powersave_common
	/* No return */

_GLOBAL(power7_sleep)
	li	r3,1
	li	r4,0
	b	power7_powersave_common
	/* No return */

_GLOBAL(power7_wakeup_tb_loss)
	ld	r2,PACATOC(r13);
	ld	r1,PACAR1(r13)

	/* Time base re-sync */
	li	r0,OPAL_RESYNC_TIMEBASE
	LOAD_REG_ADDR(r11,opal);
	ld	r12,8(r11);
	ld	r2,0(r11);
	mtctr	r12
	bctrl

	/* TODO: Check r3 for failure */

	REST_NVGPRS(r1)
	REST_GPR(2, r1)
	ld	r3,_CCR(r1)
	ld	r4,_MSR(r1)
	ld	r5,_NIP(r1)
	addi	r1,r1,INT_FRAME_SIZE
	mtcr	r3
	mfspr	r3,SPRN_SRR1		/* Return SRR1 */
	mtspr	SPRN_SRR1,r4
	mtspr	SPRN_SRR0,r5
	rfid

_GLOBAL(power7_wakeup_loss)
	ld	r1,PACAR1(r13)
	REST_NVGPRS(r1)
	REST_GPR(2, r1)
	ld	r3,_CCR(r1)
	ld	r4,_MSR(r1)
	ld	r5,_NIP(r1)
	addi	r1,r1,INT_FRAME_SIZE
	mtcr	r3
	mtspr	SPRN_SRR1,r4
	mtspr	SPRN_SRR0,r5
	rfid

_GLOBAL(power7_wakeup_noloss)
	lbz	r0,PACA_NAPSTATELOST(r13)
	cmpwi	r0,0
	bne	power7_wakeup_loss
	ld	r1,PACAR1(r13)
	ld	r4,_MSR(r1)
	ld	r5,_NIP(r1)
	addi	r1,r1,INT_FRAME_SIZE
	mtspr	SPRN_SRR1,r4
	mtspr	SPRN_SRR0,r5
	rfid
