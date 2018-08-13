// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef SPHX_MAIN_H
#define SPHX_MAIN_H

#include "proofs.h"
#include "policy/policy.h"
#include "primitives/transaction.h" 
#include "script/standard.h"
#include "sync.h"
#include "txmempool.h"
#include "net.h"
#include "script/script.h"
#include "globals.h"

#include <list>

class CValidationState;

static const int64_t STASHEDSEND_COLLATERAL = (0.01*COIN);
static const int64_t STASHEDSEND_POOL_MAX = (4999.99*COIN);

#define INSTANTX_SIGNATURES_REQUIRED           10
#define INSTANTX_SIGNATURES_TOTAL              15


class CBlock;
class CBlockIndex;
class CBlockLocator;
class CInv;
class CKeyItem;
class CNode;
class CReserveKey;
class CWallet;

// Will go to net_processing.h
/** The maximum number of orphan transactions kept in memory */
static const unsigned int MAX_ORPHAN_TRANSACTIONS = MAX_BLOCK_SIZE/100;
/** Default for -maxorphanblocks, maximum number of orphan blocks kept in memory */
static const unsigned int DEFAULT_MAX_ORPHAN_BLOCKS = 10000;

// Will go to validation.h
/** Number of blocks that can be requested at any given time from a single peer. */
static const int MAX_BLOCKS_IN_TRANSIT_PER_PEER = 128;
/** Timeout in seconds before considering a block download peer unresponsive. */
static const unsigned int BLOCK_DOWNLOAD_TIMEOUT = 60;

static const int64_t DRIFT = 30;
inline int64_t FutureDrift(int64_t nTime) { return nTime + DRIFT; }

/** "reject" message codes **/
static const unsigned char REJECT_INVALID = 0x10;

inline int64_t GetMNCollateral(int nHeight) { return MASTERNODE_VALUE; } // *COIN is added with usage implements

extern CScript COINBASE_FLAGS;
extern CCriticalSection cs_main;
extern CTxMemPool mempool;
extern std::map<uint256, CBlockIndex*> mapBlockIndex;
extern std::set<std::pair<COutPoint, unsigned int> > setStakeSeen;
extern CBlockIndex* pindexGenesisBlock;
extern unsigned int nStakeMinAge;
extern unsigned int nNodeLifespan;
extern int nCoinbaseMaturity;
extern unsigned int nModifierInterval;
extern int nBestHeight;
extern uint256 nBestChainTrust;
extern uint256 nBestInvalidTrust;
extern uint256 hashBestChain;
extern CBlockIndex* pindexBest;
extern uint64_t nLastBlockTx;
extern uint64_t nLastBlockSize;
extern int64_t nLastCoinStakeSearchInterval;
extern const std::string strMessageMagic;
extern int64_t nTimeBestReceived;
extern bool fImporting;
extern bool fReindex;
struct COrphanBlock;
extern std::map<uint256, COrphanBlock*> mapOrphanBlocks;
extern bool fHaveGUI;

// Settings
extern bool fUseFastIndex;
extern unsigned int nDerivationMethodIndex;

extern bool fLargeWorkForkFound;
extern bool fLargeWorkInvalidChainFound;

// Minimum disk space required - used in CheckDiskSpace()
static const uint64_t nMinDiskSpace = 52428800;

class CReserveKey;
class CTxDB;
class CTxIndex;
class CWalletInterface;
struct CNodeStateStats;

namespace {
    struct CMainSignals {
        // Notifies listeners of updated transaction data (passing hash, transaction, and optionally the block it is found in.
        boost::signals2::signal<void (const CTransaction &, const CBlock *, bool)> SyncTransaction;
        // Notifies listeners of an erased transaction (currently disabled, requires transaction replacement).
        boost::signals2::signal<void (const uint256 &)> EraseTransaction;
        // Notifies listeners of an updated transaction without new data (for now: a coinbase potentially becoming visible).
        boost::signals2::signal<void (const uint256 &)> UpdatedTransaction;
        // Notifies listeners of a new active block chain.
        boost::signals2::signal<void (const CBlockLocator &)> SetBestChain;
        // Notifies listeners about an inventory item being seen on the network.
        boost::signals2::signal<void (const uint256 &)> Inventory;
        // Tells listeners to broadcast their data.
        boost::signals2::signal<void (bool)> Broadcast;
    } g_signals;

} // namespace

