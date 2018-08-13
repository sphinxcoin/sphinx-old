// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin developers
// Copyright (c) 2017 Empinel/The Bitcoin Developers
// Copyright (c) 2017 ION Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "assert.h"

#include "chainparams.h"
#include "main.h"
#include "primitives/block.h"
#include "util.h"
#include "amount.h"
#include "globals.h"

#include <boost/assign/list_of.hpp>

using namespace boost::assign;

struct SeedSpec6 {
    uint8_t addr[16];
    uint16_t port;
};

#include "chainparamsseeds.h"

/**
 * Main network
 */

//! Convert the pnSeeds6 array into usable address objects.
static void convertSeed6(std::vector<CAddress> &vSeedsOut, const SeedSpec6 *data, unsigned int count)
{
    // It'll only connect to one or two seed nodes because once it connects,
    // it'll get a pile of addresses with newer timestamps.
    // Seed nodes are given a random 'last seen time' of between one and two
    // weeks ago.
    const int64_t nOneWeek = 7*24*60*60;
    for (unsigned int i = 0; i < count; i++)
    {
        struct in6_addr ip;
        memcpy(&ip, data[i].addr, sizeof(ip));
        CAddress addr(CService(ip, data[i].port));
        addr.nTime = GetTime() - GetRand(nOneWeek) - nOneWeek;
        vSeedsOut.push_back(addr);
    }
}

class CMainParams : public CChainParams {
public:
    CMainParams() {
        // The message start string is designed to be unlikely to occur in normal data.
        // The characters are rarely used upper ASCII, not valid as UTF-8, and produce
        // a large 4-byte int at any alignment.
        pchMessageStart[0] = 0xd2;
        pchMessageStart[1] = 0x2d;
        pchMessageStart[2] = 0x1c;
        pchMessageStart[3] = 0xe5;
        vAlertPubKey = ParseHex("04cc24ab003c828cdd9cf4db2ebbde8esdfsdfsdsdfsdfsfsdfsdf1cecb3bbfa8b3127fcb9dd9b84d44112080827ed7c49a648af9fe788ff42e316aee665879c553f099e55299d6b54edd7e0");
        nDefaultPort = 39999;
        nRPCPort = 39998;
        nProofOfWorkLimit = CBigNum(~uint256(0) >> 16);
        nProofOfStakeLimit = CBigNum(~uint256(0) >> 20);
        nTargetSpacing = 60;
        nTargetTimespan = 10 * 60;  // 10 mins

        const char* pszTimestamp = PSZ_TIMESTAMP; 
        std::vector<CTxIn> vin;
        vin.resize(1);
        vin[0].scriptSig = CScript() << 0 << CScriptNum(42) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
        std::vector<CTxOut> vout;
        vout.resize(1);
        vout[0].SetEmpty();
        CTransaction txNew(1, GENESIS_nTIME, vin, vout, 0);
        genesis.vtx.push_back(txNew);
        genesis.hashPrevBlock = 0;
        genesis.hashMerkleRoot = genesis.BuildMerkleTree();
        genesis.nVersion = 1;
        genesis.nTime = GENESIS_nTIME;
        genesis.nBits = nProofOfWorkLimit.GetCompact();
        genesis.nNonce = GENESIS_nNONCE;

        hashGenesisBlock = genesis.GetHash();
/*
	for(uint32_t ii=0;ii<500000;ii++) {
                genesis.nNonce = ii;
                hashGenesisBlock = genesis.GetHash();
                printf("nNonce = %9u, hash = %s, MerkleRoot = %s\n",ii,hashGenesisBlock.GetHex().c_str(),genesis.hashMerkleRoot.GetHex().c_str());
                if(ii%10000 == 0)
                        printf("%9u\n",ii);
        }
*/
        assert(hashGenesisBlock == uint256(GENESIS_HASH));
        assert(genesis.hashMerkleRoot == uint256(GENESIS_MERKLE));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, PUBKEY_PRE);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, SCRIPT_PRE);
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, SECRET_PRE);
        base58Prefixes[STEALTH_ADDRESS] = std::vector<unsigned char>(1, STEALTH_PRE);
        base58Prefixes[EXT_PUBLIC_KEY] = list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();

        vSeeds.push_back(CDNSSeedData("23.91.97.27", "23.91.97.27"));
	vSeeds.push_back(CDNSSeedData("106.75.99.86","106.75.99.86"));
        
        convertSeed6(vFixedSeeds, pnSeed6_main, ARRAYLEN(pnSeed6_main));

	    nPoolMaxTransactions = 3;
        strStashedsendPoolDummyAddress = DUMMY_ADDRESS;
        nLastPOWBlock = LASTPOWBLOCK;
    }

    virtual const CBlock& GenesisBlock() const { return genesis; }
    virtual Network NetworkID() const { return CChainParams::MAIN; }

    virtual const vector<CAddress>& FixedSeeds() const {
        return vFixedSeeds;
    }
