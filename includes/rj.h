#ifndef RJ_H
#define RJ_H

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

// debug includes
#include <stdio.h>
#include <stdlib.h>

#define DEBUG 0

#define RIJN_MAX_ROUNDS 14

/*
 * Constants representing choice of Rijndael key or block size,
 * either of which can be 128 bits, 192 bits or 256 bits.
 */

	typedef enum {
		rijn_128 = 4,   /* number of rows */
		rijn_192 = 6,
    rijn_256 = 8
	} rijn_size_t;

/*
 * Rijndael parameters.
 */

	typedef struct {
		rijn_size_t rijn_blockrows;
		rijn_size_t rijn_keyrows;
	} rijn_param_t;

#define RIJN_PARAM_DEFAULT_INITIALIZER { rijn_128, rijn_256 }
#define RIJN_PARAM_INITIALIZER(BLOCKSZ, KEYSZ) { (BLOCKSZ), (KEYSZ) }

//	void rijn_param_init(rijn_param_t *, rijn_size_t blocksz, rijn_size_t keysz);

/*
 * Rijndael keys are 128, 192 or 256 bits wide.  There are up to
 * RIJN_MAX_ROUNDS + 1 round keys needed, so the schedule is that big.
 */

	typedef unsigned char rijn_unit_t[4];
	typedef rijn_unit_t rijn_key_t[8];
	typedef rijn_unit_t rijn_block_t[8];
	typedef unsigned char rijn_flatblock_t[sizeof (rijn_block_t)];

	typedef struct {
		rijn_param_t rijn_param;
		int rijn_nrounds;
		rijn_block_t rijn_roundkey[RIJN_MAX_ROUNDS];
	} rijn_keysched_t;

	void rijn_sched_key(rijn_keysched_t *, rijn_key_t *, const rijn_param_t *);
	void rijn_encrypt(rijn_keysched_t *, unsigned char *, const unsigned char *);
	void rijn_decrypt(rijn_keysched_t *, unsigned char *, const unsigned char *);
	void rijn_cbc_encrypt(rijn_keysched_t *, unsigned char *iv, unsigned char *out, 
						  const unsigned char *in, size_t nblocks);
	void rijn_cbc_decrypt(rijn_keysched_t *, unsigned char *iv, unsigned char *out,
						  const unsigned char *in, size_t nblocks);
	void rijn_cfb_encrypt(rijn_keysched_t *, unsigned char *iv, unsigned char *out,
						  const unsigned char *pt, size_t nbytes);
	void rijn_cfb_decrypt(rijn_keysched_t *, unsigned char *iv, unsigned char *out,
						  const unsigned char *in, size_t nbytes);

/* utils fct*/
	char    *ft_strncpyz(char *dst, const char *src, size_t n);
	char        *ft_strncatz(char *dest, const char *src, size_t n);

  void rijn_init(rijn_keysched_t *sched, uint64_t keysha);
	char *rijn_build_decrypt(rijn_keysched_t *sched,   unsigned char *str);
	char *rijn_build_encrypt(rijn_keysched_t *sched,   unsigned char *str);
	void hex_dump(const unsigned char *data, size_t size, FILE *stream);	

#endif
