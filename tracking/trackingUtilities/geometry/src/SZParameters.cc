/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackingUtilities/geometry/SZParameters.h>

#include <tracking/trackingUtilities/geometry/UncertainParameters.icc.h>

using namespace Belle2;
using namespace TrackingUtilities;

template struct TrackingUtilities::UncertainParametersUtil<SZUtil, ESZParameter>;
