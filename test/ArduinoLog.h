#pragma once

#include <stdio.h>

class Logging
{
public:
    template <class T, typename... Args>
    static void infoln(T msg, Args... args)
    {
        printf(msg, args...);
        printf("\n");
    };

    template <class T, typename... Args>
    static void verboseln(T msg, Args... args)
    {
        printf(msg, args...);
        printf("\n");
    };

    template <class T, typename... Args>
    static void traceln(T msg, Args... args)
    {
        printf(msg, args...);
        printf("\n");
    };
};

extern Logging Log;