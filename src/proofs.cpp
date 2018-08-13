// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2015-2017 The Silk Network developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "proofs.h"

#include "chain.h"
#include "chainparams.h"
#include "main.h"
#include "uint256.h"
#include "util.h"
#include "amount.h"
#include "checkpoints.h"
#include "hashblock.h"
#include "bignum.h"
#include "globals.h"

#include <math.h>
#include <stdint.h> 

unsigned int nStakeMinAge = 1 * 60 * 60; // 3 Hours
unsigned int nModifierInterval = 2 * 60; // time to elapse before new modifier is computed

uint256 CBlock::GetHash() const {
        return GetPoWHash();
}

uint256 CBlock::GetPoWHash() const {
        return Hash(BEGIN(nVersion), END(nNonce));
}

// miner's coin base reward
int64_t GetCoinbaseValue(int nHeight, CAmount nFees)
{
    CAmount nSubsidy;
    if (nHeight >= 1 && nHeight <= 5) {
        nSubsidy = 10000 * COIN;
    } else if (nHeight >= 6 && nHeight <= 10) {
        nSubsidy = 100000 * COIN;
    } else if (nHeight >= 11 && nHeight <= LASTPOWBLOCK ) {
        nSubsidy = 2500 * COIN;
    } else {
        nSubsidy = 0 * COIN;
    }
    return nSubsidy + nFees;
}

// miner's coin stake reward based on coin age spent (coin-days)
int64_t GetCoinstakeValue(int64_t nCoinAge, CAmount nFees, int nHeight)
{
        CAmount nSubsidy = 10 * COIN; // Constant reward of 10 CAT
    return nSubsidy + nFees;
}

unsigned int GetNextTargetRequired(const CBlockIndex* pindexLast, bool fProofOfStake)
{
	CBigNum bnTargetLimit = fProofOfStake ? Params().ProofOfStakeLimit() : Params().ProofOfWorkLimit();

    if (pindexLast == NULL)
        return bnTargetLimit.GetCompact(); // genesis block

    const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, fProofOfStake);
    if (pindexPrev->pprev == NULL)
        return bnTargetLimit.GetCompact(); // first block
    const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);
    if (pindexPrevPrev->pprev == NULL)
        return bnTargetLimit.GetCompact(); // second block

    int64_t nActualSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();

    if (nActualSpacing < 0) {
        nActualSpacing = Params().TargetSpacing();
    }

    // target change every block
    // retarget with exponential moving toward target spacing
    // Includes fix for wrong retargeting difficulty by Mammix2
	CBigNum bnNew;
    bnNew.SetCompact(pindexPrev->nBits);

    int64_t nInterval = Params().TargetTimespan() / Params().TargetSpacing();
    bnNew *= ((nInterval - 1) * Params().TargetSpacing() + nActualSpacing + nActualSpacing);
    bnNew /= ((nInterval + 1) * Params().TargetSpacing());

    if (bnNew <= 0 || bnNew > bnTargetLimit)
        bnNew = bnTargetLimit;

    return bnNew.GetCompact();
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits)
{
	CBigNum bnTarget;
    bnTarget.SetCompact(nBits);

    // Check range
    if (bnTarget <= 0 || bnTarget > Params().ProofOfWorkLimit())
        return error("CheckProofOfWork() : nBits below minimum work");

    // Check proof of work matches claimed amount
    if (hash > bnTarget.getuint256())
        return error("CheckProofOfWork() : hash doesn't match nBits");

    return true;
}
