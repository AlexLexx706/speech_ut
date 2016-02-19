#ifndef CNDVITERBIPLUGIN_GLOBAL_H
#define CNDVITERBIPLUGIN_GLOBAL_H

#include <QGlobal.h>

#ifdef CNDVITERBIPLUGIN_LIB
# define FileSeparation_EXPORT Q_DECL_EXPORT
#else
# define FileSeparation_EXPORT Q_DECL_IMPORT
#endif

#endif // CNDVITERBIPLUGIN_GLOBAL_H
