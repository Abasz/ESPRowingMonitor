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

    template <class T, typename... Args>
    static void warningln(T msg, Args... args)
    {
        printf(msg, args...);
        printf("\n");
    };

    template <class T, typename... Args>
    static void errorln(T msg, Args... args)
    {
        printf(msg, args...);
        printf("\n");
    };

    inline void setLevel(int level) {}
};

extern Logging Log;