// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SPHX_PRIMITIVES_TRANSACTSPHX_H
#define SPHX_PRIMITIVES_TRANSACTSPHX_H

#include "policy/policy.h"
#include "script/script.h"
#include "serialize.h"
#include "uint256.h"

class CScript;
class CTransaction;
class CTxDB;
class CDiskTxPos;
class CBlockIndex;

/** An outpoint - a combination of a transaction hash and an index n into its vout */
class COutPoint
{
public:
    uint256 hash;
    unsigned int n;

    COutPoint() { SetNull(); }
    COutPoint(uint256 hashIn, unsigned int nIn) { hash = hashIn; n = nIn; }
    IMPLEMENT_SERIALIZE( READWRITE(FLATDATA(*this)); )
    void SetNull() { hash = 0; n = (unsigned int) -1; }
    bool IsNull() const { return (hash == 0 && n == (unsigned int) -1); }

    friend bool operator<(const COutPoint& a, const COutPoint& b)
    {
        return (a.hash < b.hash || (a.hash == b.hash && a.n < b.n));
    }

    friend bool operator==(const COutPoint& a, const COutPoint& b)
    {
        return (a.hash == b.hash && a.n == b.n);
    }

    friend bool operator!=(const COutPoint& a, const COutPoint& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
    std::string ToStringShort() const;

};

/** An inpoint - a combination of a transaction and an index n into its vin */
class CInPoint
{
public:
    CTransaction* ptx;
    unsigned int n;

    CInPoint() { SetNull(); }
    CInPoint(CTransaction* ptxIn, unsigned int nIn) { ptx = ptxIn; n = nIn; }
    void SetNull() { ptx = NULL; n = (unsigned int) -1; }
    bool IsNull() const { return (ptx == NULL && n == (unsigned int) -1); }
};

/** An input of a transaction.  It contains the location of the previous
 * transaction's output that it claims and a signature that matches the
 * output's public key.
 */
class CTxIn
{
public:
    COutPoint prevout;
    CScript scriptSig;
    CScript prevPubKey;
    unsigned int nSequence;

    CTxIn()
    {
        nSequence = std::numeric_limits<unsigned int>::max();
    }

    explicit CTxIn(COutPoint prevoutIn, CScript scriptSigIn=CScript(), unsigned int nSequenceIn=std::numeric_limits<unsigned int>::max());

    explicit CTxIn(uint256 hashPrevTx, unsigned int nOut, CScript scriptSigIn=CScript(), unsigned int nSequenceIn=std::numeric_limits<unsigned int>::max());

    IMPLEMENT_SERIALIZE
    (
        READWRITE(prevout);
        READWRITE(scriptSig);
        READWRITE(nSequence);
    )

    bool IsFinal() const
    {
        return (nSequence == std::numeric_limits<unsigned int>::max());
    }

    friend bool operator==(const CTxIn& a, const CTxIn& b)
    {
        return (a.prevout   == b.prevout &&
                a.scriptSig == b.scriptSig &&
                a.nSequence == b.nSequence);
    }

    friend bool operator!=(const CTxIn& a, const CTxIn& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
};




/** An output of a transaction.  It contains the public key that the next input
 * must be able to sign with to claim it.
 */
class CTxOut
{
public:
    CAmount nValue;
    int nRounds;
    CScript scriptPubKey;

    CTxOut()
    {
        SetNull();
    }

    CTxOut(CAmount nValueIn, CScript scriptPubKeyIn);

    IMPLEMENT_SERIALIZE
    (
        READWRITE(nValue);
        READWRITE(scriptPubKey);
    )

    void SetNull()
    {
        nValue = -1;
        nRounds = -10; // an initial value, should be no way to get this by calculations
        scriptPubKey.clear();
    }

    bool IsNull() const
    {
        return (nValue == -1);
    }

    void SetEmpty()
    {
        nValue = 0;
        scriptPubKey.clear();
    }

    bool IsEmpty() const
    {
        return (nValue == 0 && scriptPubKey.empty());
    }

    uint256 GetHash() const;

    bool IsDust(int64_t MIN_RELAY_TX_FEE) const
    {
        // "Dust" is defined in terms of CTransaction::nMinRelayTxFee,
        // which has units satoshis-per-kilobyte.
        // If you'd pay more than 1/3 in fees
        // to spend something, then we consider it dust.
        // A typical txout is 34 bytes big, and will
        // need a CTxIn of at least 148 bytes to spend,
        // so dust is a txout less than 546 satoshis
        // with default nMinRelayTxFee.
        return ((nValue*1000)/(3*((int)GetSerializeSize(SER_DISK,0)+148)) < MIN_RELAY_TX_FEE);
    }

