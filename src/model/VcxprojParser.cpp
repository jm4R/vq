#include "VcxprojParser.hpp"

#include "model/ProjectDescription.hpp"
#include "model/XmlValuesExtractor.hpp"

#include <QDir>
#include <QFile>

#include <QDebug>

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

void merge(ConfigurationDescription& to, ConfigurationDescription&& from)
{
    to.defines << std::move(from.defines);
    to.includePaths << std::move(from.includePaths);
    to.headerPaths << std::move(from.headerPaths);
    to.sourcePaths << std::move(from.sourcePaths);
    to.propsFiles << std::move(from.propsFiles);
    to.properties.unite(std::move(from.properties));
}

void applyProperties(TokenizedString& value,
                     const QHash<QString, QString>& props)
{
    for (const auto& p : props.keys())
    {
        const auto from = "$(" + p + ")";
        const auto& to = props[p];
        value = value.replaceInStrings(from, to);
    }
}

ConfigurationDescription readConfiguration(XmlValuesExtractor& extractor,
                                           const QString& name,
                                           const QString rootPath,
                                           bool recursive = true)
{
    // e.g.
    // Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'"
    auto pred = [&](const QDomElement& element) {
        if (!element.hasAttribute("Condition"))
            return true;
        auto condition = element.attribute("Condition").remove(' ');
        if (!condition.startsWith(
                R"('$(Configuration)|$(Platform)'==')"))
            return true;
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
        extractor.extractMapIf(pred, {"Project", "PropertyGroup"});

    auto pred2 = [&](const QDomElement& element) {
        if (!element.hasAttribute("Include"))
            return true;
        return element.attribute("Include") == name;
    };

    result.properties.unite(extractor.extractMapIf(
        pred2, {"Project", "ItemGroup", "ProjectConfiguration"}));

    applyProperties(result.propsFiles, result.properties);

    if (recursive)
    {
        for (const auto& propPath : result.propsFiles)
        {
            auto path = propPath;
            path.replace('\\', '/');
            if (QDir::isRelativePath(path))
                path = rootPath + "/" + path;
            if (!QFile::exists(path))
            {
                qDebug() << "props file not found: " << propPath;
                continue;
            }
            auto file = QFile{path};
            file.open(QIODevice::ReadOnly);
            auto propsExtractor = XmlValuesExtractor{file};
            auto propsConfig =
                readConfiguration(propsExtractor, name, rootPath, false);
            merge(result, std::move(propsConfig));
        }
    }

    applyProperties(result.defines, result.properties);
    applyProperties(result.includePaths, result.properties);
    applyProperties(result.headerPaths, result.properties);
    applyProperties(result.sourcePaths, result.properties);

    return result;
}

} // namespace

ProjectDescription parseVcxproj(QIODevice& device, const QString& rootPath)
{
    device.open(QIODevice::ReadOnly);
    auto result = ProjectDescription{};
    auto extractor = XmlValuesExtractor{device};

    auto configurationNames = extractor.extract(
        {"Project", "ItemGroup", "ProjectConfiguration"}, "Include");

    for (const auto& cfgName : configurationNames)
    {
        result.configurations.append(
            readConfiguration(extractor, cfgName, rootPath));
    }

    result.compilerVersionStr =
        extractor.extractOne({"Project"}, "ToolsVersion");
    result.executablePath =
        extractor.extractOne({"Project", "ItemDefinitionGroup", "Link",
                              "OutputFile"}); // TODO: per configuration

    postProcess(result);

    return result;
}
