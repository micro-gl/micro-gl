//
// Created by Tomer Shalev on 2019-06-12.
//

#include "Logger.h"

Logger * Logger::instance() {
    return _instance== nullptr ? _instance=new Logger() : _instance;
}

Logger::Logger() {

}

void Logger::log(const char *formatStr, ...)
{

    va_list params;
    char buf[BUFSIZ];

    va_start ( params, formatStr );
    vsprintf ( buf, formatStr, params );

    std::cout << buf << std::endl;

    va_end ( params );
}