#include "model/OutputGenerator.hpp"

#include "model/ProjectDescription.hpp"

#include <QDir>

OutputGenerator::OutputGenerator(QObject* parent) : QObject{parent} {}

OutputGenerator::~OutputGenerator() {}

void OutputGenerator::generate(const ProjectDescription& d)
try
{
    const auto projectName = QFileInfo{d.vcxprojPath}.baseName();
    const auto creatorDir = [&] {
        auto r = QDir{d.vcxprojPath};
        r.cdUp();
        return r;
    }();

    auto makeFile = [&](const char* suffix) -> QFile {
        const auto name = creatorDir.filePath(projectName + suffix);
        if (QFile::exists(name))
        {
            throw std::runtime_error{
                (name + " : file already exist").toStdString()};
        }
        return {name};
    };

    auto&& creatorFile = makeFile(".creator");
    auto&& cxxflagsFile = makeFile(".cxxflags");
    auto&& definesFile = makeFile(".config");
    auto&& srcListFile = makeFile(".files");
    auto&& includesFile = makeFile(".includes");

    auto writeToFile = [](QFile& file, const QString& value) {
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        file.write(value.toStdString().c_str());
        file.close();
    };

    writeToFile(creatorFile, "[General]\n");
    writeToFile(cxxflagsFile, d.flags.join('\n'));
    writeToFile(definesFile, d.defines.buildString("#define ", "\n"));
    writeToFile(srcListFile,
                d.sourcePaths.join('\n') + "\n" + d.headerPaths.join('\n'));
    writeToFile(includesFile, d.includePaths.join('\n'));

    emit finished();
}
catch (std::exception& ex)
{
    emit failure(ex.what());
}
catch (...)
{
    emit failure("Unknown error (0xda7900)");
}
