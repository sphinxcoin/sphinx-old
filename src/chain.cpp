// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chain.h"
#include "bignum.h"

uint256 CBlockIndex::GetBlockTrust() const
{
	CBigNum bnTarget;
	bnTarget.SetCompact(nBits);

	if (bnTarget <= 0)
		return 0;

	return ((CBigNum(1) << 256) / (bnTarget + 1)).getuint256();
}
