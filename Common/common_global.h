#ifndef COMMON_GLOBAL_H
#define COMMON_GLOBAL_H

#include <QGlobal.h>

#ifdef COMMON_LIB
# define COMMON_EXPORT Q_DECL_EXPORT
#else
# define COMMON_EXPORT Q_DECL_IMPORT
#endif

#endif // COMMON_GLOBAL_H
