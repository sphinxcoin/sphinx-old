#include "test_sphinxcoin.h"

#include "chainparams.h"
#include "key.h"
#include "pubkey.h"
#include "txdb.h"
#include "txmempool.h"

#include "test/testutil.h"

#include <memory>

extern bool fPrintToConsole;
extern void noui_connect();

BasicTestingSetup::BasicTestingSetup(CChainParams::Network chainName)
{
        ECC_Start();
        fPrintToDebugLog = false; // don't want to write to debug.log file
        SelectParams(chainName);
        noui_connect();
}

BasicTestingSetup::~BasicTestingSetup()
{
        ECC_Stop();
}