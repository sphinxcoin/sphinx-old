// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SPHX_SCRIPT_SIGN_H
#define SPHX_SCRIPT_SIGN_H

#include "script/interpreter.h"
#include "script/script.h"
#include "main.h"

#include <vector>

using namespace std;

class CKeyStore;
class CScript;
class CTransaction;
class CMutableTransaction;

/** Virtual base class for signature creators. */
class BaseSignatureCreator {
protected:
    const CKeyStore* keystore;

public:
    explicit BaseSignatureCreator(const CKeyStore* keystoreIn) : keystore(keystoreIn) {}
    const CKeyStore& KeyStore() const { return *keystore; };
    virtual ~BaseSignatureCreator() {}
    virtual const BaseSignatureChecker& Checker() const =0;

    /** Create a singular (non-script) signature. */
    virtual bool CreateSig(std::vector<unsigned char>& vchSig, const CKeyID& keyid, const CScript& scriptCode) const =0;
};

/** A signature creator for transactions. */
class TransactionSignatureCreator : public BaseSignatureCreator {
    const CTransaction* txTo;
    unsigned int nIn;
    int nHashType;
    CAmount amount;
//    const TransactionSignatureChecker checker;
    const SignatureChecker checker;

public:
    TransactionSignatureCreator(const CKeyStore* keystoreIn, const CTransaction* txToIn, unsigned int nInIn, const CAmount& amountIn, int nHashTypeIn=SIGHASH_ALL);
    const BaseSignatureChecker& Checker() const override { return checker; }
    bool CreateSig(std::vector<unsigned char>& vchSig, const CKeyID& keyid, const CScript& scriptCode) const override;
};

class MutableTransactionSignatureCreator : public TransactionSignatureCreator {
    CTransaction tx;

public:
    MutableTransactionSignatureCreator(const CKeyStore* keystoreIn, const CMutableTransaction* txToIn, unsigned int nInIn, const CAmount& amountIn, int nHashTypeIn) : TransactionSignatureCreator(keystoreIn, &tx, nInIn, amountIn, nHashTypeIn), tx(*txToIn) {}
};

/** A signature creator that just produces 72-byte empty signatures. */
class DummySignatureCreator : public BaseSignatureCreator {
public:
    explicit DummySignatureCreator(const CKeyStore* keystoreIn) : BaseSignatureCreator(keystoreIn) {}
    const BaseSignatureChecker& Checker() const override;
    bool CreateSig(std::vector<unsigned char>& vchSig, const CKeyID& keyid, const CScript& scriptCode) const override;
};

struct SignatureData {
    CScript scriptSig;

    SignatureData() {}
    explicit SignatureData(const CScript& script) : scriptSig(script) {}
};

/** Produce a script signature using a generic signature creator. */
bool ProduceSignature(const BaseSignatureCreator& creator, const CScript& scriptPubKey, SignatureData& sigdata);

bool CheckSig(vector<unsigned char> vchSig, const vector<unsigned char> &vchPubKey, const CScript &scriptCode, const CTransaction& txTo, unsigned int nIn, int nHashType, int flags);

bool SignSignature(const CKeyStore& keystore, const CScript& fromPubKey, CTransaction& txTo, unsigned int nIn, int nHashType=SIGHASH_ALL);
bool SignSignature(const CKeyStore& keystore, const CTransaction& txFrom, CTransaction& txTo, unsigned int nIn, int nHashType=SIGHASH_ALL);

// Given two sets of signatures for scriptPubKey, possibly with OP_0 placeholders,
// combine them intelligently and return the result.
CScript CombineSignatures(CScript scriptPubKey, const CTransaction& txTo, unsigned int nIn, const CScript& scriptSig1, const CScript& scriptSig2);
SignatureData CombineSignatures(const CScript& scriptPubKey, const BaseSignatureChecker& checker, const SignatureData& scriptSig1, const SignatureData& scriptSig2);

/** Extract signature data from a transaction, and insert it. */
SignatureData DataFromTransaction(const CMutableTransaction& tx, unsigned int nIn);
void UpdateTransaction(CMutableTransaction& tx, unsigned int nIn, const SignatureData& data);

bool Sign1(const CKeyID& address, const CKeyStore& keystore, uint256 hash, int nHashType, CScript& scriptSigRet);
bool SignN(const vector<valtype>& multisigdata, const CKeyStore& keystore, uint256 hash, int nHashType, CScript& scriptSigRet);

#endif  // SPHX_SCRIPT_SIGN_H