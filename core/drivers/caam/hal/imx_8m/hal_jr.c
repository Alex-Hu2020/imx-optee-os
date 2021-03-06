// SPDX-License-Identifier: BSD-2-Clause
/**
 * @copyright 2018 NXP
 *
 * @file    hal_jr.c
 *
 * @brief   CAAM Job Rings Hardware Abstration Layer.\n
 *          Implementation of primitives to access HW
 */

/* Global includes */
#include <io.h>

/* Local includes */
#include "common.h"
#include "caam_pwr.h"

/* Hal includes */
#include "hal_jr.h"

/* Registers includes */
#include "ctrl_regs.h"
#include "jr_regs.h"

//#define HAL_DEBUG
#ifdef HAL_DEBUG
#define HAL_TRACE		DRV_TRACE
#else
#define HAL_TRACE(...)
#endif

/*
 * List of JR configuration registers to save/restore
 */
const struct reglist jrcfg_backup[] = {
	{JR0DID_MS, 1, 0, 0},
	{JR0DID_LS, 1, 0, 0},
};

/**
 * @brief   Configures the Job Ring Owner and lock it.\n
 *          If the configuration is already locked, checks if the configuration
 *          set and returns an error if value is not corresponding to the
 *          expected value.
 *
 * @param[in] ctrl_base  Base address of the controller
 * @param[in] jr_offset  Job Ring offset to configure
 * @param[in] owner      Onwer ID to configure
 *
 * @retval   CAAM_NO_ERROR  Success
 * @retval   CAAM_FAILURE   An error occurred
 *
 */
enum CAAM_Status hal_jr_setowner(vaddr_t ctrl_base, paddr_t jr_offset,
							enum jr_owner owner)
{
	enum CAAM_Status retstatus = CAAM_FAILURE;

	uint32_t val;
	uint32_t cfg_ms;
	uint32_t cfg_ls = 0;
	uint8_t  jr_idx = JRx_IDX(jr_offset);

	/* Read the Job Ring Lock bit */
	val = read32(ctrl_base + JRxDID_MS(jr_idx));
	HAL_TRACE("JR%dDID_MS value 0x%x", jr_idx, val);

	/* Prepare the Job Ring MS/LS registers */
	if (owner & JROWNER_SECURE) {
		/* Configuration only lock for the Secure JR */
		cfg_ms  = JRxDID_MS_PRIM_DID(owner & ~JROWNER_SECURE);
		cfg_ms |= JRxDID_MS_PRIM_TZ | JRxDID_MS_TZ_OWN;
		cfg_ms |= JRxDID_MS_AMTD;
		cfg_ms |= JRxDID_MS_PRIM_ICID(owner & ~JROWNER_SECURE);
#ifdef CFG_IMXCRYPT
		cfg_ms |= JRxDID_MS_LDID;
		cfg_ms |= JRxDID_MS_LAMTD;
#endif
	} else {
		cfg_ms  = JRxDID_MS_PRIM_DID(owner);
		cfg_ms |= JRxDID_MS_PRIM_ICID(owner);
	}

	/*
	 * If the configuration already locked, check that is the configuration
	 * that we want. If not return in error.
	 */
	if (val & JRxDID_MS_LDID) {
		/* Check if the setup configuration is correct or not */
		HAL_TRACE("JR%dDID_MS value 0x%x (0x%x)", jr_idx, val, cfg_ms);
		if ((cfg_ms | JRxDID_MS_LDID) == val) {
			/* Read the LS register and compare with
			 * expected value
			 */
			val = read32(ctrl_base + JRxDID_LS(jr_idx));
			HAL_TRACE("JR%dDID_LS value 0x%x (0x%x)",
				jr_idx, val, cfg_ls);
			if (val == cfg_ls)
				retstatus = CAAM_NO_ERROR;
		}
	} else {
		HAL_TRACE("JR%dDID_LS set value 0x%x", jr_idx, cfg_ls);
		HAL_TRACE("JR%dDID_MS set value 0x%x", jr_idx, cfg_ms);
		/* Set the configuration */
		write32(cfg_ls, ctrl_base + JRxDID_LS(jr_idx));
		write32(cfg_ms, ctrl_base + JRxDID_MS(jr_idx));
		retstatus = CAAM_NO_ERROR;
	}

	return retstatus;
}

/**
 * @brief   Let the JR prepare data that need backup
 *
 * @param[in] ctrl_base   CAAM JR Base Address
 * @param[in] jr_offset   Job Ring offset to prepare backup for
 *
 * @retval index of the next entry in the queue
 */
void hal_jr_prepare_backup(vaddr_t ctrl_base, paddr_t jr_offset)
{
	uint8_t jr_idx = JRx_IDX(jr_offset);

	caam_pwr_add_backup(ctrl_base + (jr_idx * JRxDID_SIZE),
			jrcfg_backup, ARRAY_SIZE(jrcfg_backup));
}
