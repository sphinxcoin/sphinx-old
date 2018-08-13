# Copyright (c) 2013-2016 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.cpppensource.org/licenses/mit-license.php.

DIST_SUBDIRS = secp256k1 univalue

AM_LDFLAGS = $(PTHREAD_CFLAGS) $(LIBTOOL_LDFLAGS) $(HARDENED_LDFLAGS)
AM_CXXFLAGS = $(HARDENED_CXXFLAGS) $(ERROR_CXXFLAGS)
AM_CPPFLAGS = $(HARDENED_CPPFLAGS)
EXTRA_LIBRARIES = 

if EMBEDDED_UNIVALUE
LIBUNIVALUE = univalue/libunivalue.la

$(LIBUNIVALUE): $(wildcard univalue/lib/*) $(wildcard univalue/include/*)
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C $(@D) $(@F)
else
LIBUNIVALUE = $(UNIVALUE_LIBS)
endif

SPHINXCOIN_CONFIG_INCLUDES=-I$(builddir)/config
SPHINXCOIN_INCLUDES=-I$(builddir) -I$(builddir)/obj $(BDB_CPPFLAGS) $(BOOST_CPPFLAGS) $(LEVELDB_CPPFLAGS) $(CRYPTO_CFLAGS) $(SSL_CFLAGS)

SPHINXCOIN_INCLUDES += -I$(srcdir)/secp256k1/include
SPHINXCOIN_INCLUDES += $(UNIVALUE_CFLAGS)

LIBSPHINXCOIN_SERVER=libsphinxcoin_server.a
LIBSPHINXCOIN_COMMON=libsphinxcoin_common.a
#LIBSPHINXCOIN_CONSENSUS=libsphinxcoin_consensus.a
LIBSPHINXCOIN_CLI=libsphinxcoin_cli.a
LIBSPHINXCOIN_UTIL=libsphinxcoin_util.a
LIBSPHINXCOIN_CRYPTO=crypto/libsphinxcoin_crypto.a
LIBSPHINXCOINQT=qt/libsphinxcoinqt.a
LIBSECP256K1=secp256k1/libsecp256k1.la

# if ENABLE_ZMQ
# LIBSPHINXCOIN_ZMQ=libsphinxcoin_zmq.a
# endif
if BUILD_SPHINXCOIN_LIBS
LIBSPHINXCOINCONSENSUS=libsphinxcoinconsensus.la
endif
if ENABLE_WALLET
LIBSPHINXCOIN_WALLET=libsphinxcoin_wallet.a
endif

$(LIBSECP256K1): $(wildcard secp256k1/src/*) $(wildcard secp256k1/include/*)
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C $(@D) $(@F)

# Make is not made aware of per-object dependencies to avoid limiting building parallelization
# But to build the less dependent modules first, we manually select their order here:
EXTRA_LIBRARIES += \
  $(LIBSPHINXCOIN_CRYPTO) \
  $(LIBSPHINXCOIN_UTIL) \
  $(LIBSPHINXCOIN_COMMON) \
  $(LIBSPHINXCOIN_SERVER) \
  $(LIBSPHINXCOIN_CLI) \
  $(LIBSPHINXCOIN_WALLET) \
  $(LIBSPHINXCOIN_ZMQ)

lib_LTLIBRARIES = $(LIBSPHINXCOINCONSENSUS)

bin_PROGRAMS =
noinst_PROGRAMS =
TESTS =
BENCHMARKS =

if BUILD_SPHINXCOIND
  bin_PROGRAMS += sphinxcoind
endif

if BUILD_SPHINXCOIN_UTILS
  bin_PROGRAMS += sphinxcoin-cli sphinxcoin-tx
#  bin_PROGRAMS += sphinxcoin-cli
endif

.PHONY: FORCE check-symbols check-security
# sphinxcoin core #
SPHINXCOIN_CORE_H = \
  activemasternode.h \
  addrman.h \
  alert.h \
  allocators.h \
  amount.h \
  arith_uint256.h \
  base58.h \
  bignum.h \
  chain.h \
  chainparams.h  \
  chainparamsseeds.h \
  checkpoints.h \
  clientversion.h \
  coincontrol.h \
  coins.h \
  compat.h \
  compressor.h \
  consensus/consensus.h \
  core_io.h  \
  core_memusage.h \
  crypter.h \
  cuckoocache.h \
  db.h \
  ecwrapper.h \
  hash.h \
  hashblock.h \    
  indirectmap.h \
  init.h \
  instantx.h \
  key.h \
  keystore.h \
  limitedmap.h  \
  main.h \
  masternode-payments.h \
  masternode-pos.h \
  masternode.h \
  masternodeconfig.h \
  masternodeman.h \
  memusage.h \
  miner.h \
  mruset.h \
  net.h \
  netbase.h \
  policy/feerate.h \
  policy/policy.h \
  prevector.h \
  primitives/block.h \
  primitives/transaction.h \
  proofs.h \
  protocol.h \
  pubkey.h \
  reverselock.h \
  rpcclient.h \
  rpcprotocol.h \
  rpcserver.h \
  script/script.h \
  script/interpreter.h \
  script/ismine.h \
  script/script_error.h \
  script/sigcache.h \
  script/sign.h \
  script/standard.h \
  serialize.h \
  spork.h \
  stake.h \
  stashedsend-relay.h \
  stashedsend.h \
  stealth.h \
  strlcpy.h \
  support/cleanse.h	\
  sync.h \
  threadsafety.h \
  tinyformat.h \
  torcontrol.h \
  txdb-leveldb.h \
  txdb.h \
  txmempool.h \
  ui_interface.h \
  uint256.h \
  util.h \
  utilstrencodings.h  \
  version.h	\
  wallet.h \
  walletdb.h 

# TODO: check if it's a problem that this could be build after sphinxcoind.o
obj/build.h: FORCE
	@$(MKDIR_P) $(builddir)/obj
	/bin/sh ../share/genbuild.sh obj/build.h
version.cpp: obj/build.h

# server: shared between sphinxcoind and sphinxcoin-qt
libsphinxcoin_server_a_CPPFLAGS = $(AM_CPPFLAGS) $(SPHINXCOIN_INCLUDES) $(MINIUPNPC_CPPFLAGS) $(EVENT_CFLAGS) $(EVENT_PTHREADS_CFLAGS)
libsphinxcoin_server_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libsphinxcoin_server_a_SOURCES = \
	activemasternode.cpp \
	addrman.cpp \
	alert.cpp \
	allocators.cpp \
	chain.cpp \
	checkpoints.cpp \
	crypter.cpp \
	ecwrapper.cpp \
	hash.cpp \
	init.cpp \
	instantx.cpp \
	json/json_spirit.h \
	json/json_spirit_error_position.h \
	json/json_spirit_reader.cpp \
	json/json_spirit_reader.h \
	json/json_spirit_reader_template.h \
	json/json_spirit_stream_reader.h \
	json/json_spirit_utils.h \
	json/json_spirit_value.cpp \
	json/json_spirit_value.h \
	json/json_spirit_writer.cpp \
	json/json_spirit_writer.h \
	json/json_spirit_writer_template.h \
	main.cpp \
	masternode-payments.cpp \
	masternode.cpp \
	masternodeconfig.cpp \
	masternodeman.cpp \
	net.cpp \
	noui.cpp \
	policy/feerate.cpp \
	policy/policy.cpp \
	primitives/block.cpp \
	primitives/transaction.cpp \
	proofs.cpp \
	pubkey.cpp \
	rpcblockchain.cpp \
	rpcmisc.cpp \
	rpcnet.cpp \
	rpcrawtransaction.cpp \
	rpcserver.cpp \
	rpcstashedsend.cpp \
	script/script.cpp \
	script/interpreter.cpp \
	script/ismine.cpp \
	script/script_error.cpp \
	spork.cpp \
	stake.cpp \
	uint256.cpp \
	stashedsend-relay.cpp \
	stashedsend.cpp \
	txdb-leveldb.cpp \
	txmempool.cpp \
	version.cpp \
	$(SPHINXCOIN_CORE_H)

# if ENABLE_ZMQ
# libsphinxcoin_zmq_a_CPPFLAGS = $(SPHINXCOIN_INCLUDES) $(ZMQ_CFLAGS)
# libsphinxcoin_zmq_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
# libsphinxcoin_zmq_a_SOURCES = \
#   zmq/zmqabstractnotifier.cpp \
#   zmq/zmqnotificationinterface.cpp \
#   zmq/zmqpublishnotifier.cpp
# endif

# wallet: shared between sphinxcoind and sphinxcoin-qt, but only linked
# when wallet enabled
libsphinxcoin_wallet_a_CPPFLAGS = $(AM_CPPFLAGS) $(SPHINXCOIN_INCLUDES)
libsphinxcoin_wallet_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libsphinxcoin_wallet_a_SOURCES = \
	db.cpp \
	miner.cpp \
	rpcdump.cpp \
	rpcmining.cpp \
	rpcwallet.cpp \
	stealth.cpp \
	wallet.cpp \
	walletdb.cpp \
	$(SPHINXCOIN_CORE_H)

# crypto primitives library
crypto_libsphinxcoin_crypto_a_CPPFLAGS = $(AM_CPPFLAGS) $(SPHINXCOIN_CONFIG_INCLUDES) $(PIC_FLAGS)
crypto_libsphinxcoin_crypto_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS) $(PIC_FLAGS)
crypto_libsphinxcoin_crypto_a_SOURCES = \
  crypto/common.h \
  crypto/hmac_sha256.cpp \
  crypto/hmac_sha256.h \
  crypto/hmac_sha512.cpp \
  crypto/hmac_sha512.h \
  crypto/ripemd160.cpp \
  crypto/aes_helper.c \
  crypto/blake.c \
  crypto/bmw.c \
  crypto/cubehash.c \
  crypto/echo.c \
  crypto/groestl.c \
  crypto/jh.c \
  crypto/keccak.c \
  crypto/luffa.c \
  crypto/shavite.c \
  crypto/simd.c \
  crypto/skein.c \
  crypto/ripemd160.h \
  crypto/sha1.cpp \
  crypto/sha1.h \
  crypto/sha256.cpp \
  crypto/sha256.h \
  crypto/sha512.h \
  crypto/sph_blake.h \
  crypto/sph_bmw.h \
  crypto/sph_cubehash.h \
  crypto/sph_echo.h \
  crypto/sph_groestl.h \
  crypto/sph_jh.h \
  crypto/sph_keccak.h \
  crypto/sph_luffa.h \
  crypto/sph_shavite.h \
  crypto/sph_simd.h \
  crypto/sph_skein.h \
  crypto/sph_types.h \
  crypto/sha512.cpp \
  crypto/sha512.h

# consensus: shared between all executables that validate any consensus rules.
#libsphinxcoin_consensus_a_CPPFLAGS = $(AM_CPPFLAGS) $(SPHINXCOIN_INCLUDES)
#libsphinxcoin_consensus_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
#libsphinxcoin_consensus_a_SOURCES = 

# common: shared between sphinxcoind, and sphinxcoin-qt and non-server tools
libsphinxcoin_common_a_CPPFLAGS = $(AM_CPPFLAGS) $(SPHINXCOIN_INCLUDES)
libsphinxcoin_common_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libsphinxcoin_common_a_SOURCES = \
	base58.cpp \
	chainparams.cpp \
	coins.cpp \
	compressor.cpp \
	core_read.cpp \
	core_write.cpp \
	key.cpp \
	keystore.cpp \
	netbase.cpp \
	protocol.cpp \
	script/sigcache.cpp \
	script/sign.cpp \
	script/standard.cpp \
	$(SPHINXCOIN_CORE_H)

# util: shared between all executables.
# This library *must* be included to make sure that the glibc
# backward-compatibility objects and their sanity checks are linked.
libsphinxcoin_util_a_CPPFLAGS = $(AM_CPPFLAGS) $(SPHINXCOIN_INCLUDES)
libsphinxcoin_util_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libsphinxcoin_util_a_SOURCES = \
  compat/glibc_sanity.cpp \
  compat/glibcxx_sanity.cpp \
  compat/strnlen.cpp \
  rpcprotocol.cpp \
  support/cleanse.cpp \
  sync.cpp \
  util.cpp \
  utilstrencodings.cpp \
  $(SPHINXCOIN_CORE_H)

if GLIBC_BACK_COMPAT
libsphinxcoin_util_a_SOURCES += compat/glibc_compat.cpp
endif

# cli: shared between sphinxcoin-cli and sphinxcoin-qt
libsphinxcoin_cli_a_CPPFLAGS = $(AM_CPPFLAGS) $(SPHINXCOIN_INCLUDES) 
libsphinxcoin_cli_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
# libsphinxcoin_cli_a_SOURCES = \
#  rpc/client.cpp \
#  $(SPHINXCOIN_CORE_H)
libsphinxcoin_cli_a_SOURCES = \
  rpcclient.cpp \
  $(SPHINXCOIN_CORE_H)

nodist_libsphinxcoin_util_a_SOURCES = $(srcdir)/obj/build.h

# sphinxcoind binary #
sphinxcoind_SOURCES = sphinxcoind.cpp 
sphinxcoind_CPPFLAGS = $(AM_CPPFLAGS) $(SPHINXCOIN_INCLUDES) $(MINIUPNPC_CPPFLAGS) $(EVENT_CFLAGS) $(EVENT_PTHREADS_CFLAGS)
sphinxcoind_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
sphinxcoind_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
sphinxcoind_SOURCES += sphinxcoind-res.rc
endif

sphinxcoind_LDADD = \
  $(LIBSPHINXCOIN_SERVER) \
  $(LIBSPHINXCOIN_COMMON) \
  $(LIBSPHINXCOIN_UTIL) \
  $(LIBSPHINXCOIN_WALLET) \
  $(LIBSPHINXCOIN_CRYPTO) \
  $(LIBLEVELDB) \
  $(LIBMEMENV) \
  $(LIBSECP256K1)

sphinxcoind_LDADD += $(BOOST_LIBS) $(BDB_LIBS) $(SSL_LIBS) $(CRYPTO_LIBS) $(MINIUPNPC_LIBS) $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS) $(ZMQ_LIBS)

# sphinxcoin-cli binary #
sphinxcoin_cli_SOURCES = sphinxcoin-cli.cpp 
sphinxcoin_cli_CPPFLAGS = $(AM_CPPFLAGS) $(SPHINXCOIN_INCLUDES) $(MINIUPNPC_CPPFLAGS) $(EVENT_CFLAGS) $(EVENT_PTHREADS_CFLAGS)
sphinxcoin_cli_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
sphinxcoin_cli_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
sphinxcoin_cli_SOURCES += sphinxcoin-cli-res.rc
endif

sphinxcoin_cli_LDADD = \
  $(LIBSPHINXCOIN_CLI) \
  $(LIBSPHINXCOIN_SERVER) \
  $(LIBSPHINXCOIN_COMMON) \
  $(LIBSPHINXCOIN_UTIL) \
  $(LIBSPHINXCOIN_CRYPTO) \
  $(LIBSPHINXCOIN_WALLET) \
  $(LIBLEVELDB) \
  $(LIBSECP256K1) \
  $(LIBMEMENV) 

sphinxcoin_cli_LDADD += $(BOOST_LIBS) $(BDB_LIBS) $(SSL_LIBS) $(CRYPTO_LIBS) $(MINIUPNPC_LIBS) $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS) $(ZMQ_LIBS)
#sphinxcoin_cli_LDADD += $(BOOST_LIBS) $(BDB_LIBS) $(SSL_LIBS) $(CRYPTO_LIBS) $(MINIUPNPC_LIBS) $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS) $(ZMQ_LIBS)

# sphinxcoin-tx binary #
sphinxcoin_tx_SOURCES = sphinxcoin-tx.cpp 
sphinxcoin_tx_CPPFLAGS = $(AM_CPPFLAGS) $(SPHINXCOIN_INCLUDES) $(MINIUPNPC_CPPFLAGS) $(EVENT_CFLAGS) $(EVENT_PTHREADS_CFLAGS)
sphinxcoin_tx_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
sphinxcoin_tx_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
sphinxcoin_tx_SOURCES += sphinxcoin-tx-res.rc
endif

sphinxcoin_tx_LDADD = \
  $(LIBSPHINXCOIN_SERVER) \
  $(LIBSPHINXCOIN_COMMON) \
  $(LIBUNIVALUE) \
  $(LIBSPHINXCOIN_UTIL) \
  $(LIBSPHINXCOIN_WALLET) \
  $(LIBSPHINXCOIN_CRYPTO) \
  $(LIBLEVELDB) \
  $(LIBMEMENV) \
  $(LIBSECP256K1)

sphinxcoin_tx_LDADD += $(BOOST_LIBS) $(BDB_LIBS) $(SSL_LIBS) $(CRYPTO_LIBS) $(MINIUPNPC_LIBS) $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS) $(ZMQ_LIBS)

# sphinxcoinconsensus library #
if BUILD_SPHINXCOIN_LIBS
# include_HEADERS = script/sphinxcoinconsensus.h
include_HEADERS = 
libsphinxcoinconsensus_la_SOURCES = $(crypto_libsphinxcoin_crypto_a_SOURCES) $(libsphinxcoin_consensus_a_SOURCES)

if GLIBC_BACK_COMPAT
  libsphinxcoinconsensus_la_SOURCES += compat/glibc_compat.cpp
endif

libsphinxcoinconsensus_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined $(RELDFLAGS)
libsphinxcoinconsensus_la_LIBADD = $(LIBSECP256K1)
libsphinxcoinconsensus_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(builddir)/obj -I$(srcdir)/secp256k1/include -DBUILD_SPHINXCOIN_INTERNAL
libsphinxcoinconsensus_la_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)

endif
#

CLEANFILES = $(EXTRA_LIBRARIES)

CLEANFILES += *.gcda *.gcno
CLEANFILES += compat/*.gcda compat/*.gcno
CLEANFILES += consensus/*.gcda consensus/*.gcno
CLEANFILES += crypto/*.gcda crypto/*.gcno
CLEANFILES += policy/*.gcda policy/*.gcno
CLEANFILES += primitives/*.gcda primitives/*.gcno
CLEANFILES += script/*.gcda script/*.gcno
CLEANFILES += support/*.gcda support/*.gcno
CLEANFILES += univalue/*.gcda univalue/*.gcno
CLEANFILES += wallet/*.gcda wallet/*.gcno
CLEANFILES += wallet/test/*.gcda wallet/test/*.gcno
CLEANFILES += zmq/*.gcda zmq/*.gcno

DISTCLEANFILES = obj/build.h

config/sphinxcoin-config.h: config/stamp-h1
	@$(MAKE) -C $(top_builddir) $(subdir)/$(@)
config/stamp-h1: $(top_srcdir)/$(subdir)/config/sphinxcoin-config.h.in $(top_builddir)/config.status
	$(AM_V_at)$(MAKE) -C $(top_builddir) $(subdir)/$(@)
$(top_srcdir)/$(subdir)/config/sphinxcoin-config.h.in:  $(am__configure_deps)
	$(AM_V_at)$(MAKE) -C $(top_srcdir) $(subdir)/config/sphinxcoin-config.h.in

clean-local:
	-$(MAKE) -C secp256k1 clean
	-$(MAKE) -C univalue clean
	-rm -f leveldb/*/*.gcda leveldb/*/*.gcno leveldb/helpers/memenv/*.gcda leveldb/helpers/memenv/*.gcno
	-rm -f config.h
	-rm -rf test/__pycache__

.rc.o:
	@test -f $(WINDRES)
	## FIXME: How to get the appropriate modulename_CPPFLAGS in here?
	$(AM_V_GEN) $(WINDRES) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(CPPFLAGS) -DWINDRES_PREPROC -i $< -o $@

.mm.o:
	$(AM_V_CXX) $(OBJCXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) \
	  $(CPPFLAGS) $(AM_CXXFLAGS) $(QT_INCLUDES) $(AM_CXXFLAGS) $(PIE_FLAGS) $(CXXFLAGS) -c -o $@ $<

check-symbols: $(bin_PROGRAMS)
if GLIBC_BACK_COMPAT
	@echo "Checking glibc back compat..."
	$(AM_V_at) READELF=$(READELF) CPPFILT=$(CPPFILT) $(top_srcdir)/contrib/devtools/symbol-check.py < $(bin_PROGRAMS)
endif

check-security: $(bin_PROGRAMS)
if HARDEN
	@echo "Checking binary security..."
	$(AM_V_at) READELF=$(READELF) OBJDUMP=$(OBJDUMP) $(top_srcdir)/contrib/devtools/security-check.py < $(bin_PROGRAMS)
endif

%.pb.cc %.pb.h: %.proto
	@test -f $(PROTOC)
	$(AM_V_GEN) $(PROTOC) --cpp_out=$(@D) --proto_path=$(<D) $<

if EMBEDDED_LEVELDB
include Makefile.leveldb.include
endif

if ENABLE_TESTS
include Makefile.test.include
endif

if ENABLE_QT
include Makefile.qt.include
endif
