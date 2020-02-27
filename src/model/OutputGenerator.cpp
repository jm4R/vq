#include "model/OutputGenerator.hpp"

OutputGenerator::OutputGenerator(QObject* parent) : QObject{parent} {}

OutputGenerator::~OutputGenerator() {}

void OutputGenerator::generate()
try
{
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