/** Register a wallet to receive updates from core */
void RegisterWallet(CWalletInterface* pwalletIn);
/** Unregister a wallet from core */
void UnregisterWallet(CWalletInterface* pwalletIn);
/** Unregister all wallets from core */
void UnregisterAllWallets();
/** Push an updated transaction to all registered wallets */
void SyncWithWallets(const CTransaction& tx, const CBlock* pblock = NULL, bool fConnect = true);
/** Ask wallets to resend their transactions */
void ResendWalletTransactions(bool fForce = false);

/** Register with a network node to receive its signals */
void RegisterNodeSignals(CNodeSignals& nodeSignals);
/** Unregister a network node */
void UnregisterNodeSignals(CNodeSignals& nodeSignals);

void PushGetBlocks(CNode* pnode, CBlockIndex* pindexBegin, uint256 hashEnd);

bool ProcessBlock(CNode* pfrom, CBlock* pblock);
bool CheckDiskSpace(uint64_t nAdditionalBytes=0);
FILE* OpenBlockFile(unsigned int nFile, unsigned int nBlockPos, const char* pszMode="rb");
FILE* AppendBlockFile(unsigned int& nFileRet);
bool LoadBlockIndex(bool fAllowNew=true);
void PrintBlockTree();
bool ProcessMessages(CNode* pfrom);
bool SendMessages(CNode* pto, bool fSendTrickle);
void ThreadImport(std::vector<boost::filesystem::path> vImportFiles);

bool IsInitialBlockDownload();
bool IsConfirmedInNPrevBlocks(const CTxIndex& txindex, const CBlockIndex* pindexFrom, int nMaxDepth, int& nActualDepth);
std::string GetWarnings(std::string strFor);
bool GetTransaction(const uint256 &hash, CTransaction &tx, uint256 &hashBlock);
uint256 WantedByOrphan(const COrphanBlock* pblockOrphan);
const CBlockIndex* GetLastBlockIndex(const CBlockIndex* pindex, bool fProofOfStake);
void ThreadStakeMiner(CWallet *pwallet);


/** (try to) add transaction to memory pool **/
bool AcceptToMemoryPool(CTxMemPool& pool, CTransaction &tx, bool fLimitFree,
                        bool* pfMissingInputs, bool fRejectInsaneFee=false, bool ignoreFees=false);

bool AcceptableInputs(CTxMemPool& pool, const CTransaction &txo, bool fLimitFree,
                        bool* pfMissingInputs, bool fRejectInsaneFee=false, bool isDSTX=false);


bool FindTransactionsByDestination(const CTxDestination &dest, std::vector<uint256> &vtxhash);

int GetInputAge(CTxIn& vin);
int GetInputAgeIX(uint256 nTXHash, CTxIn& vin);
int GetIXConfirmations(uint256 nTXHash);
/** Abort with a message */
bool AbortNode(const std::string &msg, const std::string &userMessage="");
/** Get statistics from node state */
bool GetNodeStateStats(NodeId nodeid, CNodeStateStats &stats);
/** Increase a node's misbehavior score. */
void Misbehaving(NodeId nodeid, int howmuch);


int64_t GetMasternodePayment(int nHeight, int64_t blockValue);

struct CNodeStateStats {
    int nMisbehavior;
};


/** Position on disk for a particular transaction. */
class CDiskTxPos
{
public:
    unsigned int nFile;
    unsigned int nBlockPos;
    unsigned int nTxPos;

    CDiskTxPos()
    {
        SetNull();
    }

    CDiskTxPos(unsigned int nFileIn, unsigned int nBlockPosIn, unsigned int nTxPosIn)
    {
        nFile = nFileIn;
        nBlockPos = nBlockPosIn;
        nTxPos = nTxPosIn;
    }

    IMPLEMENT_SERIALIZE( READWRITE(FLATDATA(*this)); )
    void SetNull() { nFile = (unsigned int) -1; nBlockPos = 0; nTxPos = 0; }
    bool IsNull() const { return (nFile == (unsigned int) -1); }

    friend bool operator==(const CDiskTxPos& a, const CDiskTxPos& b)
    {
        return (a.nFile     == b.nFile &&
                a.nBlockPos == b.nBlockPos &&
                a.nTxPos    == b.nTxPos);
    }

