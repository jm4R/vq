#include "VcxprojParser.hpp"

#include <QXmlQuery>
#include <QXmlResultItems>

#include <QDebug>

VcxprojParser::VcxprojParser(QObject* parent) : QObject{parent} {}

ProjectDescription VcxprojParser::parse()
{
    constexpr static auto XPATH_SRC =
        "/Project/ItemGroup/ClCompile/@Include/data(.)";
    constexpr static auto XPATH_INC =
        "/Project/ItemDefinitionGroup/AdditionalIncludeDirectories/data(.)";
    constexpr static auto XPATH_DEF =
        "/Project/ItemDefinitionGroup/PreprocessorDefinitions/data(.)";

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

    ProjectDescription result;
    result.sourcePaths = extract(testXml, XPATH_SRC);
    result.includePaths = extract(testXml, XPATH_INC);
    result.defines = extract(testXml, XPATH_DEF);

    return result;
}

TokenizedString VcxprojParser::extract(const QString& xml, const QString& xpath)
{
    QXmlQuery query;
    query.setFocus(xml);
    query.setQuery(xpath);

    if (query.isValid())
    {
        throw __LINE__;
    }

    QString str;
    QXmlResultItems result;
    for (auto item = result.next(); !item.isNull(); item = result.next())
    {
        str += item.toAtomicValue().toString();
    }
    if (result.hasError())
    {
        throw __LINE__;
    }

    return {str};
}
