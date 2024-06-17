#pragma once

#include "Base.h"
#include "Commander.h"
#include "Settings.h"

class Controller
{
public:
    Controller();

    Settings settings;
    Commander commander;
};
