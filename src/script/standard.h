// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SPHX_SCRIPT_STANDARD_H
#define SPHX_SCRIPT_STANDARD_H

#include "policy/policy.h"
#include "stealth.h"
#include "pubkey.h"

#include <boost/variant.hpp>

#include <vector>
#include <stdint.h>

class CStealthAddress;
class CKeyStore;
class CKeyID;
class CScript;

typedef std::vector<unsigned char> valtype;

/** A reference to a CScript: the Hash160 of its serialization (see script.h) */
class CScriptID : public uint160
{
public:
    CScriptID() : uint160(0) { }
    CScriptID(const CScript& in);
    CScriptID(const uint160 &in) : uint160(in) { }

//    CScriptID GetID();

};


class CNoDestination {
public:
    friend bool operator==(const CNoDestination &a, const CNoDestination &b) { return true; }
    friend bool operator<(const CNoDestination &a, const CNoDestination &b) { return true; }
};

/** A txout script template with a specific destination. It is either:
 *  * CNoDestination: no destination set
 *  * CKeyID: TX_PUBKEYHASH destination
 *  * CScriptID: TX_SCRIPTHASH destination
 *  A CTxDestination is the internal data type encoded in a CSphinxcoinAddress
 */
typedef boost::variant<CNoDestination, CKeyID, CScriptID, CStealthAddress> CTxDestination;

const char* GetTxnOutputType(txnouttype t);

bool Solver(const CScript& scriptPubKey, txnouttype& typeRet, std::vector<std::vector<unsigned char> >& vSolutionsRet);
bool Solver(const CKeyStore& keystore, const CScript& scriptPubKey, uint256 hash, int nHashType, CScript& scriptSigRet, txnouttype& whichTypeRet);

bool ExtractDestination(const CScript& scriptPubKey, CTxDestination& addressRet);
bool ExtractDestinations(const CScript& scriptPubKey, txnouttype& typeRet, std::vector<CTxDestination>& addressRet, int& nRequiredRet);

CScript GetScriptForDestination(const CTxDestination& dest);
CScript GetScriptForRawPubKey(const CPubKey& pubkey);
CScript GetScriptForMultisig(int nRequired, const std::vector<CPubKey>& keys);

#endif // SPHX_SCRIPT_STANDARD_H