// SPDX-License-Identifier: BSD-2-Clause
/**
 * @copyright 2018 NXP
 *
 * @file    hash_sw.c
 *
 * @brief   Implementation of the hash pseudo-driver compatible with the
 *          i.MX cryptographic library. Call LibTomCrypt's algorithm instead
 *          of using the HW module.
 */

/* Global includes */
#include <trace.h>
#include <utee_defines.h>

/* Library i.MX includes */
#include <libimxcrypt.h>
#include <libimxcrypt_hash.h>

/* Local includes */
#include "local.h"

//#define LIB_DEBUG
#ifdef LIB_DEBUG
#define LIB_TRACE	DMSG
#else
#define LIB_TRACE(...)
#endif

/**
 * @brief   Find SW hash index into the LibTomCrypt. If supported
 *          be HW, returns (-1)
 *
 * @param[in]      algo   Algorithm ID of the context
 *
 * @retval  >=0 if found
 * @retval  (-1) if not found
 */
static int get_sw_hashindex(enum imxcrypt_hash_id algo)
{
	int index = (-1);

	switch (algo) {
#ifndef CFG_CRYPTO_HASH_HW_MD5
	case HASH_MD5:
		index = find_hash("md5");
		break;
#endif

#ifndef CFG_CRYPTO_HASH_HW_SHA1
	case HASH_SHA1:
		index = find_hash("sha1");
		break;
#endif

#ifndef CFG_CRYPTO_HASH_HW_SHA224
	case HASH_SHA224:
		index = find_hash("sha224");
		break;
#endif

#ifndef CFG_CRYPTO_HASH_HW_SHA256
	case HASH_SHA256:
		index = find_hash("sha256");
		break;
#endif

#ifndef CFG_CRYPTO_HASH_HW_SHA384
	case HASH_SHA384:
		index = find_hash("sha384");
		break;
#endif

#ifndef CFG_CRYPTO_HASH_HW_SHA512
	case HASH_SHA512:
		index = find_hash("sha512");
		break;
#endif

	default:
		break;
	}

	return index;
}

/**
 * @brief   Allocate the SW hashing data context
 *
 * @param[in/out]  ctx    Caller context variable
 * @param[in]      algo   Algorithm ID of the context
 *
 * @retval TEE_SUCCESS                 Success
 * @retval TEE_ERROR_OUT_OF_MEMORY     Out of memory
 * @retval TEE_ERROR_NOT_IMPLEMENTED   Algorithm not implemented
 */
static TEE_Result do_allocate(void **ctx, enum imxcrypt_hash_id algo)
{
	void *hash_ctx;

	LIB_TRACE("HASH_SW: Allocate Context (0x%"PRIxPTR")", (uintptr_t)ctx);

	if (get_sw_hashindex(algo) == (-1))
		return TEE_ERROR_NOT_IMPLEMENTED;

	hash_ctx = calloc(1, sizeof(hash_state));
	if (!hash_ctx) {
		LIB_TRACE("HASH_SW: Allocation Hash data error");
		return TEE_ERROR_OUT_OF_MEMORY;
	}

	*ctx = hash_ctx;
	return TEE_SUCCESS;
}

/**
 * @brief   Free the SW hashing data context
 *
 * @param[in/out]  ctx    Caller context variable
 *
 */
static void do_free(void *ctx)
{
	LIB_TRACE("HASH_SW: Free Context (0x%"PRIxPTR")", (uintptr_t)ctx);

	if (ctx)
		free(ctx);
}

/**
 * @brief   Initialization of the Hash operation
 *
 * @param[in] ctx   Operation Software context
 * @param[in] algo  Algorithm ID of the context
 *
 * @retval TEE_SUCCESS               Success
 * @retval TEE_ERROR_BAD_PARAMETERS  Bad parameters
 * @retval TEE_ERROR_NOT_IMPLEMENTED   Algorithm not implemented
 */
static TEE_Result do_init(void *ctx, enum imxcrypt_hash_id algo)
{
	int hash_idx;
	int ret;

	LIB_TRACE("HASH_SW: Init Algo %d - Context @0x%08"PRIxPTR"",
				algo, (uintptr_t)ctx);

	hash_idx = get_sw_hashindex(algo);

	if (hash_idx == (-1))
		return TEE_ERROR_NOT_IMPLEMENTED;

	ret = hash_descriptor[hash_idx]->init(ctx);

	return conv_CRYPT_to_TEE_Result(ret);
}