    friend bool operator==(const CTxOut& a, const CTxOut& b)
    {
        return (a.nValue       == b.nValue &&
                a.nRounds      == b.nRounds &&
                a.scriptPubKey == b.scriptPubKey);
    }

    friend bool operator!=(const CTxOut& a, const CTxOut& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
};

 /** A mutable version of CTransaction. */
 struct CMutableTransaction
 {
     int nVersion;
     unsigned int nTime;
     std::vector<CTxIn> vin;
     std::vector<CTxOut> vout;
     unsigned int nLockTime;
 /*
     int32_t nVersion;
     std::vector<CTxIn> vin;
     std::vector<CTxOut> vout;
     uint32_t nLockTime;
 */
     CMutableTransaction();
     CMutableTransaction(const CTransaction& tx);
 
     template <typename Stream>
     inline void Serialize(Stream& s) const {
         SerializeTransaction(*this, s);
     }
 
 
     template <typename Stream>
     inline void Unserialize(Stream& s) {
         UnserializeTransaction(*this, s);
     }
 
     IMPLEMENT_SERIALIZE
     (
         READWRITE(this->nVersion);
         nVersion = this->nVersion;
         READWRITE(nTime);
         READWRITE(vin);
         READWRITE(vout);
         READWRITE(nLockTime);
     )
 
 /*
     template <typename Stream>
     CMutableTransaction(deserialize_type, Stream& s) {
         Unserialize(s);
     }
 */
     /** Compute the hash of this CMutableTransaction. This is computed on the
      * fly, as opposed to GetHash() in CTransaction, which uses a cached result.
      */
     uint256 GetHash() const;
 
     friend bool operator==(const CMutableTransaction& a, const CMutableTransaction& b)
     {
         return a.GetHash() == b.GetHash();
     }
 
 };
 
/** The basic transaction that is broadcasted on the network and contained in
 * blocks.  A transaction can contain multiple inputs and outputs.
 */
 class CTransaction
 {
 public:
 
     // Default transaction version.
     static const int CURRENT_VERSION=1;
 
     // Changing the default transaction version requires a two step process: first
     // adapting relay policy by bumping MAX_STANDARD_VERSION, and then later date
     // bumping the default CURRENT_VERSION at which point both CURRENT_VERSION and
     // MAX_STANDARD_VERSION will be equal.
     static const int32_t MAX_STANDARD_VERSION=1;
 
     int nVersion;
     unsigned int nTime;
     std::vector<CTxIn> vin;
     std::vector<CTxOut> vout;
     unsigned int nLockTime;
 
     // Denial-of-service detection:
     mutable int nDoS;
     bool DoS(int nDoSIn, bool fIn) const { nDoS += nDoSIn; return fIn; }
 
     CTransaction(){
         SetNull();
     }
 
     CTransaction(int nVersion, unsigned int nTime, const std::vector<CTxIn>& vin, const std::vector<CTxOut>& vout, unsigned int nLockTime)
         : nVersion(nVersion), nTime(nTime), vin(vin), vout(vout), nLockTime(nLockTime), nDoS(0)
     {
     }
 
     /** Convert a CMutableTransaction into a CTransaction. */
     CTransaction(const CMutableTransaction &tx);
     CTransaction(CMutableTransaction &&tx);
 
     IMPLEMENT_SERIALIZE
     (
         READWRITE(this->nVersion);
         nVersion = this->nVersion;
         READWRITE(nTime);
         READWRITE(vin);
         READWRITE(vout);
         READWRITE(nLockTime);
     )
 
     void SetNull()
     {
         nVersion = CTransaction::CURRENT_VERSION;
         nTime = GetAdjustedTime();
         vin.clear();
         vout.clear();
         nLockTime = 0;
         nDoS = 0;  // Denial-of-service prevention
     }
 
     bool IsNull() const
     {
         return (vin.empty() && vout.empty());
     }
 
     uint256 GetHash() const
     {
         return SerializeHash(*this);
     }
 
     bool IsCoinBase() const
     {
         return (vin.size() == 1 && vin[0].prevout.IsNull() && vout.size() >= 1);
     }
 
     bool IsCoinStake() const
     {
         // sphinxcoin: the coin stake transaction is marked with the first output empty
         return (vin.size() > 0 && (!vin[0].prevout.IsNull()) && vout.size() >= 2 && vout[0].IsEmpty());
     }
 
     // Compute priority, given priority of inputs and (optionally) tx size
     double ComputePriority(double dPriorityInputs, unsigned int nTxSize=0) const;
 
