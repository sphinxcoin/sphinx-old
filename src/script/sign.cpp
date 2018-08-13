// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "script/sign.h"
#include "script/script.h"
#include "script/sigcache.h"
#include "script/interpreter.h"

#include "main.h"

#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

using namespace std;
using namespace boost;

TransactionSignatureCreator::TransactionSignatureCreator(const CKeyStore* keystoreIn, const CTransaction* txToIn, unsigned int nInIn, const CAmount& amountIn, int nHashTypeIn) : BaseSignatureCreator(keystoreIn), txTo(txToIn), nIn(nInIn), nHashType(nHashTypeIn), amount(amountIn), checker(*txTo, nIn) {}

bool TransactionSignatureCreator::CreateSig(std::vector<unsigned char>& vchSig, const CKeyID& address, const CScript& scriptCode) const
{
    CKey key;
    if (!keystore->GetKey(address, key))
        return false;

    uint256 hash = SignatureHash(scriptCode, *txTo, nIn, nHashType);
    if (!key.Sign(hash, vchSig))
        return false;
    vchSig.push_back((unsigned char)nHashType);
    return true;
}

void UpdateTransaction(CMutableTransaction& tx, unsigned int nIn, const SignatureData& data)
{
    assert(tx.vin.size() > nIn);
    tx.vin[nIn].scriptSig = data.scriptSig;
//    tx.vin[nIn].scriptWitness = data.scriptWitness;
}

bool Sign1(const CKeyID& address, const CKeyStore& keystore, uint256 hash, int nHashType, CScript& scriptSigRet)
{
    CKey key;
    if (!keystore.GetKey(address, key))
        return false;

    vector<unsigned char> vchSig;
    bool test = -1;
    if (!key.Sign(hash, vchSig, test))
        return false;
    vchSig.push_back((unsigned char)nHashType);
    scriptSigRet << vchSig;

    return true;
}

bool SignN(const vector<valtype>& multisigdata, const CKeyStore& keystore, uint256 hash, int nHashType, CScript& scriptSigRet)
{
    int nSigned = 0;
    int nRequired = multisigdata.front()[0];
    for (unsigned int i = 1; i < multisigdata.size()-1 && nSigned < nRequired; i++)
    {
        const valtype& pubkey = multisigdata[i];
        CKeyID keyID = CPubKey(pubkey).GetID();
        if (Sign1(keyID, keystore, hash, nHashType, scriptSigRet))
            ++nSigned;
    }
    return nSigned==nRequired;
}

static bool Sign1c(const CKeyID& address, const BaseSignatureCreator& creator, const CScript& scriptCode, std::vector<valtype>& ret)
{
    std::vector<unsigned char> vchSig;
    if (!creator.CreateSig(vchSig, address, scriptCode))
        return false;
    ret.push_back(vchSig);
    return true;
}

static bool SignNc(const std::vector<valtype>& multisigdata, const BaseSignatureCreator& creator, const CScript& scriptCode, std::vector<valtype>& ret)
{
    int nSigned = 0;
    int nRequired = multisigdata.front()[0];
    for (unsigned int i = 1; i < multisigdata.size()-1 && nSigned < nRequired; i++)
    {
        const valtype& pubkey = multisigdata[i];
        CKeyID keyID = CPubKey(pubkey).GetID();
        if (Sign1c(keyID, creator, scriptCode, ret))
            ++nSigned;
    }
    return nSigned==nRequired;
}

