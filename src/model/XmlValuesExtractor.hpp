#pragma once

#include <QDomDocument>
#include <QIODevice>
#include <QHash>

class XmlValuesExtractor
{
public:
    XmlValuesExtractor(QIODevice& device) : _device{device} {}

    QString extractOne(const QStringList path, const QString& attribute = {},
                       const QString defaultVal = {})
    {
        auto values = extract(path, attribute);
        if (values.size() == 1)
            return values[0];
        return defaultVal;
    }

    QStringList extract(const QStringList path, const QString& attribute = {})
    {
        auto emptyCondition = [](const QDomElement&) { return true; };
        return extractIf(emptyCondition, path, attribute);
    }

    template <typename Condition>
    QStringList extractIf(Condition condition, const QStringList path,
                          const QString& attribute = {})
    {
        _device.reset();

        const auto& doc = [&] {
            auto r = QDomDocument{};
            r.setContent(&_device, false);
            return r;
        }();

        if (path.empty() || doc.documentElement().tagName() != path[0])
            return {};

        auto values = valuesByPath(doc.documentElement(), path.cbegin() + 1,
                                   path.cend(), attribute, condition);

        return values;
    }

    template <typename Condition>
    QHash<QString, QString> extractMap(Condition condition,
                                       const QStringList path,
                                       const QString& attribute = {})
    {
        _device.reset();

        const auto& doc = [&] {
            auto r = QDomDocument{};
            r.setContent(&_device, false);
            return r;
        }();

        if (path.empty() || doc.documentElement().tagName() != path[0])
            return {};

        auto values = mapValuesByPath(doc.documentElement(), path.cbegin() + 1,
                                      path.cend(), attribute, condition);

        return values;
    }

private:
    template <typename It, typename Condition>
    QStringList valuesByPath(const QDomElement& e, const It& pathBegin,
                             const It& pathEnd, const QString& attribute,
                             Condition condition)
    {
        if (!condition(e))
            return {};

        if (pathBegin == pathEnd)
        {
            if (attribute.isEmpty())
                return {e.text()};
            else if (e.hasAttribute(attribute))
                return {e.attribute(attribute)};
            else
                return {};
        }

        const auto& list = e.childNodes();
        const auto& count = list.count();

        auto values = QStringList{};
        for (int i = 0; i < count; ++i)
        {
            const auto& node = list.at(i);
            if (node.isElement() && node.toElement().tagName() == *pathBegin)
            {
                values << valuesByPath(node.toElement(), pathBegin + 1, pathEnd,
                                       attribute, condition);
            }
        }

        return values;
    }

    template <typename It, typename Condition>
    QHash<QString, QString>
        mapValuesByPath(const QDomElement& e, const It& pathBegin,
                        const It& pathEnd, const QString& attribute,
                        Condition condition)
    {
        if (!condition(e))
            return {};

        if (pathBegin == pathEnd)
        {
            return mapValues(e, attribute);
        }

        const auto& list = e.childNodes();
        const auto& count = list.count();

        auto values = QHash<QString, QString>{};
        for (int i = 0; i < count; ++i)
        {
            const auto& node = list.at(i);
            if (node.isElement() && node.toElement().tagName() == *pathBegin)
            {
                values.unite(mapValuesByPath(node.toElement(), pathBegin + 1,
                                             pathEnd, attribute, condition));
            }
        }

        return values;
    }

    QHash<QString, QString> mapValues(const QDomElement& e,
                                      const QString& attribute)
    {
        const auto& list = e.childNodes();
        const auto& count = list.count();

        auto values = QHash<QString, QString>{};
        for (int i = 0; i < count; ++i)
        {
            const auto& node = list.at(i);
            if (node.isElement())
            {
                const auto& element = node.toElement();
                if (attribute.isEmpty())
                {
                    values.insert(element.tagName(), element.text());
                }
                else if (element.hasAttribute(attribute))
                {
                    values.insert(element.tagName(),
                                  element.attribute(attribute));
                }
            }
        }

        return values;
    }

private:
    QIODevice& _device;
};
