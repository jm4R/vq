#pragma once

#include "model/ProjectDescription.hpp"

class QIODevice;

ProjectDescription parseVcxproj(QIODevice& device);