/**
 * Sign scriptPubKey using signature made with creator.
 * Signatures are returned in scriptSigRet (or returns false if scriptPubKey can't be signed),
 * unless whichTypeRet is TX_SCRIPTHASH, in which case scriptSigRet is the redemption script.
 * Returns false if scriptPubKey could not be completely satisfied.
 */
 static bool SignStep(const BaseSignatureCreator& creator, const CScript& scriptPubKey,
                     std::vector<valtype>& ret, txnouttype& whichTypeRet)
{
    CScript scriptRet;
    uint160 h160;
    ret.clear();

    std::vector<valtype> vSolutions;
    if (!Solver(scriptPubKey, whichTypeRet, vSolutions))
        return false;

    CKeyID keyID;
    switch (whichTypeRet)
    {
    case TX_NONSTANDARD:
    case TX_NULL_DATA:
        return false;
    case TX_PUBKEY:
        keyID = CPubKey(vSolutions[0]).GetID();
        return Sign1c(keyID, creator, scriptPubKey, ret);
    case TX_PUBKEYHASH:
        keyID = CKeyID(uint160(vSolutions[0]));
        if (!Sign1c(keyID, creator, scriptPubKey, ret))
            return false;
        else
        {
            CPubKey vch;
            creator.KeyStore().GetPubKey(keyID, vch);
            ret.push_back(ToByteVector(vch));
        }
        return true;
    case TX_SCRIPTHASH:
        if (creator.KeyStore().GetCScript(uint160(vSolutions[0]), scriptRet)) {
            ret.push_back(std::vector<unsigned char>(scriptRet.begin(), scriptRet.end()));
            return true;
        }
        return false;

    case TX_MULTISIG:
        ret.push_back(valtype()); // workaround CHECKMULTISIG bug
        return (SignNc(vSolutions, creator, scriptPubKey, ret));

    default:
        return false;
    }
}

bool SignSignature(const CKeyStore &keystore, const CScript& fromPubKey, CTransaction& txTo, unsigned int nIn, int nHashType)
{
    assert(nIn < txTo.vin.size());
    CTxIn& txin = txTo.vin[nIn];

    // Leave out the signature from the hash, since a signature can't sign itself.
    // The checksig op will also drop the signatures from its hash.
    uint256 hash = SignatureHash(fromPubKey, txTo, nIn, nHashType);

    txnouttype whichType;
    if (!Solver(keystore, fromPubKey, hash, nHashType, txin.scriptSig, whichType))
        return false;

    if (whichType == TX_SCRIPTHASH)
    {
        // Solver returns the subscript that need to be evaluated;
        // the final scriptSig is the signatures from that
        // and then the serialized subscript:
        CScript subscript = txin.scriptSig;

        // Recompute txn hash using subscript in place of scriptPubKey:
        uint256 hash2 = SignatureHash(subscript, txTo, nIn, nHashType);

        txnouttype subType;
        bool fSolved =
            Solver(keystore, subscript, hash2, nHashType, txin.scriptSig, subType) && subType != TX_SCRIPTHASH;
        // Append serialized subscript whether or not it is completely signed:
        txin.scriptSig << static_cast<valtype>(subscript);
        if (!fSolved) return false;
    }

    // Test solution
    return VerifyScript(txin.scriptSig, fromPubKey, STANDARD_SCRIPT_VERIFY_FLAGS, SignatureChecker(txTo, nIn));
}

bool SignSignature(const CKeyStore &keystore, const CTransaction& txFrom, CTransaction& txTo, unsigned int nIn, int nHashType)
{
    assert(nIn < txTo.vin.size());
    CTxIn& txin = txTo.vin[nIn];
    assert(txin.prevout.n < txFrom.vout.size());
    const CTxOut& txout = txFrom.vout[txin.prevout.n];

    return SignSignature(keystore, txout.scriptPubKey, txTo, nIn, nHashType);
}



/*bool SignSignature(const CKeyStore &keystore, const CScript& fromPubKey, CTransaction& txTo, unsigned int nIn, int nHashType)
{
    assert(nIn < txTo.vin.size());
    CTxIn& txin = txTo.vin[nIn];

    // Leave out the signature from the hash, since a signature can't sign itself.
    // The checksig op will also drop the signatures from its hash.
    uint256 hash = SignatureHash(fromPubKey, txTo, nIn, nHashType);

    txnouttype whichType;
    if (!Solver(keystore, fromPubKey, hash, nHashType, txin.scriptSig, whichType))
        return false;

    if (whichType == TX_SCRIPTHASH)
    {
        // Solver returns the subscript that need to be evaluated;
        // the final scriptSig is the signatures from that
        // and then the serialized subscript:
        CScript subscript = txin.scriptSig;

        // Recompute txn hash using subscript in place of scriptPubKey:
        uint256 hash2 = SignatureHash(subscript, txTo, nIn, nHashType);

        txnouttype subType;
        bool fSolved =
            Solver(keystore, subscript, hash2, nHashType, txin.scriptSig, subType) && subType != TX_SCRIPTHASH;
        // Append serialized subscript whether or not it is completely signed:
        txin.scriptSig << static_cast<valtype>(subscript);
        if (!fSolved) return false;
    }

    // Test solution
    return VerifyScript(txin.scriptSig, fromPubKey, txTo, nIn, STANDARD_SCRIPT_VERIFY_FLAGS, 0);
}*/

