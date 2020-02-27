#pragma once

#include "model/TokenizedString.hpp"

#include <QString>

struct ProjectDescription
{
    QString vcxprojPath;
    //QString destPath;
    TokenizedString defines;
    TokenizedString includePaths;
    TokenizedString headerPaths;
    TokenizedString sourcePaths;
    TokenizedString flags;

    QString msbuildPath;
    QString slnPath;
    QString postBuildCommand;
    QString executablePath;
};
