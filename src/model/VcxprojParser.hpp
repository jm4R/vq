#pragma once

class ProjectDescription;
class QIODevice;
class QString;

ProjectDescription parseVcxproj(QIODevice& device, const QString& rootPath);
