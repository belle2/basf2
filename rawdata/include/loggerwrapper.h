#pragma once

#ifndef B2FATAL

#ifdef NOBASF2
#include <iostream>
#define B2FATAL(a) {std::cerr << a << std::endl; exit(1);}
#else
#include <framework/logging/Logger.h>
#endif

#endif
