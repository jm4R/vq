#include "VcxprojParser.hpp"

#include <QDomDocument>

#include <QDebug>

namespace {

template <typename It>
QStringList valuesByPath(const QDomElement& e, const It& pathBegin,
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

    auto values = QStringList{};
    for (int i = 0; i < count; ++i)
    {
        const auto& node = list.at(i);
        qDebug() << *pathBegin << " " << node.toElement().tagName();
        if (node.isElement() && node.toElement().tagName() == *pathBegin)
            values << valuesByPath(node.toElement(), pathBegin + 1, pathEnd,
                                   attribute);
    }

    return values;
}

TokenizedString extract(const QString& xml, const QStringList path,
                        const QString& attribute = {})
{
    const auto& doc = [&] {
        auto r = QDomDocument{};
        r.setContent(xml, false);
        return r;
    }();

    if (path.empty() || doc.documentElement().tagName() != path[0])
        return {};

    auto values = valuesByPath(doc.documentElement(), path.cbegin() + 1,
                               path.cend(), attribute);

    return {std::move(values)};
}

} // namespace

VcxprojParser::VcxprojParser(QObject* parent) : QObject{parent} {}

ProjectDescription VcxprojParser::parse()
{
    auto testXml = R"(
                   <Project>
                    <ItemGroup>
                     <ClCompile Include="a.cpp"/>
                     <ClCompile Include="a.hpp"/>
                     <ClCompile Include="b.hpp"/>
                    </ItemGroup>
                    <ItemDefinitionGroup>
                     <AdditionalIncludeDirectories>some_incl_dir</AdditionalIncludeDirectories>
                     <AdditionalIncludeDirectories>another</AdditionalIncludeDirectories>
                     <PreprocessorDefinitions>NDEBUG</PreprocessorDefinitions>
                    </ItemDefinitionGroup>
                   </Project>)";

    auto result = ProjectDescription{};
    result.sourcePaths =
        extract(testXml, {"Project", "ItemGroup", "ClCompile"}, "Include");
    result.includePaths = extract(testXml, {"Project", "ItemDefinitionGroup",
                                            "AdditionalIncludeDirectories"});
    result.defines = extract(
        testXml, {"Project", "ItemDefinitionGroup", "PreprocessorDefinitions"});

    return result;
}
