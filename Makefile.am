# Copyright (c) 2013-2016 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

ACLOCAL_AMFLAGS = -I build-aux/m4
SUBDIRS = src
if ENABLE_MAN
SUBDIRS += doc/man
endif
.PHONY: deploy FORCE

GZIP_ENV="-9n"
export PYTHONPATH

if BUILD_SPHINXCOIN_LIBS
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libsphinxcoinconsensus.pc
endif

SPHINXCOIND_BIN=$(top_builddir)/src/$(SPHINXCOIN_DAEMON_NAME)$(EXEEXT)
SPHINXCOIN_QT_BIN=$(top_builddir)/src/qt/$(SPHINXCOIN_GUI_NAME)$(EXEEXT)
SPHINXCOIN_CLI_BIN=$(top_builddir)/src/$(SPHINXCOIN_CLI_NAME)$(EXEEXT)
SPHINXCOIND_WIN_INSTALLER=$(PACKAGE)-$(PACKAGE_VERSION)-win$(WINDOWS_BITS)-setup$(EXEEXT)

empty :=
space := $(empty) $(empty)

OSX_APP=Sphinxcoin-Qt.app
OSX_VOLNAME = $(subst $(space),-,$(PACKAGE_NAME))
OSX_DMG = $(OSX_VOLNAME).dmg
OSX_DSSTORE_GEN=$(top_srcdir)/contrib/macdeploy/custom_dsstore.py
OSX_DEPLOY_SCRIPT=$(top_srcdir)/contrib/macdeploy/macdeployqtplus
OSX_FANCY_PLIST=$(top_srcdir)/contrib/macdeploy/fancy.plist
OSX_INSTALLER_ICONS=$(top_srcdir)/src/qt/res/icons/sphinxcoin.icns
OSX_PLIST=$(top_builddir)/share/qt/Info.plist #not installed
OSX_QT_TRANSLATIONS = da,de,es,hu,ru,uk,zh_CN,zh_TW

