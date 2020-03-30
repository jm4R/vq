#pragma once

class QIODevice;
class ProjectDescription;

ProjectDescription parseVcxproj(QIODevice& device);
