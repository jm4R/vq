#pragma once

#include <QObject>

struct ProjectDescription;

class QDir;

class OutputGenerator : public QObject
{
    Q_OBJECT
public:
    explicit OutputGenerator(QObject* parent = nullptr);
    ~OutputGenerator();

signals:
    void failure(const QString& what);
    void finished();

public slots:
    void generate(const ProjectDescription& p);
};