     /** Amount of ions spent by this transaction.
         @return sum of all outputs (note: does not include fees)
      */
     int64_t GetValueOut() const
     {
         CAmount nValueOut = 0;
         for (std::vector<CTxOut>::const_iterator it(vout.begin()); it != vout.end(); ++it)
         {
             nValueOut += it->nValue;
             if (!MoneyRange(it->nValue) || !MoneyRange(nValueOut))
                 throw std::runtime_error("CTransaction::GetValueOut(): value out of range");
         }
         return nValueOut;
     }
 
     /** Amount of ions coming in to this transaction
         Note that lightweight clients may not know anything besides the hash of previous transactions,
         so may not be able to calculate this.
 
         @param[in] mapInputs    Map of previous transactions that have outputs we're spending
         @return Sum of value of all inputs (scriptSigs)
         @see CTransaction::FetchInputs
      */
     int64_t GetValueIn(const MapPrevTx& mapInputs) const;
 
     bool ReadFromDisk(CDiskTxPos pos, FILE** pfileRet=NULL);
 
     friend bool operator==(const CTransaction& a, const CTransaction& b)
     {
         return (a.nVersion  == b.nVersion &&
                 a.nTime     == b.nTime &&
                 a.vin       == b.vin &&
                 a.vout      == b.vout &&
                 a.nLockTime == b.nLockTime);
     }
 
     friend bool operator!=(const CTransaction& a, const CTransaction& b)
     {
         return !(a == b);
     }
 
     std::string ToString() const
     {
         std::string str;
         str += IsCoinBase()? "Coinbase" : (IsCoinStake()? "Coinstake" : "CTransaction");
         str += strprintf("(hash=%s, nTime=%d, ver=%d, vin.size=%u, vout.size=%u, nLockTime=%d)\n",
             GetHash().ToString(),
             nTime,
             nVersion,
             vin.size(),
             vout.size(),
             nLockTime);
         for (unsigned int i = 0; i < vin.size(); i++)
             str += "    " + vin[i].ToString() + "\n";
         for (unsigned int i = 0; i < vout.size(); i++)
             str += "    " + vout[i].ToString() + "\n";
         return str;
     }
 
 
     bool ReadFromDisk(CTxDB& txdb, const uint256& hash, CTxIndex& txindexRet);
     bool ReadFromDisk(CTxDB& txdb, COutPoint prevout, CTxIndex& txindexRet);
     bool ReadFromDisk(CTxDB& txdb, COutPoint prevout);
     bool ReadFromDisk(COutPoint prevout);
     bool DisconnectInputs(CTxDB& txdb);
 
     /** Fetch from memory and/or disk. inputsRet keys are transaction hashes.
 
      @param[in] txdb    Transaction database
      @param[in] mapTestPool List of pending changes to the transaction index database
      @param[in] fBlock  True if being called to add a new best-block to the chain
      @param[in] fMiner  True if being called by CreateNewBlock
      @param[out] inputsRet  Pointers to this transaction's inputs
      @param[out] fInvalid   returns true if transaction is invalid
      @return    Returns true if all inputs are in txdb or mapTestPool
      */
     bool FetchInputs(CTxDB& txdb, const std::map<uint256, CTxIndex>& mapTestPool,
                      bool fBlock, bool fMiner, MapPrevTx& inputsRet, bool& fInvalid);
 
     /** Sanity check previous transactions, then, if all checks succeed,
         mark them as spent by this transaction.
 
         @param[in] inputs   Previous transactions (from FetchInputs)
         @param[out] mapTestPool Keeps track of inputs that need to be updated on disk
         @param[in] posThisTx    Position of this transaction on disk
         @param[in] pindexBlock
         @param[in] fBlock   true if called from ConnectBlock
         @param[in] fMiner   true if called from CreateNewBlock
         @return Returns true if all checks succeed
      */
     bool ConnectInputs(CTxDB& txdb, MapPrevTx inputs,
                        std::map<uint256, CTxIndex>& mapTestPool, const CDiskTxPos& posThisTx,
                        const CBlockIndex* pindexBlock, bool fBlock, bool fMiner, unsigned int flags = STANDARD_SCRIPT_VERIFY_FLAGS, bool fValidateSig = true);
     bool CheckTransaction() const;
     bool GetCoinAge(CTxDB& txdb, const CBlockIndex* pindexPrev, uint64_t& nCoinAge) const;
 
     const CTxOut& GetOutputFor(const CTxIn& input, const MapPrevTx& inputs) const;
};
 
typedef std::shared_ptr<const CTransaction> CTransactionRef;
static inline CTransactionRef MakeTransactionRef() { return std::make_shared<const CTransaction>(); }
template <typename Tx> static inline CTransactionRef MakeTransactionRef(Tx&& txIn) { return std::make_shared<const CTransaction>(std::forward<Tx>(txIn)); }

#endif // SPHX_PRIMITIVES_TRANSACTSPHX_H
