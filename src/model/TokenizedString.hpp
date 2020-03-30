#include <QStringList>

#include <algorithm>

class TokenizedString : public QStringList
{
    constexpr static auto SEPARATOR = ';';

public:
    TokenizedString() = default;

    TokenizedString(const TokenizedString&) = default;
    TokenizedString(TokenizedString&&) = default;
    TokenizedString& operator=(const TokenizedString&) = default;
    TokenizedString& operator=(TokenizedString&&) = default;

    TokenizedString(const QString& str)
        : QStringList{str.split(SEPARATOR, QString::SkipEmptyParts)}
    {
    }

    TokenizedString(const QStringList src) : QStringList{std::move(src)}
    {
        normalize();
    }

    operator QString() const { return QStringList::join(SEPARATOR); }

    // split by separator | sort | unique:
    void normalize()
    {
        auto additional = QStringList{};
        for (auto& token : *this)
        {
            if (token.contains(SEPARATOR))
            {
                auto parts = token.split(SEPARATOR, QString::SkipEmptyParts);
                if (!parts.empty())
                {
                    token = std::move(parts.first());
                    parts.removeFirst();
                    additional << parts;
                }
                else
                    token = "";
            }
        }
        *this << std::move(additional);
        removeAll("");

        sort(Qt::CaseInsensitive);
        erase(std::unique(begin(), end()), end());
    }

    QString buildString(const char* prefix, const char* postfix) const
    {
        auto res = QString{prefix};
        res += QStringList::join(postfix + QString{prefix});
        res += postfix;
        return res;
    }
};
