#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <ctime>

#include "Logger.hpp"

bool log::silentCreation = false;
std::string log::filename("log.txt");
std::ofstream log::fileout(filename);

void log::toFile(const std::string& text)
{
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    fileout << std::put_time(&tm, "{%d-%m-%Y %H:%M:%S} ") << text << std::endl;
}

void log::toConsole(const std::string& text)
{
    std::cout << text << std::endl;
}

void log::openFile(const std::string& output_file)
{
    if(!silentCreation)
    { toFile("Log opened"); }

    fileout.close();
    fileout.open(output_file);
}

void log::closeFile()
{
    if(!silentCreation)
    { toFile("Log closed"); }

    fileout.close();
}

void log::setSilentCreation(bool silent)
{
    silentCreation = silent;
}
