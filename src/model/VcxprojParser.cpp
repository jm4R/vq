#include "VcxprojParser.hpp"

#include "model/ProjectDescription.hpp"
#include "model/XmlValuesExtractor.hpp"

namespace {

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

ConfigurationDescription readConfiguration(XmlValuesExtractor& extractor,
                                           const QString& name)
{
    // e.g.
    // Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'"
    auto pred = [&](const QDomElement& element) {
        if (!element.hasAttribute("Condition"))
            return true;
        auto condition = element.attribute("Condition");
        if (!condition.startsWith(R"('$(Configuration)|$(Platform)'==')"))
            return true;
        if (name.isEmpty())
            return false;
        return condition.endsWith("\'" + name + "\'");
    };

    auto result = ConfigurationDescription{};
    result.name = name;
    result.sourcePaths = extractor.extractIf(
        pred, {"Project", "ItemGroup", "ClCompile"}, "Include");

    result.headerPaths = extractor.extractIf(
        pred, {"Project", "ItemGroup", "ClInclude"}, "Include");

    result.includePaths = extractor.extractIf(
        pred, {"Project", "ItemDefinitionGroup", "ClCompile",
               "AdditionalIncludeDirectories"});

    result.defines =
        extractor.extractIf(pred, {"Project", "ItemDefinitionGroup",
                                   "ClCompile", "PreprocessorDefinitions"});

    result.propsFiles = extractor.extractIf(
        pred, {"Project", "ImportGroup", "Import"}, "Project");

    result.properties =
        extractor.extractMap(pred, {"Project", "PropertyGroup"});

    auto pred2 = [&](const QDomElement& element) {
        if (!element.hasAttribute("Include"))
            return true;
        return element.attribute("Include") == name;
    };

    result.properties.unite(extractor.extractMap(
        pred2, {"Project", "ItemGroup", "ProjectConfiguration"}));

    return result;
}

} // namespace

ProjectDescription parseVcxproj(QIODevice& device)
{
    auto result = ProjectDescription{};
    auto extractor = XmlValuesExtractor{device};

    auto configurationNames = extractor.extract(
        {"Project", "ItemGroup", "ProjectConfiguration"}, "Include");

    result.commonConfiguration = readConfiguration(extractor, "");
    for (const auto& cfgName : configurationNames)
    {
        result.configurations.append(readConfiguration(extractor, cfgName));
    }

    result.compilerVersionStr =
        extractor.extractOne({"Project"}, "ToolsVersion");

    postProcess(result);

    return result;
}
