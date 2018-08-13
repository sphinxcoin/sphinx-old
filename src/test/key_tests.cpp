// Copyright (c) 2012-2015 The Ion Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "key.h"

#include "base58.h"
#include "script/script.h"
#include "uint256.h"
#include "util.h"
#include "utilstrencodings.h"
#include "test/test_sphinxcoin.h"

#include <string>
#include <vector>

#include <boost/test/unit_test.hpp>

// ION currently doens't support generating uncompressed keys

static const std::string strSecret1     ("923B9CjRTGCcizoQnFxz4CFmr32AxWnoDHjsroKR82GpUVVnE6a");
static const std::string strSecret2     ("928sdFsEbTy4VSRZ43KAnegnumAzxbmXDrjUY4w4NwDBxZTYWFg");
static const std::string strSecret1C    ("cPabmCnEvv6GgN7qxm46q6gCwRvAx2V8yEtFUoSHTzcpYK3sQmRn");
static const std::string strSecret2C    ("cQ1kT2cj182SQXDNURDCCVLddsy8pQTWPkWUY8MbupGoMtS4nosi");
static const CSphinxcoinAddress addr1 ("gPdX9XjivUVeT3KNvZi4fqmSCxc7snkoJi");
static const CSphinxcoinAddress addr2 ("gLHZvB1Geqq3jDhPx2fVcaqeeiETUxptns");
static const CSphinxcoinAddress addr1C("gJC9vSpDbNQkZi9JrYgWxGhLaKUtWSXYzH");
static const CSphinxcoinAddress addr2C("g6Zzx2YE9J9MVcNsMpQDovz8QndFxd8zj9");


static const std::string strAddressBad("1HV9Lc3sNHZxwj4Zk6fB38tEmBryq2cBiF");


