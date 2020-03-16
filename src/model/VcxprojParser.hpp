#pragma once

#include "model/ProjectDescription.hpp"

#include <QObject>

class QDomDocument;

class VcxprojParser : public QObject
{
    Q_OBJECT
public:
    explicit VcxprojParser(QObject *parent = nullptr);

    static ProjectDescription parse();
};
