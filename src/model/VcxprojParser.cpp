#include "VcxprojParser.hpp"

#include <QDomDocument>
#include <QIODevice>

#include <QDebug>

namespace {

template <typename It>
TokenizedString valuesByPath(const QDomElement& e, const It& pathBegin,
                             const It& pathEnd, const QString& attribute = {})
{
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

    auto values = TokenizedString{};
    for (int i = 0; i < count; ++i)
    {
        const auto& node = list.at(i);
        if (node.isElement() && node.toElement().tagName() == *pathBegin)
        {
            values << valuesByPath(node.toElement(), pathBegin + 1, pathEnd,
                                   attribute);
            qDebug() << "   " << *pathBegin << " "
                     << node.toElement().tagName();
        }
    }

    return values;
}

TokenizedString extract(QIODevice& device, const QStringList path,
                        const QString& attribute = {})
{
    device.reset();

    const auto& doc = [&] {
        auto r = QDomDocument{};
        r.setContent(&device, false);
        return r;
    }();

    if (path.empty() || doc.documentElement().tagName() != path[0])
        return {};

    auto values = valuesByPath(doc.documentElement(), path.cbegin() + 1,
                               path.cend(), attribute);

    values.normalize();
    return values;
}

void postProcess(ProjectDescription& d)
{
    if (!d.compilerVersionStr.isEmpty())
    {
        auto list = d.compilerVersionStr.split('.');
        d.compilerVersionMajor = list[0].toInt();
        if (list.size() > 1)
            d.compilerVersionMinor = list[1].toInt();
    }

    d.flags.append({"-GX", "-fms-compatibility"});

    if (d.compilerVersionMajor > 0)
    {
        d.flags.append("-fms-compatibility-version=" +
                       QString::number(d.compilerVersionMajor));
    }
}

} // namespace

ProjectDescription parseVcxproj(QIODevice& device)
{
    auto result = ProjectDescription{};

    result.sourcePaths =
        extract(device, {"Project", "ItemGroup", "ClCompile"}, "Include");

    result.headerPaths =
        extract(device, {"Project", "ItemGroup", "ClInclude"}, "Include");

    result.includePaths =
        extract(device, {"Project", "ItemDefinitionGroup", "ClCompile",
                         "AdditionalIncludeDirectories"});

    result.defines = extract(device, {"Project", "ItemDefinitionGroup",
                                      "ClCompile", "PreprocessorDefinitions"});

    result.compilerVersionStr = extract(device, {"Project"}, "ToolsVersion");

    result.propsFiles = extract(device, {"Project", "ImportGroup", "Import"}, "Project");
    result.configurations = extract(device, {"Project", "ItemGroup", "ProjectConfiguration"}, "Include");

    postProcess(result);

    return result;
}
