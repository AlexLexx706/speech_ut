#ifndef AUDIOOUTPUTDS_GLOBAL_H
#define AUDIOOUTPUTDS_GLOBAL_H

#include <QGlobal.h>

#ifdef AUDIOOUTPUTDS_LIB
# define AUDIOOUTPUTDS_EXPORT Q_DECL_EXPORT
#else
# define AUDIOOUTPUTDS_EXPORT Q_DECL_IMPORT
#endif

#endif // AUDIOOUTPUTDS_GLOBAL_H
