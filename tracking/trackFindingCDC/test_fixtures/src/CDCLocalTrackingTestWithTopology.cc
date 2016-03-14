/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* This file provides the implementation of the common test fixture to all
   test of the CDCLocalTracking code. */

#include "../include/CDCLocalTrackingTestWithTopology.h"

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <framework/gearbox/Gearbox.h>
#include <framework/logging/Logger.h>

#include <gtest/gtest.h>

#include <string>
#include <vector>

using namespace std;

using namespace Belle2;
using namespace CDCLocalTracking;


void CDCLocalTrackingTestWithTopology::SetUpTestCase()
{

  //Setup the gearbox
  Gearbox& gearbox = Gearbox::getInstance();

  vector<string> backends;
  backends.push_back("file:");
  gearbox.setBackends(backends);

  B2INFO("Start open gearbox.");
  gearbox.open("geometry/Belle2.xml");
  B2INFO("Finished open gearbox.");

  //Also preload the CDCGeometry
  const CDCWireTopology& wireTopology __attribute__((unused)) = CDCWireTopology::getInstance();

}

void CDCLocalTrackingTestWithTopology::TearDownTestCase()
{

  Gearbox& gearbox = Gearbox::getInstance();
  gearbox.close();

}



