// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SPHX_SCRIPT_SIGCACHE_H
#define SPHX_SCRIPT_SIGCACHE_H

#include "pubkey.h"

#include <vector>

#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/thread.hpp>

// Valid signature cache, to avoid doing expensive ECDSA signature checking
// twice for every transaction (once when accepted into memory pool, and
// again when accepted into the block chain)
class CSignatureCache
{
private:
     // sigdata_type is (signature hash, signature, public key):
    typedef boost::tuple<uint256, std::vector<unsigned char>, CPubKey> sigdata_type;
    std::set< sigdata_type> setValid;
    boost::shared_mutex cs_sigcache;

public:
    bool Get(const uint256 &hash, const std::vector<unsigned char>& vchSig, const CPubKey& pubKey);

    void Set(const uint256 &hash, const std::vector<unsigned char>& vchSig, const CPubKey& pubKey);
};

#endif // SPHX_SCRIPT_SIGCACHE_H
