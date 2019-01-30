/* SPDX-License-Identifier: BSD-2-Clause */
/**
 * @copyright 2019 NXP
 *
 * @file    snvs_regs.h
 *
 * @brief   SMVS Registers.\n
 */
#ifndef __SNVS_REGS_H__
#define __SNVS_REGS_H__

/*
 * SNVS HP Registers
 */
#define SNVS_HPLR		0x00
#define BM_SNVS_HPLR_MKS_SL		BIT32(9)

#define SNVS_HPCOMR		0x04
#define BM_SNVS_HPCOMR_MKS_EN	BIT32(13)

#define SNVS_HPSTATUS	0x14
#define BM_SNVS_HPSTATUS_OTPMK_ZERO		BIT32(27)
#define BM_SNVS_HPSTATUS_OTPMK_SYND		SHIFT_U32(0x1FF, 16)
#define BM_SNVS_HPSTATUS_SYS_SEC_CFG	SHIFT_U32(0x7, 12)
#define SNVS_HPSTATUS_CLOSED			BIT32(13)
#define SNVS_HPSTATUS_BAD				BIT32(14)

/*
 * SNVS LP Registers
 */
#define SNVS_LPLR		0x34
#define BM_SNVS_LPLR_MKS_HL		BIT32(9)

#define SNVS_LPCR		0x38
#define BM_SNVS_LPCR_TOP		BIT32(6)
#define BM_SNVS_LPCR_DP_EN		BIT32(5)
#define BM_SNVS_LPCR_SRTC_ENV	BIT32(0)

#define SNVS_LPMKCR		0x3C
#define BM_SNVS_LP_MKCR_MKS_SEL	SHIFT_U32(0x3, 0)

#define SNVS_LPSVCR		0x40
#define SNVS_LPTGFCR	0x44
#define SNVS_LPTDCR		0x48
#define SNVS_LPSR		0x4C
#define SNVS_LPSRTCMR	0x50
#define SNVS_LPSRTCLR	0x54
#define SNVS_LPTAR		0x58
#define SNVS_LPSMCMR	0x5C
#define SNVS_LPSMCLR	0x60
#define SNVS_LPPGDR		0x64
#define SNVS_LPGPR0_A	0x68
#define SNVS_LPZMKR0	0x6C
#define SNVS_LPCGR0_30	0x90
#define SNVS_LPCGR0_31	0x94
#define SNVS_LPCGR0_32	0x98
#define SNVS_LPCGR0_33	0x9C
#define SNVS_LPTDC2R	0xA0
#define SNVS_LPTDSR		0xA4
#define SNVS_LPTGF1CR	0xA8
#define SNVS_LPTGF2CR	0xAC
#define SNVS_LPAT1CR	0xC0
#define SNVS_LPAT2CR	0xC4
#define SNVS_LPAT3CR	0xC8
#define SNVS_LPAT4CR	0xCC
#define SNVS_LPAT5CR	0xD0
#define SNVS_LPATCTLR	0xE0
#define SNVS_LPATCLKR	0xE4
#define SNVS_LPATRC1R	0xE8
#define SNVS_LPATRC2R	0xEC


#endif /* __SNVS_REGS_H__ */