/**
 * @brief   Update the Hash operation
 *
 * @param[in] ctx   Operation Software context
 * @param[in] algo  Algorithm ID of the context
 * @param[in] data  Data to hash
 * @param[in] len   Data length
 *
 * @retval TEE_SUCCESS               Success
 * @retval TEE_ERROR_GENERIC         Other Error
 * @retval TEE_ERROR_BAD_PARAMETERS  Bad parameters
 * @retval TEE_ERROR_OUT_OF_MEMORY   Out of memory
 */
static TEE_Result do_update(void *ctx, enum imxcrypt_hash_id algo,
					const uint8_t *data, size_t len)
{
	int hash_idx;
	int ret;

	LIB_TRACE("HASH_SW: Update Algo %d - Input @0x%08"PRIxPTR"-%d",
				algo, (uintptr_t)data, len);

	hash_idx = get_sw_hashindex(algo);

	if (hash_idx == (-1))
		return TEE_ERROR_NOT_IMPLEMENTED;

	ret = hash_descriptor[hash_idx]->process(ctx, data, len);

	return conv_CRYPT_to_TEE_Result(ret);
}

/**
 * @brief   Finalize the Hash operation
 *
 * @param[in] ctx   Operation Software context
 * @param[in] algo  Algorithm ID of the context
 * @param[in] len   Digest buffer length
 *
 * @param[out] digest  Hash digest buffer
 *
 * @retval TEE_SUCCESS               Success
 * @retval TEE_ERROR_GENERIC         Other Error
 * @retval TEE_ERROR_BAD_PARAMETERS  Bad parameters
 * @retval TEE_ERROR_OUT_OF_MEMORY   Out of memory
 */
static TEE_Result do_final(void *ctx, enum imxcrypt_hash_id algo,
					uint8_t *digest, size_t len)
{
	int hash_idx;
	int ret;
	uint8_t *block_digest = NULL;
	uint8_t *tmp_digest   = digest;

	LIB_TRACE("HASH_SW: Final Algo %d - Digest @0x%08"PRIxPTR"-%d",
				algo, (uintptr_t)tmp_digest, len);

	hash_idx = get_sw_hashindex(algo);

	if (hash_idx == (-1))
		return TEE_ERROR_NOT_IMPLEMENTED;

	/* Check the length of the digest */
	if (hash_descriptor[hash_idx]->hashsize > len) {
		/* Allocate a temporary block digest */
		block_digest = malloc(hash_descriptor[hash_idx]->hashsize);
		if (!block_digest)
			return TEE_ERROR_OUT_OF_MEMORY;

		tmp_digest = block_digest;
	}

	ret = hash_descriptor[hash_idx]->done(ctx, tmp_digest);

	if (block_digest) {
		if (ret == CRYPT_OK)
			memcpy(digest, tmp_digest, len);

		free(block_digest);
	}

	return conv_CRYPT_to_TEE_Result(ret);
}

/**
 * @brief   Copy Sofware Hashing Context
 *
 * @param[in] src_ctx  Reference the context source
 *
 * @param[out] dst_ctx  Reference the context destination
 *
 */
static void do_cpy_state(void *dst_ctx, void *src_ctx)
{
	LIB_TRACE("HASH_SW: Copy State (0x%"PRIxPTR") to (0x%"PRIxPTR")",
			 (uintptr_t)src_ctx, (uintptr_t)dst_ctx);

	memcpy(dst_ctx, src_ctx, sizeof(hash_state));
}

/**
 * @brief   Registration of the HASH SW Driver
 */
const struct imxcrypt_hash driver_hash_sw = {
	.alloc_ctx   = &do_allocate,
	.free_ctx    = &do_free,
	.init        = &do_init,
	.update      = &do_update,
	.final       = &do_final,
	.cpy_state   = &do_cpy_state,
	.compute_key = NULL,
};

/**
 * @brief   Initialize the HASH SW module
 *
 * @retval  0    Success
 * @retval  (-1) Otherwise
 */
int libsoft_hash_sw_init(void)
{
	return imxcrypt_register(CRYPTO_HASH_SW, (void *)&driver_hash_sw);
}
