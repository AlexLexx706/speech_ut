#ifndef VITERBIPLUGIN_GLOBAL_H
#define VITERBIPLUGIN_GLOBAL_H

#include <QGlobal.h>

#ifdef VITERBIPLUGIN_LIB
# define VITERBIPLUGIN_EXPORT Q_DECL_EXPORT
#else
# define VITERBIPLUGIN_EXPORT Q_DECL_IMPORT
#endif

#endif // VITERBIPLUGIN_GLOBAL_H