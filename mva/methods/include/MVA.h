/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#ifndef INCLUDE_GUARD_BELLE2_METHODS_MVA_HEADER
#define INCLUDE_GUARD_BELLE2_METHODS_MVA_HEADER

/**
 * ROOT requires this header to build the dictionaries.
 * We include the headers of all classes in linkdef.h
 */

#include <mva/methods/FastBDT.h>
#include <mva/methods/TMVA.h>
#include <mva/methods/Python.h>
#include <mva/methods/FANN.h>
#include <mva/methods/PDF.h>
#include <mva/methods/Combination.h>
#include <mva/methods/Reweighter.h>
#include <mva/methods/RegressionFastBDT.h>
#include <mva/methods/Trivial.h>

#endif