    friend bool operator!=(const CDiskTxPos& a, const CDiskTxPos& b)
    {
        return !(a == b);
    }


    std::string ToString() const
    {
        if (IsNull())
            return "null";
        else
            return strprintf("(nFile=%u, nBlockPos=%u, nTxPos=%u)", nFile, nBlockPos, nTxPos);
    }
};




enum GetMinFee_mode
{
    GMF_BLOCK,
    GMF_RELAY,
    GMF_SEND,
};

int64_t GetMinFee(const CTransaction& tx, unsigned int nBytes, bool fAllowFree, enum GetMinFee_mode mode);


struct CMutableTransaction;

/**
 * Basic transaction serialization format:
 * - int32_t nVersion
 * - std::vector<CTxIn> vin
 * - std::vector<CTxOut> vout
 * - uint32_t nLockTime
 *
 * Extended transaction serialization format (currently removed):
 * - int32_t nVersion
 * - unsigned char dummy = 0x00
 * - unsigned char flags (!= 0)
 * - std::vector<CTxIn> vin
 * - std::vector<CTxOut> vout
 * - if (flags & 1):
 *   - CTxWitness wit;
 * - uint32_t nLockTime
 */
 template<typename Stream, typename TxType>
 inline void UnserializeTransaction(TxType& tx, Stream& s) {
 
     s >> tx.nVersion;
     s >> tx.nTime;
     tx.vin.clear();
     tx.vout.clear();
     s >> tx.vin;
     /* We read a non-empty vin. Assume a normal vout follows. */
     s >> tx.vout;
     s >> tx.nLockTime;
 }
 
 template<typename Stream, typename TxType>
 inline void SerializeTransaction(const TxType& tx, Stream& s) {
 
     s << tx.nVersion;
     s << tx.nTime;
     s << tx.vin;
     s << tx.vout;
     s << tx.nLockTime;
 }

/** Count ECDSA signature operations the old-fashioned (pre-0.6) way
    @return number of sigops this transaction's outputs will produce when spent
    @see CTransaction::FetchInputs
*/
unsigned int GetLegacySigOpCount(const CTransaction& tx);

/** Count ECDSA signature operations in pay-to-script-hash inputs.

    @param[in] mapInputs    Map of previous transactions that have outputs we're spending
    @return maximum number of sigops required to validate this transaction's inputs
    @see CTransaction::FetchInputs
 */
unsigned int GetP2SHSigOpCount(const CTransaction& tx, const MapPrevTx& mapInputs);

inline bool AllowFree(double dPriority)
{
    // Large (in bytes) low-priority (new, small-coin) transactions
    // need a fee.
    return dPriority > COIN * 576 / 250;
}

bool IsFinalTx(const CTransaction &tx, int nBlockHeight = 0, int64_t nBlockTime = 0);

/** A transaction with a merkle branch linking it to the block chain. */
class CMerkleTx : public CTransaction
{
private:
    int GetDepthInMainChainINTERNAL(CBlockIndex* &pindexRet) const;
public:
    uint256 hashBlock;
    std::vector<uint256> vMerkleBranch;
    int nIndex;

    // memory only
    mutable bool fMerkleVerified;


    CMerkleTx()
    {
        Init();
    }

    CMerkleTx(const CTransaction& txIn) : CTransaction(txIn)
    {
        Init();
    }

    void Init()
    {
        hashBlock = 0;
        nIndex = -1;
        fMerkleVerified = false;
    }


    IMPLEMENT_SERIALIZE
    (
        nSerSize += SerReadWrite(s, *(CTransaction*)this, nType, nVersion, ser_action);
        nVersion = this->nVersion;
        READWRITE(hashBlock);
        READWRITE(vMerkleBranch);
        READWRITE(nIndex);
    )

    int SetMerkleBranch(const CBlock* pblock=NULL);

