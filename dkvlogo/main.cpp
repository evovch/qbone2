#include <QCoreApplication>

#include "../qbone/display.h"

int main(int argc, char *argv[])
{
    Display *led = new Display();
    led->putLogo();
}
