#ifndef QTSTDHASH_H
#define QTSTDHASH_H

#include "qstring.h"
#include "qhash.h"

namespace std
{
template<> struct hash<QString>
{
    std::size_t operator()(const QString& s) const
    {
        return qHash(s);
    }
};
}

#endif // QTSTDHASH_H
