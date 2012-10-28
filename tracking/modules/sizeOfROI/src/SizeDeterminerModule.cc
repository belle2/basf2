/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 Belle II Collaboration                               *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 **************************************************************************/

#include <tracking/modules/sizeOfROI/SizeDeterminerModule.h>
#include <framework/logging/Logger.h>
#include <cmath>

using namespace std;
using namespace Belle2;

REG_MODULE(SizeDeterminer)

SizeDeterminerModule::SizeDeterminerModule() : Module()
{
  setDescription("Provide plots to determine the necessary size for ROIs");
}

void SizeDeterminerModule::initialize()
{
}

void SizeDeterminerModule::beginRun()
{
}

void SizeDeterminerModule::event()
{
}
