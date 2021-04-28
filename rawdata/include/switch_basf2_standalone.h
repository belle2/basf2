#pragma once

#ifndef B2FATAL
#ifdef NOBASF2
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <climits>
#include <errno.h>
#include <unistd.h>
#define B2FATAL(a) {std::cerr << a << std::endl; exit(1);}
#else
#include <framework/logging/Logger.h>
#endif // NOBASF2
#endif // B2FATAL

// To compile DAQ program on COPPER(SL5.7)
#ifndef OVERRIDE_CPP17
#ifndef NOBASF2
#define OVERRIDE_CPP17 override
#else
#define OVERRIDE_CPP17
#endif // NOBASF2
#endif // OVERRIDE_CPP17

// To compile DAQ program on COPPER(SL5.7)
#ifndef FINAL_CPP17
#ifndef NOBASF2
#define FINAL_CPP17 final
#else
#define FINAL_CPP17
#endif // NOBASF2
#endif // FINAL_CPP17

// Max channels of PCIe40 board (2020.9.9)
#define MAX_PCIE40_CH 48
#define MAX_COPPER_CH 4
