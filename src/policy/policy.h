// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SPHX_POLICY_POLICY_H
#define SPHX_POLICY_POLICY_H

#include "consensus/consensus.h"
#include "uint256.h"
#include "amount.h"

#include <string>
#include <map>

class CCoinsViewCache;
class CTxOut;
class CTxIndex;
class CTransaction;

typedef std::map<uint256, std::pair<CTxIndex, CTransaction> > MapPrevTx;

/** The maximum size for mined blocks */
static const unsigned int MAX_BLOCK_SIZE_GEN = MAX_BLOCK_SIZE/2;
/** Default for -blockprioritysize, maximum space for zero/low-fee transactions **/
static const unsigned int DEFAULT_BLOCK_PRIORITY_SIZE = 50000;
/** The maximum size for transactions we're willing to relay/mine **/
static const unsigned int MAX_STANDARD_TX_SIZE = MAX_BLOCK_SIZE_GEN/5;
/** Maxiumum number of signature check operations in an IsStandard() P2SH script */
static const unsigned int MAX_P2SH_SIGOPS = 15;
/** The maximum number of sigops we're willing to relay/mine in a single tx */
static const unsigned int MAX_TX_SIGOPS = MAX_BLOCK_SIGOPS/5;
/** Fees smaller than this (in satoshi) are considered zero fee (for transaction creation) */
static const int64_t MIN_TX_FEE = 0.0001*COIN;
/** Fees smaller than this (in satoshi) are considered zero fee (for relaying) */
static const int64_t MIN_RELAY_TX_FEE = MIN_TX_FEE;

/** Script verification flags */
enum
{
    SCRIPT_VERIFY_NONE      = 0,
    SCRIPT_VERIFY_NOCACHE   = (1U << 0),
    // Evaluate P2SH subscripts (softfork safe, BIP16).
    SCRIPT_VERIFY_P2SH      = (1U << 0),

    // Passing a non-strict-DER signature or one with undefined hashtype to a checksig operation causes script failure.
    // Evaluating a pubkey that is not (0x04 + 64 bytes) or (0x02 or 0x03 + 32 bytes) by checksig causes script failure.
    // (softfork safe, but not used or intended as a consensus rule).
    SCRIPT_VERIFY_STRICTENC = (1U << 1),

    // Passing a non-strict-DER signature to a checksig operation causes script failure (softfork safe, BIP62 rule 1)
    SCRIPT_VERIFY_DERSIG    = (1U << 2),

    // Passing a non-strict-DER signature or one with S > order/2 to a checksig operation causes script failure
    // (softfork safe, BIP62 rule 5).
    SCRIPT_VERIFY_LOW_S     = (1U << 3),

    // verify dummy stack item consumed by CHECKMULTISIG is of zero-length (softfork safe, BIP62 rule 7).
    SCRIPT_VERIFY_NULLDUMMY = (1U << 4),

    // Using a non-push operator in the scriptSig causes script failure (softfork safe, BIP62 rule 2).
    SCRIPT_VERIFY_SIGPUSHONLY = (1U << 5),

    // Require minimal encodings for all push operations (OP_0... OP_16, OP_1NEGATE where possible, direct
    // pushes up to 75 bytes, OP_PUSHDATA up to 255 bytes, OP_PUSHDATA2 for anything larger). Evaluating
    // any other push causes the script to fail (BIP62 rule 3).
    // In addition, whenever a stack element is interpreted as a number, it must be of minimal length (BIP62 rule 4).
    // (softfork safe)
    SCRIPT_VERIFY_MINIMALDATA = (1U << 6),

    // Discourage use of NOPs reserved for upgrades (NOP1-10)
    //
    // Provided so that nodes can avoid accepting or mining transactions
    // containing executed NOP's whose meaning may change after a soft-fork,
    // thus rendering the script invalid; with this flag set executing
    // discouraged NOPs fails the script. This verification flag will never be
    // a mandatory flag applied to scripts in a block. NOPs that are not
    // executed, e.g.  within an unexecuted IF ENDIF block, are *not* rejected.
    SCRIPT_VERIFY_DISCOURAGE_UPGRADABLE_NOPS  = (1U << 7)

};

// Mandatory script verification flags that all new blocks must comply with for
// them to be valid. (but old blocks may not comply with)
//
// Failing one of these tests may trigger a DoS ban - see ConnectInputs() for
// details.
static const unsigned int MANDATORY_SCRIPT_VERIFY_FLAGS = SCRIPT_VERIFY_NONE;

// Standard script verification flags that standard transactions will comply
// with. However scripts violating these flags may still be present in valid
// blocks and we must accept those blocks.
static const unsigned int STANDARD_SCRIPT_VERIFY_FLAGS = MANDATORY_SCRIPT_VERIFY_FLAGS |
SCRIPT_VERIFY_STRICTENC |
SCRIPT_VERIFY_NULLDUMMY |
SCRIPT_VERIFY_DISCOURAGE_UPGRADABLE_NOPS;

// For convenience, standard but not mandatory verify flags.
static const unsigned int STANDARD_NOT_MANDATORY_VERIFY_FLAGS = STANDARD_SCRIPT_VERIFY_FLAGS & ~MANDATORY_SCRIPT_VERIFY_FLAGS;

enum txnouttype
{
    TX_NONSTANDARD,
    // 'standard' transaction types:
    TX_PUBKEY,
    TX_PUBKEYHASH,
    TX_SCRIPTHASH,
    TX_MULTISIG,
    TX_NULL_DATA,
};

bool IsStandard(const CScript& scriptPubKey, txnouttype& whichType);

/** Check for standard transaction types
    @return True if all outputs (scriptPubKeys) use only standard transaction forms
*/
bool IsStandardTx(const CTransaction& tx, std::string& reason);

/** Check for standard transaction types
    @param[in] mapInputs    Map of previous transactions that have outputs we're spending
    @return True if all inputs (scriptSigs) use only standard transaction forms
    @see CTransaction::FetchInputs
*/
bool AreInputsStandard(const CTransaction& tx, const MapPrevTx& mapInputs);

#endif // SPHX_POLICY_POLICY_H