/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/modules/trackFinderVXDTests/CollectorTestModule.h>

#include <iostream>

using namespace std;
using namespace Belle2;


REG_MODULE(CollectorTest)

CollectorTestModule::CollectorTestModule() : Module()
{
  setDescription("CollectorTest.");
}

void CollectorTestModule::initialize()
{

}

void CollectorTestModule::event()
{

}

void CollectorTestModule::terminate()
{
  cerr << "\n";
}

