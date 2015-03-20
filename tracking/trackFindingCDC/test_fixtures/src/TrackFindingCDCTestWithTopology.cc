/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/TrackFindingCDCTestWithTopology.h"

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <framework/gearbox/Gearbox.h>
#include <framework/logging/Logger.h>

#include <gtest/gtest.h>

#include <string>
#include <vector>

using namespace std;

using namespace Belle2;
using namespace TrackFindingCDC;


void TrackFindingCDCTestWithTopology::SetUpTestCase()
{
  //Setup the gearbox
  TestHelpers::TestWithGearbox::SetUpTestCase();

  //Also preload the CDCGeometry
  const CDCWireTopology& wireTopology __attribute__((unused)) = CDCWireTopology::getInstance();
}

void TrackFindingCDCTestWithTopology::TearDownTestCase()
{
  //Close the gearbox
  TestHelpers::TestWithGearbox::TearDownTestCase();
}



