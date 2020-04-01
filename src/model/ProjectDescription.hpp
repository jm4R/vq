#pragma once

#include "model/TokenizedString.hpp"

#include <QHash>
#include <QString>
#include <QVector>

struct ConfigurationDescription
{
    QString name;

    TokenizedString defines;
    TokenizedString includePaths;
    TokenizedString headerPaths;
    TokenizedString sourcePaths;
    TokenizedString propsFiles;
    QHash<QString, QString> properties;
};

struct ProjectDescription
{
    QString vcxprojPath;
    //QString destPath;
    TokenizedString flags;

    QString msbuildPath;
    QString slnPath;
    QString postBuildCommand;
    QString executablePath;

    //internal:
    QString compilerVersionStr;
    int compilerVersionMajor{};
    int compilerVersionMinor{};

    QVector<ConfigurationDescription> configurations;
};
