#ifndef CLIENTVERSION_H
#define CLIENTVERSION_H

#if defined(HAVE_CONFIG_H)
#include "config/sphinxcoin-config.h"
#endif

//
// client versioning
//

#if !defined(CLIENT_VERSION_MAJOR) || !defined(CLIENT_VERSION_MINOR) || !defined(CLIENT_VERSION_REVISION) || !defined(CLIENT_VERSION_BUILD) || !defined(CLIENT_VERSION_IS_RELEASE) || !defined(COPYRIGHT_YEAR)
// These need to be macros, as version.cpp's and sphinxcoin-qt.rc's voodoo requires it
#define CLIENT_VERSION_MAJOR       1 
#define CLIENT_VERSION_MINOR       9 
#define CLIENT_VERSION_REVISION    9
#define CLIENT_VERSION_BUILD       1
#endif

#if !defined(CLIENT_VERSION_IS_RELEASE)
// Set to true for release, false for prerelease or test build
#define CLIENT_VERSION_IS_RELEASE  true
#endif

// Converts the parameter X to a string after macro replacement on X has been performed.
// Don't merge these into one macro!
#define STRINGIZE(X) DO_STRINGIZE(X)
#define DO_STRINGIZE(X) #X

//! Copyright string used in Windows .rc files
#define COPYRIGHT_STR "2009-" STRINGIZE(COPYRIGHT_YEAR) " " COPYRIGHT_HOLDERS_FINAL

#endif // CLIENTVERSION_H
