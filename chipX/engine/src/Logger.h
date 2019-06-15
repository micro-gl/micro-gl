//
// Created by Tomer Shalev on 2019-06-12.
//

#pragma once

#include <iostream>
#include <stdio.h>
#include <fstream>
#include <stdarg.h>

class Logger {
public:
    Logger * instance();
    static void log(const char *formatStr, ...);

private:
    Logger * _instance = nullptr;
    Logger();

};

