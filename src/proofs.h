// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SPHX_POW_H
#define SPHX_POW_H

#include <stdint.h>

#include "amount.h"
#include <sync.h>
#include <util.h>
#include <stdint.h>

class CBlockIndex;
class uint256;

/** Calculate difficulty using retarget algorithm by maintaining target */
unsigned int GetNextTargetRequired(const CBlockIndex* pindexLast, bool fProofOfStake);
/** Calculate difficulty using retarget algorithm V1 by maintaining target */
unsigned int GetNextTargetRequiredV1(const CBlockIndex* pindexLast, bool fProofOfStake);
/** Calculate difficulty using retarget algorithm V2 by maintaining target */
unsigned int GetNextTargetRequiredV2(const CBlockIndex* pindexLast, bool fProofOfStake);
/** Check whether a block hash satisfies the proof-of-work requirement specified by nBits */
bool CheckProofOfWork(uint256 hash, unsigned int nBits);
/** Determine Block Reward for Proof Of Work **/
int64_t GetCoinbaseValue(int nHeight, CAmount nFees);
/** Determine Block Reward for Proof Of Stake **/
int64_t GetCoinstakeValue(int64_t nCoinAge, CAmount nFees, int nHeight);

#endif // SPHX_POW_H
