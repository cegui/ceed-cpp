#ifndef QTSTDHASH_H
#define QTSTDHASH_H

#include "qstring.h"
#include "qhash.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
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
#endif

#endif // QTSTDHASH_H
