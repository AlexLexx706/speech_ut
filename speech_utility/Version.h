#ifndef INTERFACE_VER_H
#define INTERFACE_VER_H

#if defined(NDEBUG) || defined(QT_NO_DEBUG) || !defined(_DEBUG)
	#include "version.auto.inc"
#else
	const int progBuildVersion = 0;
#endif

#endif //INTERFACE_VER_H
