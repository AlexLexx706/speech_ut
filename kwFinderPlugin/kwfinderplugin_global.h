#ifndef KWFINDERPLUGIN_GLOBAL_H
#define KWFINDERPLUGIN_GLOBAL_H

#include <QGlobal.h>

#ifdef KWFINDERPLUGIN_LIB
# define KWFINDERPLUGIN_EXPORT Q_DECL_EXPORT
#else
# define KWFINDERPLUGIN_EXPORT Q_DECL_IMPORT
#endif

#endif // KWFINDERPLUGIN_GLOBAL_H