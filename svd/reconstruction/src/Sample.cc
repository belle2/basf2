/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Peter Kvasnicka                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/reconstruction/Sample.h>

using namespace Belle2;
using namespace Belle2::SVD;

// Static data members of the Sample class.

double Sample::m_baseTime     = 0.0;
double Sample::m_samplingTime = 30.0;

void Sample::setBaseTime(double time) { m_baseTime = time; }
void Sample::setSamplingTime(double time) { m_samplingTime = time; }