BOOST_FIXTURE_TEST_SUITE(key_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(key_test1)
{

/*  Generate new keys as follows:

    CKey newKey1;
    CKey newKey2;
    newKey1.MakeNewKey(false);
    newKey1C.SetPrivKey(newKey1.GetPrivKey(), true); 
    CPubKey newPubKey1 = newKey1.GetPubKey();
    CKeyID newKeyID1 = newPubKey1.GetID();
    CSphinxcoinAddress newAddress1;
    newAddress1.Set(newKeyID1);
    CTxDestination newDest1 = newAddress1.Get();

    printf("HexStr(newKey1.GetPubKey()): [%s]\n", HexStr(newKey1.GetPubKey()).c_str());
    printf("CSphinxcoinSecret(newKey1).ToString()): [%s]\n", CSphinxcoinSecret(newKey1).ToString().c_str());
    printf("newAddress1: [%s]\n", newAddress1.ToString().c_str());    
*/    
    
    CSphinxcoinSecret bsecret1, bsecret2, bsecret1C, bsecret2C, baddress1;
    BOOST_CHECK( bsecret1.SetString (strSecret1));
    BOOST_CHECK( bsecret2.SetString (strSecret2));
    BOOST_CHECK( bsecret1C.SetString(strSecret1C));
    BOOST_CHECK( bsecret2C.SetString(strSecret2C));
    BOOST_CHECK(!baddress1.SetString(strAddressBad));


    CKey key1  = bsecret1.GetKey();
    BOOST_CHECK(key1.IsCompressed() == false);
    CKey key2  = bsecret2.GetKey();
    BOOST_CHECK(key2.IsCompressed() == false);
    CKey key1C = bsecret1C.GetKey();
    BOOST_CHECK(key1C.IsCompressed() == true);
    CKey key2C = bsecret2C.GetKey();
    BOOST_CHECK(key2C.IsCompressed() == true);

    CPubKey pubkey1  = key1. GetPubKey();
    CPubKey pubkey2  = key2. GetPubKey();
    CPubKey pubkey1C = key1C.GetPubKey();
    CPubKey pubkey2C = key2C.GetPubKey();

    BOOST_CHECK(key1.VerifyPubKey(pubkey1));
    BOOST_CHECK(!key1.VerifyPubKey(pubkey1C));
    BOOST_CHECK(!key1.VerifyPubKey(pubkey2));
    BOOST_CHECK(!key1.VerifyPubKey(pubkey2C));

    BOOST_CHECK(!key1C.VerifyPubKey(pubkey1));
    BOOST_CHECK(key1C.VerifyPubKey(pubkey1C));
    BOOST_CHECK(!key1C.VerifyPubKey(pubkey2));
    BOOST_CHECK(!key1C.VerifyPubKey(pubkey2C));

    BOOST_CHECK(!key2.VerifyPubKey(pubkey1));
    BOOST_CHECK(!key2.VerifyPubKey(pubkey1C));
    BOOST_CHECK(key2.VerifyPubKey(pubkey2));
    BOOST_CHECK(!key2.VerifyPubKey(pubkey2C));

    BOOST_CHECK(!key2C.VerifyPubKey(pubkey1));
    BOOST_CHECK(!key2C.VerifyPubKey(pubkey1C));
    BOOST_CHECK(!key2C.VerifyPubKey(pubkey2));
    BOOST_CHECK(key2C.VerifyPubKey(pubkey2C));

    BOOST_CHECK(addr1.Get()  == CTxDestination(pubkey1.GetID()));
    BOOST_CHECK(addr2.Get()  == CTxDestination(pubkey2.GetID()));
    BOOST_CHECK(addr1C.Get() == CTxDestination(pubkey1C.GetID()));
    BOOST_CHECK(addr2C.Get() == CTxDestination(pubkey2C.GetID()));

    for (int n=0; n<16; n++)
    {
        std::string strMsg = strprintf("Very secret message %i: 11", n);
        uint256 hashMsg = Hash(strMsg.begin(), strMsg.end());

        // normal signatures

        std::vector<unsigned char> sign1, sign2, sign1C, sign2C;

        BOOST_CHECK(key1.Sign (hashMsg, sign1));
        BOOST_CHECK(key2.Sign (hashMsg, sign2));
        BOOST_CHECK(key1C.Sign(hashMsg, sign1C));
        BOOST_CHECK(key2C.Sign(hashMsg, sign2C));

        BOOST_CHECK( pubkey1.Verify(hashMsg, sign1));
        BOOST_CHECK(!pubkey1.Verify(hashMsg, sign2));
        BOOST_CHECK( pubkey1.Verify(hashMsg, sign1C));
        BOOST_CHECK(!pubkey1.Verify(hashMsg, sign2C));

        BOOST_CHECK(!pubkey2.Verify(hashMsg, sign1));
        BOOST_CHECK( pubkey2.Verify(hashMsg, sign2));
        BOOST_CHECK(!pubkey2.Verify(hashMsg, sign1C));
        BOOST_CHECK( pubkey2.Verify(hashMsg, sign2C));

        BOOST_CHECK( pubkey1C.Verify(hashMsg, sign1));
        BOOST_CHECK(!pubkey1C.Verify(hashMsg, sign2));
        BOOST_CHECK( pubkey1C.Verify(hashMsg, sign1C));
        BOOST_CHECK(!pubkey1C.Verify(hashMsg, sign2C));

        BOOST_CHECK(!pubkey2C.Verify(hashMsg, sign1));
        BOOST_CHECK( pubkey2C.Verify(hashMsg, sign2));
        BOOST_CHECK(!pubkey2C.Verify(hashMsg, sign1C));
        BOOST_CHECK( pubkey2C.Verify(hashMsg, sign2C));

        // compact signatures (with key recovery)

        std::vector<unsigned char> csign1, csign2, csign1C, csign2C;

        BOOST_CHECK(key1.SignCompact (hashMsg, csign1));
        BOOST_CHECK(key2.SignCompact (hashMsg, csign2));
        BOOST_CHECK(key1C.SignCompact(hashMsg, csign1C));
        BOOST_CHECK(key2C.SignCompact(hashMsg, csign2C));

        CPubKey rkey1, rkey2, rkey1C, rkey2C;

        BOOST_CHECK(rkey1.RecoverCompact (hashMsg, csign1));
        BOOST_CHECK(rkey2.RecoverCompact (hashMsg, csign2));
        BOOST_CHECK(rkey1C.RecoverCompact(hashMsg, csign1C));
        BOOST_CHECK(rkey2C.RecoverCompact(hashMsg, csign2C));

        BOOST_CHECK(rkey1  == pubkey1);
        BOOST_CHECK(rkey2  == pubkey2);
        BOOST_CHECK(rkey1C == pubkey1C);
        BOOST_CHECK(rkey2C == pubkey2C);
    }

    // test deterministic signing

    std::vector<unsigned char> detsig, detsigc;
    std::string strMsg = "Very deterministic message";
    uint256 hashMsg = Hash(strMsg.begin(), strMsg.end());
    BOOST_CHECK(key1.Sign(hashMsg, detsig));
    BOOST_CHECK(key1C.Sign(hashMsg, detsigc));
    BOOST_CHECK(detsig == detsigc);
    BOOST_CHECK(detsig == ParseHex("3044022023fd1400ee487830fb86ea96b1b503871b035b661f654a76706f14226ac6ca08022074e27308eec938199ed844ed10c7b2998ecaeae1530b5017f069da42d30eef65"));
    BOOST_CHECK(key2.Sign(hashMsg, detsig));
    BOOST_CHECK(key2C.Sign(hashMsg, detsigc));
    BOOST_CHECK(detsig == detsigc);
    BOOST_CHECK(detsig == ParseHex("3044022033d60b472c8fafd5886a484c432ce93ceeca029e5830cc259d5898a77b0577e702206f3b97735ca2795a31cc13346b2f89e21f718cb9154cf1e819a2ec2eb463be1f"));
    BOOST_CHECK(key1.SignCompact(hashMsg, detsig));
    BOOST_CHECK(key1C.SignCompact(hashMsg, detsigc));
    BOOST_CHECK(detsig == ParseHex("1b23fd1400ee487830fb86ea96b1b503871b035b661f654a76706f14226ac6ca0874e27308eec938199ed844ed10c7b2998ecaeae1530b5017f069da42d30eef65"));
    BOOST_CHECK(detsigc == ParseHex("1f23fd1400ee487830fb86ea96b1b503871b035b661f654a76706f14226ac6ca0874e27308eec938199ed844ed10c7b2998ecaeae1530b5017f069da42d30eef65"));
    BOOST_CHECK(key2.SignCompact(hashMsg, detsig));
    BOOST_CHECK(key2C.SignCompact(hashMsg, detsigc));
    BOOST_CHECK(detsig == ParseHex("1b33d60b472c8fafd5886a484c432ce93ceeca029e5830cc259d5898a77b0577e76f3b97735ca2795a31cc13346b2f89e21f718cb9154cf1e819a2ec2eb463be1f"));
    BOOST_CHECK(detsigc == ParseHex("1f33d60b472c8fafd5886a484c432ce93ceeca029e5830cc259d5898a77b0577e76f3b97735ca2795a31cc13346b2f89e21f718cb9154cf1e819a2ec2eb463be1f"));
    
//    printf("String: [%s]\n", HexStr(detsig).c_str());
}

BOOST_AUTO_TEST_SUITE_END()
