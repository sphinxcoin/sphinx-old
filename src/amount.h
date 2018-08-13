// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SPHX_AMOUNT_H
#define SPHX_AMOUNT_H

#include "serialize.h"

#include "utilstrencodings.h"

#include <stdlib.h>
#include <string>

/** Amount in satoshis (Can be negative) */
typedef int64_t CAmount;

static const CAmount COIN = 100000000;
static const CAmount CENT = 1000000;

/** No amount larger than this (in satoshi) is valid.
 *
 * Note that this constant is *not* the total money supply, which in Sphinxcoin
 * currently happens to be less than 21,000,000 SPHX for various reasons, but
 * rather a sanity check. As this sanity check is used by consensus-critical
 * validation code, the exact value of the MAX_MONEY constant is consensus
 * critical; in unusual circumstances like a(nother) overflow bug that allowed
 * for the creation of coins out of thin air modification could lead to a fork.
 * */

 /**
   In whitepaper are 55Mio coins and according to this value, it is bitcoins default.
   Looking at transaction of the premine, I only assume that premine value was 16.4Mio
   getblocknumber 1 returns     "mint" : 16400000.00000000, which means
      It this case we can count (Total amount)-(Premine)= 55000000-16400000 = 38600000
 */
static const CAmount MAX_MONEY = 21000000 * COIN; //fix correct amount of coins
inline bool MoneyRange(const CAmount& nValue) { return (nValue >= 0 && nValue <= MAX_MONEY); }

#endif //  SPHX_AMOUNT_H
