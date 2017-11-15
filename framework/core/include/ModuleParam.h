/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

/*
This header file has been split into declarations '.dcl.h' and inline implementations '.icc.h'
The split has been done to be able to hide the underlying implementation in terms of boost::python
from the user's side.

This move accomblished a substantial reduction in compile time
in parts of the tracking package that heavily uses the ModuleParamList
to coordinate parameters between parts of the algorithms by avoiding
to include the heavy implementation of boost::python almost everywhere.
*/

#include <framework/core/ModuleParam.icc.h>
#include <framework/core/ModuleParam.dcl.h>
