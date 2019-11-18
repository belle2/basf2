#pragma once

#ifndef B2FATAL

#ifdef NOBASF2
#include <iostream>
#define B2FATAL(a) {std::cerr << a << std::endl; exit(1);}
#else
#include <framework/logging/Logger.h>
#endif

#endif

// To compile DAQ program on COPPER(SL5.7)
#ifndef OVERRIDE_CPP17
#ifndef NOBASF2
#define OVERRIDE_CPP17 override
#else
#define OVERRIDE_CPP17
#endif
#endif
