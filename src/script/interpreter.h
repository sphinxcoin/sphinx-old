// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SPHX_SCRIPT_INTERPRETER_H
#define SPHX_SCRIPT_INTERPRETER_H

#include "script_error.h"
#include "primitives/transaction.h"

#include <vector>
#include <stdint.h>
#include <string>

typedef std::vector<unsigned char> valtype;

class CPubKey;
class CScript;
class CTransaction;
class uint256;

//class BaseSignatureChecker;

/** Signature hash types/flags */
enum
{
    SIGHASH_ALL = 1,
    SIGHASH_NONE = 2,
    SIGHASH_SINGLE = 3,
    SIGHASH_ANYONECANPAY = 0x80,
};

bool CheckSignatureEncoding(const valtype &vchSig, unsigned int flags, ScriptError* serror);

uint256 SignatureHash(CScript scriptCode, const CTransaction& txTo, unsigned int nIn, int nHashType);

class BaseSignatureChecker
{
public:
    virtual bool CheckSig(const std::vector<unsigned char>& scriptSig, const std::vector<unsigned char>& vchPubKey, const CScript& scriptCode) const
    {
        return false;
    }

    virtual ~BaseSignatureChecker() {}
};

class SignatureChecker : public BaseSignatureChecker
{
private:
    const CTransaction& txTo;
    unsigned int nIn;

protected:
    virtual bool VerifySignature(const std::vector<unsigned char>& vchSig, const CPubKey& vchPubKey, const uint256& sighash) const;

public:
    SignatureChecker(const CTransaction& txToIn, unsigned int nInIn) : txTo(txToIn), nIn(nInIn) {}
    bool CheckSig(const std::vector<unsigned char>& scriptSig, const std::vector<unsigned char>& vchPubKey, const CScript& scriptCode) const;
};

class MutableTransactionSignatureChecker : public SignatureChecker
{
private:
    const CTransaction txTo;

public:
    MutableTransactionSignatureChecker(const CMutableTransaction* txToIn, unsigned int nInIn) : SignatureChecker(txTo, nInIn), txTo(*txToIn) {}
};
 
bool EvalScript(std::vector<std::vector<unsigned char> >& stack, const CScript& script, const CTransaction& txTo, unsigned int nIn, unsigned int flags, int nHashType);
bool EvalScript(std::vector<std::vector<unsigned char> >& stack, const CScript& script, unsigned int flags, const BaseSignatureChecker& checker, ScriptError* error = NULL);

bool VerifyScript(const CScript& scriptSig, const CScript& scriptPubKey, const CTransaction& txTo, unsigned int nIn, unsigned int flags, int nHashType);
bool VerifyScript(const CScript& scriptSig, const CScript& scriptPubKey, unsigned int flags, const BaseSignatureChecker& checker, ScriptError* error = NULL);

bool VerifySignature(const CTransaction& txFrom, const CTransaction& txTo, unsigned int nIn, unsigned int flags, int nHashType);

#endif // SPHX_SCRIPT_INTERPRETER_H