/*bool SignSignature(const CKeyStore &keystore, const CTransaction& txFrom, CTransaction& txTo, unsigned int nIn, int nHashType)
{
    assert(nIn < txTo.vin.size());
    CTxIn& txin = txTo.vin[nIn];
    assert(txin.prevout.n < txFrom.vout.size());
    assert(txin.prevout.hash == txFrom.GetHash());
    const CTxOut& txout = txFrom.vout[txin.prevout.n];

    return SignSignature(keystore, txout.scriptPubKey, txTo, nIn, nHashType);
}*/

// Valid signature cache, to avoid doing expensive ECDSA signature checking
// twice for every transaction (once when accepted into memory pool, and
// again when accepted into the block chain)

bool CheckSig(vector<unsigned char> vchSig, const vector<unsigned char> &vchPubKey, const CScript &scriptCode,
    const CTransaction& txTo, unsigned int nIn, int nHashType, int flags)
{
    static CSignatureCache signatureCache;

    CPubKey pubkey(vchPubKey);
    if (!pubkey.IsValid())
    return false;

    // Hash type is one byte tacked on to the end of the signature
    if (vchSig.empty())
    return false;
    if (nHashType == 0)
    nHashType = vchSig.back();
    else if (nHashType != vchSig.back())
    return false;
    vchSig.pop_back();

    uint256 sighash = SignatureHash(scriptCode, txTo, nIn, nHashType);

    if (signatureCache.Get(sighash, vchSig, pubkey))
    return true;

    if (!pubkey.Verify(sighash, vchSig))
    return false;

    if (!(flags & SCRIPT_VERIFY_NOCACHE))
    signatureCache.Set(sighash, vchSig, pubkey);

    return true;
}

static CScript PushAll(const vector<valtype>& values)
{
    CScript result;
    BOOST_FOREACH(const valtype& v, values)
        result << v;
    return result;
}

/*
static CScript PushAll(const std::vector<valtype>& values)
{
    CScript result;
    for (const valtype& v : values) {
        if (v.size() == 0) {
            result << OP_0;
        } else if (v.size() == 1 && v[0] >= 1 && v[0] <= 16) {
            result << CScript::EncodeOP_N(v[0]);
        } else {
            result << v;
        }
    }
    return result;
}
*/

static CScript CombineMultisig(CScript scriptPubKey, const CTransaction& txTo, unsigned int nIn,
                               const vector<valtype>& vSolutions,
                               vector<valtype>& sigs1, vector<valtype>& sigs2)
{
    // Combine all the signatures we've got:
    set<valtype> allsigs;
    BOOST_FOREACH(const valtype& v, sigs1)
    {
        if (!v.empty())
            allsigs.insert(v);
    }
    BOOST_FOREACH(const valtype& v, sigs2)
    {
        if (!v.empty())
            allsigs.insert(v);
    }

    // Build a map of pubkey -> signature by matching sigs to pubkeys:
    assert(vSolutions.size() > 1);
    unsigned int nSigsRequired = vSolutions.front()[0];
    unsigned int nPubKeys = vSolutions.size()-2;
    map<valtype, valtype> sigs;
    BOOST_FOREACH(const valtype& sig, allsigs)
    {
        for (unsigned int i = 0; i < nPubKeys; i++)
        {
            const valtype& pubkey = vSolutions[i+1];
            if (sigs.count(pubkey))
                continue; // Already got a sig for this pubkey

            if (CheckSig(sig, pubkey, scriptPubKey, txTo, nIn, 0, 0))
            {
                sigs[pubkey] = sig;
                break;
            }
        }
    }
    // Now build a merged CScript:
    unsigned int nSigsHave = 0;
    CScript result; result << OP_0; // pop-one-too-many workaround
    for (unsigned int i = 0; i < nPubKeys && nSigsHave < nSigsRequired; i++)
    {
        if (sigs.count(vSolutions[i+1]))
        {
            result << sigs[vSolutions[i+1]];
            ++nSigsHave;
        }
    }
    // Fill any missing with OP_0:
    for (unsigned int i = nSigsHave; i < nSigsRequired; i++)
        result << OP_0;

    return result;
}