    // Return depth of transaction in blockchain:
    // -1  : not in blockchain, and not in memory pool (conflicted transaction)
    //  0  : in memory pool, waiting to be included in a block
    // >=1 : this many blocks deep in the main chain
    int GetDepthInMainChain(CBlockIndex* &pindexRet, bool enableIX=true) const;
    int GetDepthInMainChain(bool enableIX=true) const { CBlockIndex *pindexRet; return GetDepthInMainChain(pindexRet, enableIX); }
    bool IsInMainChain() const { CBlockIndex *pindexRet; return GetDepthInMainChainINTERNAL(pindexRet) > 0; }
    int GetBlocksToMaturity() const;
    bool AcceptToMemoryPool(bool fLimitFree=true, bool fRejectInsaneFee=true, bool ignoreFees=false);
    int GetTransactionLockSignatures() const;
    bool IsTransactionLockTimedOut() const;
};




/**  A txdb record that contains the disk location of a transaction and the
 * locations of transactions that spend its outputs.  vSpent is really only
 * used as a flag, but having the location is very helpful for debugging.
 */
class CTxIndex
{
public:
    CDiskTxPos pos;
    std::vector<CDiskTxPos> vSpent;

    CTxIndex()
    {
        SetNull();
    }

    CTxIndex(const CDiskTxPos& posIn, unsigned int nOutputs)
    {
        pos = posIn;
        vSpent.resize(nOutputs);
    }

    IMPLEMENT_SERIALIZE
    (
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(pos);
        READWRITE(vSpent);
    )

    void SetNull()
    {
        pos.SetNull();
        vSpent.clear();
    }

    bool IsNull()
    {
        return pos.IsNull();
    }

    friend bool operator==(const CTxIndex& a, const CTxIndex& b)
    {
        return (a.pos    == b.pos &&
                a.vSpent == b.vSpent);
    }

    friend bool operator!=(const CTxIndex& a, const CTxIndex& b)
    {
        return !(a == b);
    }
    int GetDepthInMainChain() const;

};

/** Capture information about block/transaction validation */
class CValidationState {
private:
    enum mode_state {
        MODE_VALID,   //! everything ok
        MODE_INVALID, //! network rule violation (DoS value may be set)
        MODE_ERROR,   //! run-time error
    } mode;
    int nDoS;
    std::string strRejectReason;
    unsigned char chRejectCode;
    bool corruptionPossible;
public:
    CValidationState() : mode(MODE_VALID), nDoS(0), chRejectCode(0), corruptionPossible(false) {}
    bool DoS(int level, bool ret = false,
             unsigned char chRejectCodeIn=0, std::string strRejectReasonIn="",
             bool corruptionIn=false) {
        chRejectCode = chRejectCodeIn;
        strRejectReason = strRejectReasonIn;
        corruptionPossible = corruptionIn;
        if (mode == MODE_ERROR)
            return ret;
        nDoS += level;
        mode = MODE_INVALID;
        return ret;
    }
    bool Invalid(bool ret = false,
                 unsigned char _chRejectCode=0, std::string _strRejectReason="") {
        return DoS(0, ret, _chRejectCode, _strRejectReason);
    }
    bool Error(std::string strRejectReasonIn="") {
        if (mode == MODE_VALID)
            strRejectReason = strRejectReasonIn;
        mode = MODE_ERROR;
        return false;
    }
    bool Abort(const std::string &msg) {
        AbortNode(msg);
        return Error(msg);
    }
    bool IsValid() const {
        return mode == MODE_VALID;
    }
    bool IsInvalid() const {
        return mode == MODE_INVALID;
    }
    bool IsError() const {
        return mode == MODE_ERROR;
    }
    bool IsInvalid(int &nDoSOut) const {
        if (IsInvalid()) {
            nDoSOut = nDoS;
            return true;
        }
        return false;
    }
    bool CorruptionPossible() const {
        return corruptionPossible;
    }
    unsigned char GetRejectCode() const { return chRejectCode; }
    std::string GetRejectReason() const { return strRejectReason; }
};







class CWalletInterface {
protected:
    virtual void SyncTransaction(const CTransaction &tx, const CBlock *pblock, bool fConnect) =0;
    virtual void EraseFromWallet(const uint256 &hash) =0;
    virtual void SetBestChain(const CBlockLocator &locator) =0;
    virtual bool UpdatedTransaction(const uint256 &hash) =0;
    virtual void Inventory(const uint256 &hash) =0;
    virtual void ResendWalletTransactions(bool fForce) =0;
    friend void ::RegisterWallet(CWalletInterface*);
    friend void ::UnregisterWallet(CWalletInterface*);
    friend void ::UnregisterAllWallets();
};

#endif
