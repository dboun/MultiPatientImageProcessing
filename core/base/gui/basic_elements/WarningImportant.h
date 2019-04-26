#ifndef WARNING_IMPORTANT_H
#define WARNING_IMPORTANT_H

#include "WarningInformation.h"

class WarningImportant : public WarningInformation
{
    Q_OBJECT
public:
    using WarningInformation::WarningInformation;
};

#endif // ! WARNING_IMPORTANT_H