static CScript CombineSignatures(CScript scriptPubKey, const CTransaction& txTo, unsigned int nIn,
                                 const txnouttype txType, const vector<valtype>& vSolutions,
                                 vector<valtype>& sigs1, vector<valtype>& sigs2)
{
    switch (txType)
    {
    case TX_NONSTANDARD:
    case TX_NULL_DATA:
        // Don't know anything about this, assume bigger one is correct:
        if (sigs1.size() >= sigs2.size())
            return PushAll(sigs1);
        return PushAll(sigs2);
    case TX_PUBKEY:
    case TX_PUBKEYHASH:
        // Signatures are bigger than placeholders or empty scripts:
        if (sigs1.empty() || sigs1[0].empty())
            return PushAll(sigs2);
        return PushAll(sigs1);
    case TX_SCRIPTHASH:
        if (sigs1.empty() || sigs1.back().empty())
            return PushAll(sigs2);
        else if (sigs2.empty() || sigs2.back().empty())
            return PushAll(sigs1);
        else
        {
            // Recur to combine:
            valtype spk = sigs1.back();
            CScript pubKey2(spk.begin(), spk.end());

            txnouttype txType2;
            vector<vector<unsigned char> > vSolutions2;
            Solver(pubKey2, txType2, vSolutions2);
            sigs1.pop_back();
            sigs2.pop_back();
            CScript result = CombineSignatures(pubKey2, txTo, nIn, txType2, vSolutions2, sigs1, sigs2);
            result << spk;
            return result;
        }
    case TX_MULTISIG:
        return CombineMultisig(scriptPubKey, txTo, nIn, vSolutions, sigs1, sigs2);
    }

    return CScript();
}

CScript CombineSignatures(CScript scriptPubKey, const CTransaction& txTo, unsigned int nIn,
                          const CScript& scriptSig1, const CScript& scriptSig2)
{
    txnouttype txType;
    vector<vector<unsigned char> > vSolutions;
    Solver(scriptPubKey, txType, vSolutions);

    vector<valtype> stack1;
    EvalScript(stack1, scriptSig1, CTransaction(), 0, SCRIPT_VERIFY_NONE, 0);
    vector<valtype> stack2;
    EvalScript(stack2, scriptSig2, CTransaction(), 0, SCRIPT_VERIFY_NONE, 0);

    return CombineSignatures(scriptPubKey, txTo, nIn, txType, vSolutions, stack1, stack2);
}

bool ProduceSignature(const BaseSignatureCreator& creator, const CScript& fromPubKey, SignatureData& sigdata)
{
    CScript script = fromPubKey;
    std::vector<valtype> result;
    txnouttype whichType;
    bool solved = SignStep(creator, script, result, whichType);
    bool P2SH = false;
    CScript subscript;
//    sigdata.scriptWitness.stack.clear();

    if (solved && whichType == TX_SCRIPTHASH)
    {
        // Solver returns the subscript that needs to be evaluated;
        // the final scriptSig is the signatures from that
        // and then the serialized subscript:
        script = subscript = CScript(result[0].begin(), result[0].end());
        solved = solved && SignStep(creator, script, result, whichType) && whichType != TX_SCRIPTHASH;
        P2SH = true;
    }

    if (P2SH) {
        result.push_back(std::vector<unsigned char>(subscript.begin(), subscript.end()));
    }
    sigdata.scriptSig = PushAll(result);

    // Test solution
    return solved && VerifyScript(sigdata.scriptSig, fromPubKey, STANDARD_SCRIPT_VERIFY_FLAGS, creator.Checker());
}

SignatureData DataFromTransaction(const CMutableTransaction& tx, unsigned int nIn)
{
    SignatureData data;
    assert(tx.vin.size() > nIn);
    data.scriptSig = tx.vin[nIn].scriptSig;
//    data.scriptWitness = tx.vin[nIn].scriptWitness;
    return data;
}

