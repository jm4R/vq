#include <QStringList>

#include <algorithm>

class TokenizedString : public QStringList
{
    constexpr static auto SEPARATOR = ';';

public:
    TokenizedString() = default;
    TokenizedString(const QString& str)
        : QStringList{str.split(SEPARATOR, QString::SkipEmptyParts)}
    {
    }

    operator QString() const { return QStringList::join(SEPARATOR); }

    void normalize()
    {
        sort(Qt::CaseInsensitive);
        erase(std::unique(begin(), end()), end());
    }
};
