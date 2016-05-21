#ifndef BASE_H
#define BASE_H

// Local
#include "logmanager.h"

class Base : public LogManager
{
public:
    explicit Base(const QString &logTag, QObject *parent = NULL);
};

#endif // BASE_H