static std::vector<valtype> CombineMultisig(const CScript& scriptPubKey, const BaseSignatureChecker& checker,
                               const std::vector<valtype>& vSolutions,
                               const std::vector<valtype>& sigs1, const std::vector<valtype>& sigs2)
{
    // Combine all the signatures we've got:
    std::set<valtype> allsigs;
    for (const valtype& v : sigs1)
    {
        if (!v.empty())
            allsigs.insert(v);
    }
    for (const valtype& v : sigs2)
    {
        if (!v.empty())
            allsigs.insert(v);
    }

    // Build a map of pubkey -> signature by matching sigs to pubkeys:
    assert(vSolutions.size() > 1);
    unsigned int nSigsRequired = vSolutions.front()[0];
    unsigned int nPubKeys = vSolutions.size()-2;
    std::map<valtype, valtype> sigs;
    for (const valtype& sig : allsigs)
    {
        for (unsigned int i = 0; i < nPubKeys; i++)
        {
            const valtype& pubkey = vSolutions[i+1];
            if (sigs.count(pubkey))
                continue; // Already got a sig for this pubkey

            if (checker.CheckSig(sig, pubkey, scriptPubKey))
            {
                sigs[pubkey] = sig;
                break;
            }
        }
    }
    // Now build a merged CScript:
    unsigned int nSigsHave = 0;
    std::vector<valtype> result; result.push_back(valtype()); // pop-one-too-many workaround
    for (unsigned int i = 0; i < nPubKeys && nSigsHave < nSigsRequired; i++)
    {
        if (sigs.count(vSolutions[i+1]))
        {
            result.push_back(sigs[vSolutions[i+1]]);
            ++nSigsHave;
        }
    }
    // Fill any missing with OP_0:
    for (unsigned int i = nSigsHave; i < nSigsRequired; i++)
        result.push_back(valtype());

    return result;
}

namespace
{
struct Stacks
{
    std::vector<valtype> script;
    std::vector<valtype> witness;

    Stacks() {}
    explicit Stacks(const std::vector<valtype>& scriptSigStack_) : script(scriptSigStack_), witness() {}
    explicit Stacks(const SignatureData& data) : witness() {
        EvalScript(script, data.scriptSig, SCRIPT_VERIFY_STRICTENC, BaseSignatureChecker());
    }

    SignatureData Output() const {
        SignatureData result;
        result.scriptSig = PushAll(script);
//        result.scriptWitness.stack = witness;
        return result;
    }
};
}

static Stacks CombineSignatures(const CScript& scriptPubKey, const BaseSignatureChecker& checker,
                                 const txnouttype txType, const std::vector<valtype>& vSolutions,
                                 Stacks sigs1, Stacks sigs2)
{
    switch (txType)
    {
    case TX_NONSTANDARD:
    case TX_NULL_DATA:
        // Don't know anything about this, assume bigger one is correct:
        if (sigs1.script.size() >= sigs2.script.size())
            return sigs1;
        return sigs2;
    case TX_PUBKEY:
    case TX_PUBKEYHASH:
        // Signatures are bigger than placeholders or empty scripts:
        if (sigs1.script.empty() || sigs1.script[0].empty())
            return sigs2;
        return sigs1;
    case TX_SCRIPTHASH:
        if (sigs1.script.empty() || sigs1.script.back().empty())
            return sigs2;
        else if (sigs2.script.empty() || sigs2.script.back().empty())
            return sigs1;
        else
        {
            // Recur to combine:
            valtype spk = sigs1.script.back();
            CScript pubKey2(spk.begin(), spk.end());

            txnouttype txType2;
            std::vector<std::vector<unsigned char> > vSolutions2;
            Solver(pubKey2, txType2, vSolutions2);
            sigs1.script.pop_back();
            sigs2.script.pop_back();
            Stacks result = CombineSignatures(pubKey2, checker, txType2, vSolutions2, sigs1, sigs2);
            result.script.push_back(spk);
            return result;
        }
    case TX_MULTISIG:
        return Stacks(CombineMultisig(scriptPubKey, checker, vSolutions, sigs1.script, sigs2.script));
    default:
        return Stacks();
    }
}

SignatureData CombineSignatures(const CScript& scriptPubKey, const BaseSignatureChecker& checker,
                          const SignatureData& scriptSig1, const SignatureData& scriptSig2)
{
    txnouttype txType;
    std::vector<std::vector<unsigned char> > vSolutions;
    Solver(scriptPubKey, txType, vSolutions);

    return CombineSignatures(scriptPubKey, checker, txType, vSolutions, Stacks(scriptSig1), Stacks(scriptSig2)).Output();
}