DIST_DOCS = $(wildcard doc/*.md) $(wildcard doc/release-notes/*.md)
DIST_CONTRIB = $(top_srcdir)/contrib/sphinxcoin-cli.bash-completion \
		$(top_srcdir)/contrib/sphinxcoin-tx.bash-completion \
		$(top_srcdir)/contrib/sphinxcoind.bash-completion \
		$(top_srcdir)/contrib/init \
		$(top_srcdir)/contrib/rpm

BIN_CHECKS=$(top_srcdir)/contrib/devtools/symbol-check.py \
		$(top_srcdir)/contrib/devtools/security-check.py

WINDOWS_PACKAGING = $(top_srcdir)/share/pixmaps/sphinxcoin.ico \
	$(top_srcdir)/share/pixmaps/nsis-header.bmp \
	$(top_srcdir)/share/pixmaps/nsis-wizard.bmp \
	$(top_srcdir)/doc/README_windows.txt

OSX_PACKAGING = $(OSX_DEPLOY_SCRIPT) $(OSX_FANCY_PLIST) $(OSX_INSTALLER_ICONS) \
	$(top_srcdir)/contrib/macdeploy/background.png \
	$(OSX_DSSTORE_GEN) \
	$(top_srcdir)/contrib/macdeploy/detached-sig-apply.sh \
	$(top_srcdir)/contrib/macdeploy/detached-sig-create.sh

COVERAGE_INFO = baseline_filtered_combined.info baseline.info \
	leveldb_baseline.info test_sphinxcoin_filtered.info total_coverage.info \
	baseline_filtered.info functional_test.info functional_test_filtered.info \
	leveldb_baseline_filtered.info test_sphinxcoin_coverage.info test_sphinxcoin.info

dist-hook:
	-$(GIT) archive --format=tar HEAD -- src/clientversion.cpp | $(AMTAR) -C $(top_distdir) -xf -

$(SPHINXCOIND_WIN_INSTALLER): all-recursive
	$(MKDIR_P) $(top_builddir)/release
	STRIPPROG="$(STRIP)" $(INSTALL_STRIP_PROGRAM) $(SPHINXCOIND_BIN) $(top_builddir)/release
	STRIPPROG="$(STRIP)" $(INSTALL_STRIP_PROGRAM) $(SPHINXCOIN_QT_BIN) $(top_builddir)/release
	@test -f $(MAKENSIS) && $(MAKENSIS) -V2 $(top_builddir)/share/setup.nsi || \
		echo error: could not build $@
	@echo built $@

$(OSX_APP)/Contents/PkgInfo:
	$(MKDIR_P) $(@D)
	@echo "APPL????" > $@

$(OSX_APP)/Contents/Resources/empty.lproj:
	$(MKDIR_P) $(@D)
	@touch $@ 

$(OSX_APP)/Contents/Info.plist: $(OSX_PLIST)
	$(MKDIR_P) $(@D)
	$(INSTALL_DATA) $< $@

$(OSX_APP)/Contents/Resources/sphinxcoin.icns: $(OSX_INSTALLER_ICONS)
	$(MKDIR_P) $(@D)
	$(INSTALL_DATA) $< $@

$(OSX_APP)/Contents/MacOS/Sphinxcoin-Qt: $(SPHINXCOIN_QT_BIN)
	$(MKDIR_P) $(@D)
	STRIPPROG="$(STRIP)" $(INSTALL_STRIP_PROGRAM)  $< $@

$(OSX_APP)/Contents/Resources/Base.lproj/InfoPlist.strings:
	$(MKDIR_P) $(@D)
	echo '{	CFBundleDisplayName = "$(PACKAGE_NAME)"; CFBundleName = "$(PACKAGE_NAME)"; }' > $@

OSX_APP_BUILT=$(OSX_APP)/Contents/PkgInfo $(OSX_APP)/Contents/Resources/empty.lproj \
	$(OSX_APP)/Contents/Resources/sphinxcoin.icns $(OSX_APP)/Contents/Info.plist \
	$(OSX_APP)/Contents/MacOS/Sphinxcoin-Qt $(OSX_APP)/Contents/Resources/Base.lproj/InfoPlist.strings

osx_volname:
	echo $(OSX_VOLNAME) >$@

if BUILD_DARWIN
$(OSX_DMG): $(OSX_APP_BUILT) $(OSX_PACKAGING)
	$(PYTHON) $(OSX_DEPLOY_SCRIPT) $(OSX_APP) -add-qt-tr $(OSX_QT_TRANSLATIONS) -translations-dir=$(QT_TRANSLATION_DIR) -dmg -fancy $(OSX_FANCY_PLIST) -verbose 2 -volname $(OSX_VOLNAME)

deploydir: $(OSX_DMG)
else
APP_DIST_DIR=$(top_builddir)/dist
APP_DIST_EXTRAS=$(APP_DIST_DIR)/.background/background.png $(APP_DIST_DIR)/.DS_Store $(APP_DIST_DIR)/Applications

$(APP_DIST_DIR)/Applications:
	@rm -f $@
	@cd $(@D); $(LN_S) /Applications $(@F)

$(APP_DIST_EXTRAS): $(APP_DIST_DIR)/$(OSX_APP)/Contents/MacOS/Sphinxcoin-Qt

$(OSX_DMG): $(APP_DIST_EXTRAS)
	$(GENISOIMAGE) -no-cache-inodes -D -l -probe -V "$(OSX_VOLNAME)" -no-pad -r -dir-mode 0755 -apple -o $@ dist

$(APP_DIST_DIR)/.DS_Store: $(OSX_DSSTORE_GEN)
	$(PYTHON) $< "$@" "$(OSX_VOLNAME)"

$(APP_DIST_DIR)/$(OSX_APP)/Contents/MacOS/Sphinxcoin-Qt: $(OSX_APP_BUILT) $(OSX_PACKAGING)
	INSTALLNAMETOOL=$(INSTALLNAMETOOL)  OTOOL=$(OTOOL) STRIP=$(STRIP) $(PYTHON) $(OSX_DEPLOY_SCRIPT) $(OSX_APP) -translations-dir=$(QT_TRANSLATION_DIR) -add-qt-tr $(OSX_QT_TRANSLATIONS) -verbose 2

deploydir: $(APP_DIST_EXTRAS)
endif

if TARGET_DARWIN
appbundle: $(OSX_APP_BUILT)
deploy: $(OSX_DMG)
endif
if TARGET_WINDOWS
deploy: $(SPHINXCOIND_WIN_INSTALLER)
endif

$(SPHINXCOIN_QT_BIN): FORCE
	$(MAKE) -C src qt/$(@F)

$(SPHINXCOIND_BIN): FORCE
	$(MAKE) -C src $(@F)

$(SPHINXCOIN_CLI_BIN): FORCE
	$(MAKE) -C src $(@F)

if USE_LCOV

baseline.info:
	$(LCOV) -c -i -d $(abs_builddir)/src -o $@

baseline_filtered.info: baseline.info
	$(LCOV) -r $< "/usr/include/*" -o $@

leveldb_baseline.info: baseline_filtered.info
	$(LCOV) -c -i -d $(abs_builddir)/src/leveldb -b $(abs_builddir)/src/leveldb -o $@

leveldb_baseline_filtered.info: leveldb_baseline.info
	$(LCOV) -r $< "/usr/include/*" -o $@

baseline_filtered_combined.info: leveldb_baseline_filtered.info baseline_filtered.info
	$(LCOV) -a leveldb_baseline_filtered.info -a baseline_filtered.info -o $@

test_sphinxcoin.info: baseline_filtered_combined.info
	$(MAKE) -C src/ check
	$(LCOV) -c -d $(abs_builddir)/src -t test_sphinxcoin -o $@
	$(LCOV) -z -d $(abs_builddir)/src
	$(LCOV) -z -d $(abs_builddir)/src/leveldb

test_sphinxcoin_filtered.info: test_sphinxcoin.info
	$(LCOV) -r $< "/usr/include/*" -o $@

functional_test.info: test_sphinxcoin_filtered.info
	-@TIMEOUT=15 python test/functional/test_runner.py $(EXTENDED_FUNCTIONAL_TESTS)
	$(LCOV) -c -d $(abs_builddir)/src --t functional-tests -o $@
	$(LCOV) -z -d $(abs_builddir)/src
	$(LCOV) -z -d $(abs_builddir)/src/leveldb

functional_test_filtered.info: functional_test.info
	$(LCOV) -r $< "/usr/include/*" -o $@

test_sphinxcoin_coverage.info: baseline_filtered_combined.info test_sphinxcoin_filtered.info
	$(LCOV) -a baseline_filtered.info -a leveldb_baseline_filtered.info -a test_sphinxcoin_filtered.info -o $@

total_coverage.info: baseline_filtered_combined.info test_sphinxcoin_filtered.info functional_test_filtered.info
	$(LCOV) -a baseline_filtered.info -a leveldb_baseline_filtered.info -a test_sphinxcoin_filtered.info -a functional_test_filtered.info -o $@ | $(GREP) "\%" | $(AWK) '{ print substr($$3,2,50) "/" $$5 }' > coverage_percent.txt

test_sphinxcoin.coverage/.dirstamp:  test_sphinxcoin_coverage.info
	$(GENHTML) -s $< -o $(@D)
	@touch $@

total.coverage/.dirstamp: total_coverage.info
	$(GENHTML) -s $< -o $(@D)
	@touch $@

cov: test_sphinxcoin.coverage/.dirstamp total.coverage/.dirstamp

endif

dist_noinst_SCRIPTS = autogen.sh

EXTRA_DIST = $(top_srcdir)/share/genbuild.sh test/functional/test_runner.py test/functional $(DIST_CONTRIB) $(DIST_DOCS) $(WINDOWS_PACKAGING) $(OSX_PACKAGING) $(BIN_CHECKS)

EXTRA_DIST += \
	test/util/sphinxcoin-util-test.py \
	test/util/data/sphinxcoin-util-test.json \
	test/util/data/blanktxv1.hex \
	test/util/data/blanktxv1.json \
	test/util/data/blanktxv2.hex \
	test/util/data/blanktxv2.json \
	test/util/data/tt-delin1-out.hex \
	test/util/data/tt-delin1-out.json \
	test/util/data/tt-delout1-out.hex \
	test/util/data/tt-delout1-out.json \
	test/util/data/tt-locktime317000-out.hex \
	test/util/data/tt-locktime317000-out.json \
	test/util/data/tx394b54bb.hex \
	test/util/data/txcreate1.hex \
	test/util/data/txcreate1.json \
	test/util/data/txcreate2.hex \
	test/util/data/txcreate2.json \
	test/util/data/txcreatedata1.hex \
	test/util/data/txcreatedata1.json \
	test/util/data/txcreatedata2.hex \
	test/util/data/txcreatedata2.json \
	test/util/data/txcreatedata_seq0.hex \
	test/util/data/txcreatedata_seq0.json \
	test/util/data/txcreatedata_seq1.hex \
	test/util/data/txcreatedata_seq1.json \
	test/util/data/txcreatemultisig1.hex \
	test/util/data/txcreatemultisig1.json \
	test/util/data/txcreatemultisig2.hex \
	test/util/data/txcreatemultisig2.json \
	test/util/data/txcreatemultisig3.hex \
	test/util/data/txcreatemultisig3.json \
	test/util/data/txcreatemultisig4.hex \
	test/util/data/txcreatemultisig4.json \
	test/util/data/txcreateoutpubkey1.hex \
	test/util/data/txcreateoutpubkey1.json \
	test/util/data/txcreateoutpubkey2.hex \
	test/util/data/txcreateoutpubkey2.json \
	test/util/data/txcreateoutpubkey3.hex \
	test/util/data/txcreateoutpubkey3.json \
	test/util/data/txcreatescript1.hex \
	test/util/data/txcreatescript1.json \
	test/util/data/txcreatescript2.hex \
	test/util/data/txcreatescript2.json \
	test/util/data/txcreatescript3.hex \
	test/util/data/txcreatescript3.json \
	test/util/data/txcreatescript4.hex \
	test/util/data/txcreatescript4.json \
	test/util/data/txcreatesignv1.hex \
	test/util/data/txcreatesignv1.json \
	test/util/data/txcreatesignv2.hex

CLEANFILES = $(OSX_DMG) $(SPHINXCOIND_WIN_INSTALLER)

.INTERMEDIATE: $(COVERAGE_INFO)

DISTCHECK_CONFIGURE_FLAGS = --enable-man

clean-local:
	rm -rf coverage_percent.txt test_sphinxcoin.coverage/ total.coverage/ test/tmp/ cache/ $(OSX_APP)
	rm -rf test/functional/__pycache__
