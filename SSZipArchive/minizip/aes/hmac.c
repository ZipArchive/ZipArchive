/*
 ---------------------------------------------------------------------------
 Copyright (c) 2002, Dr Brian Gladman, Worcester, UK.   All rights reserved.

 LICENSE TERMS

 The free distribution and use of this software in both source and binary
 form is allowed (with or without changes) provided that:

   1. distributions of this source code include the above copyright
      notice, this list of conditions and the following disclaimer;

   2. distributions in binary form include the above copyright
      notice, this list of conditions and the following disclaimer
      in the documentation and/or other associated materials;

   3. the copyright holder's name is not used to endorse products
      built using this software without specific written permission.

 ALTERNATIVELY, provided that this notice is retained in full, this product
 may be distributed under the terms of the GNU General Public License (GPL),
 in which case the provisions of the GPL apply INSTEAD OF those given above.

 DISCLAIMER

 This software is provided 'as is' with no explicit or implied warranties
 in respect of its properties, including, but not limited to, correctness
 and/or fitness for purpose.
 ---------------------------------------------------------------------------
 Issue Date: 26/08/2003

 This is an implementation of HMAC, the FIPS standard keyed hash function
*/

#include "hmac.h"
#include "brg_types.h"

#if defined(__cplusplus)
extern "C"
{
#endif

/* initialise the HMAC context to zero */
void hmac_sha_begin(hmac_ctx cx[1])
{
    memset(cx, 0, sizeof(hmac_ctx));
}

/* input the HMAC key (can be called multiple times)    */
int hmac_sha_key(const unsigned char key[], unsigned long key_len, hmac_ctx cx[1])
{
    if(cx->klen == HMAC_IN_DATA)                /* error if further key input   */
        return HMAC_BAD_MODE;                   /* is attempted in data mode    */

    if(cx->klen + key_len > HASH_INPUT_SIZE)    /* if the key has to be hashed  */
    {
        if(cx->klen <= HASH_INPUT_SIZE)         /* if the hash has not yet been */
        {                                       /* started, initialise it and   */
            sha_begin(cx->ctx);                 /* hash stored key characters   */
            sha_hash(cx->key, cx->klen, cx->ctx);
        }

        sha_hash(key, key_len, cx->ctx);       /* hash long key data into hash */
    }
    else                                        /* otherwise store key data     */
        memcpy(cx->key + cx->klen, key, key_len);

    cx->klen += key_len;                        /* update the key length count  */
    return HMAC_OK;
}

/* input the HMAC data (can be called multiple times) - */
/* note that this call terminates the key input phase   */
void hmac_sha_data(const unsigned char data[], unsigned long data_len, hmac_ctx cx[1])
{   unsigned int i;

    if(cx->klen != HMAC_IN_DATA)                /* if not yet in data phase */
    {
        if(cx->klen > HASH_INPUT_SIZE)          /* if key is being hashed   */
        {                                       /* complete the hash and    */
            sha_end(cx->key, cx->ctx);          /* store the result as the  */
            cx->klen = HASH_OUTPUT_SIZE;        /* key and set new length   */
        }

        /* pad the key if necessary */
        memset(cx->key + cx->klen, 0, HASH_INPUT_SIZE - cx->klen);

        /* xor ipad into key value  */
        for(i = 0; i < (HASH_INPUT_SIZE >> 2); ++i)
            ((uint_32t*)cx->key)[i] ^= 0x36363636;

        /* and start hash operation */
        sha_begin(cx->ctx);
        sha_hash(cx->key, HASH_INPUT_SIZE, cx->ctx);

        /* mark as now in data mode */
        cx->klen = HMAC_IN_DATA;
    }

    /* hash the data (if any)       */
    if(data_len)
        sha_hash(data, data_len, cx->ctx);
}

/* compute and output the MAC value */
void hmac_sha_end(unsigned char mac[], unsigned long mac_len, hmac_ctx cx[1])
{   unsigned char dig[HASH_OUTPUT_SIZE];
    unsigned int i;

    /* if no data has been entered perform a null data phase        */
    if(cx->klen != HMAC_IN_DATA)
        hmac_sha_data((const unsigned char*)0, 0, cx);

    sha_end(dig, cx->ctx);         /* complete the inner hash       */

    /* set outer key value using opad and removing ipad */
    for(i = 0; i < (HASH_INPUT_SIZE >> 2); ++i)
        ((uint_32t*)cx->key)[i] ^= 0x36363636 ^ 0x5c5c5c5c;

    /* perform the outer hash operation */
    sha_begin(cx->ctx);
    sha_hash(cx->key, HASH_INPUT_SIZE, cx->ctx);
    sha_hash(dig, HASH_OUTPUT_SIZE, cx->ctx);
    sha_end(dig, cx->ctx);

    /* output the hash value            */
    for(i = 0; i < mac_len; ++i)
        mac[i] = dig[i];
}

/* 'do it all in one go' subroutine     */
void hmac_sha(const unsigned char key[], unsigned long key_len,
          const unsigned char data[], unsigned long data_len,
          unsigned char mac[], unsigned long mac_len)
{   hmac_ctx    cx[1];

    hmac_sha_begin(cx);
    hmac_sha_key(key, key_len, cx);
    hmac_sha_data(data, data_len, cx);
    hmac_sha_end(mac, mac_len, cx);
}

#if defined(__cplusplus)
}
#endif
