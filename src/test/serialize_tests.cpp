// Copyright (c) 2012-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "serialize.h"
#include "hash.h"
#include "test/test_sphinxcoin.h"

#include <stdint.h>
#include <string>
#include <vector>

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(serialize_tests, BasicTestingSetup)

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(varints)
{
    // encode

    CDataStream ss(SER_DISK, 0);
    CDataStream::size_type size = 0;
    for (int i = 0; i < 100000; i++) {
        ss << VARINT(i);
        size += ::GetSerializeSize(VARINT(i), 0, 0);
        BOOST_CHECK(size == ss.size());
    }

    for (uint64_t i = 0;  i < 100000000000ULL; i += 999999937) {
        ss << VARINT(i);
        size += ::GetSerializeSize(VARINT(i), 0, 0);
        BOOST_CHECK(size == ss.size());
    }

    // decode
    for (int i = 0; i < 100000; i++) {
        int j = -1;
        ss >> VARINT(j);
        BOOST_CHECK_MESSAGE(i == j, "decoded:" << j << " expected:" << i);
    }

    for (uint64_t i = 0;  i < 100000000000ULL; i += 999999937) {
        uint64_t j = -1;
        ss >> VARINT(j);
        BOOST_CHECK_MESSAGE(i == j, "decoded:" << j << " expected:" << i);
    }
}

BOOST_AUTO_TEST_SUITE_END()
