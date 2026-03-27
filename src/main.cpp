#include "Core/Application.hpp"

#ifdef _WIN32
#include <windows.h>
#pragma comment(linker, "/SUBSYSTEM:console")
#endif

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    Application app;
    app.run();

    return 0;
}