protected:
    CBlock genesis;
    vector<CAddress> vFixedSeeds;
};
static CMainParams mainParams;


//
// Testnet
//

class CTestNetParams : public CMainParams {
public:
    CTestNetParams() {
        // The message start string is designed to be unlikely to occur in normal data.
        // The characters are rarely used upper ASCII, not valid as UTF-8, and produce
        // a large 4-byte int at any alignment.
		pchMessageStart[0] = 0x2f;
		pchMessageStart[1] = 0xca;
		pchMessageStart[2] = 0x4d;
		pchMessageStart[3] = 0x3e;
		nProofOfWorkLimit = CBigNum(~uint256(0) >> 16);
        vAlertPubKey = ParseHex("9as8d7f9a8sd76f90a7df90a8sdfhadf8asdfnhasdfn7as9d8f7awefh9asdf89asd78fhasd89fhasdf789hasdf89");
		nDefaultPort = 27170;
		nRPCPort = 27171;
		strDataDir = "testnet";

		// Modify the testnet genesis block so the timestamp is valid for a later start.
		genesis.nBits = 520159231;
		genesis.nNonce = 80086;

		// assert(hashGenesisBlock == uint256("0x"));

		vFixedSeeds.clear();
		vSeeds.clear();

		base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 97);
		base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 196);
		base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 239);
		base58Prefixes[STEALTH_ADDRESS] = std::vector<unsigned char>(1, 40);
		base58Prefixes[EXT_PUBLIC_KEY] = list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
		base58Prefixes[EXT_SECRET_KEY] = list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();

		convertSeed6(vFixedSeeds, pnSeed6_test, ARRAYLEN(pnSeed6_test));

		nLastPOWBlock = 0x7fffffff;
    }

    virtual const CBlock& GenesisBlock() const { return genesis; }
    virtual Network NetworkID() const { return CChainParams::TESTNET; }

    virtual const vector<CAddress>& FixedSeeds() const {
        return vFixedSeeds;
    }
protected:
    CBlock genesis;
    vector<CAddress> vFixedSeeds;

};
static CTestNetParams testNetParams;


static CChainParams *pCurrentParams = &mainParams;

const CChainParams &Params() {
    return *pCurrentParams;
}

void SelectParams(CChainParams::Network network) {
    switch (network) {
        case CChainParams::MAIN:
            pCurrentParams = &mainParams;
            break;
        case CChainParams::TESTNET:
            pCurrentParams = &testNetParams;
            break;
        default:
            assert(false && "Unimplemented network");
            return;
    }
}

bool SelectParamsFromCommandLine() {
    
    bool fTestNet = GetBoolArg("-testnet", false);
    
    if (fTestNet) {
        SelectParams(CChainParams::TESTNET);
    } else {
        SelectParams(CChainParams::MAIN);
    }
    return true;
}
