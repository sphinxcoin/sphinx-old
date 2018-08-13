// Copyright (c) 2015-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ION_TEST_TEST_ION_H
#define ION_TEST_TEST_ION_H

#include "chainparams.h"
#include "key.h"
#include "pubkey.h"
#include "txdb.h"
#include "txmempool.h"

#include <boost/thread.hpp>

/** Basic testing setup.
 * This just configures logging and chain parameters.
 */
struct BasicTestingSetup {
    ECCVerifyHandle globalVerifyHandle;

    BasicTestingSetup(CChainParams::Network chainName = CChainParams::TESTNET);
    ~BasicTestingSetup();
};

/** Testing setup that configures a complete environment.
 * Included are data directory, coins database, script check threads setup.
 */
/*
class CConnman;
struct TestingSetup: public BasicTestingSetup {
    CCoinsViewDB *pcoinsdbview;
    fs::path pathTemp;
    boost::thread_group threadGroup;
    CConnman* connman;

    TestingSetup(const std::string& chainName = CBaseChainParams::MAIN);
    ~TestingSetup();
};
*/

#endif
