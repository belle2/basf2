/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
#include <mva/methods/Trivial.h>

#endif
