// Copyright (c) 2011-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define BOOST_TEST_MODULE Ion Test Suite

#include "net.h"

#include <boost/test/unit_test.hpp>

void Shutdown(void* parg)
{
  exit(EXIT_SUCCESS);
}
