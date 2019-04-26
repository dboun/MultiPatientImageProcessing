#ifndef WARNING_CRITICAL_H
#define WARNING_CRITICAL_H

#include "WarningInformation.h"

class WarningCritical : public WarningInformation
{
    Q_OBJECT
public:
    using WarningInformation::WarningInformation;
};

#endif // ! WARNING_CRITICAL_H