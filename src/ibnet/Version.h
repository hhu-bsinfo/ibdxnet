//
// Created by on 2/2/18.
//

#ifndef IBNET_VERSION_H
#define IBNET_VERSION_H

#define XSTRINGIFY(a) STRINGIFY(a)
#define STRINGIFY(a) #a

namespace ibnet {

#ifdef IBNET_VERSION
static const char* VERSION = XSTRINGIFY(IBNET_VERSION);
#else
static const char* VERSION = "0.0";
#endif

#ifdef IBNET_GIT_REV
static const char* GIT_REV = XSTRINGIFY(IBNET_GIT_REV);
#else
static const char* GIT_REV = "not available";
#endif

#ifdef IBNET_BUILD_DATE
static const char* BUILD_DATE = XSTRINGIFY(IBNET_BUILD_DATE);
#else
static const char* BUILD_DATE = "0000-00-00 00:00:00";
#endif

}

#undef STRINGIFY
#undef XSTRINGIFY

#endif //IBNET_VERSION_H
