/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/kernel/SensitiveDetectorBase.h>
#include <simulation/kernel/RunManager.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;


SensitiveDetectorBase::SensitiveDetectorBase(G4String name) : G4VSensitiveDetector(name)
{

}


SensitiveDetectorBase::~SensitiveDetectorBase()
{

}


bool SensitiveDetectorBase::addRelation(TObject* hit, G4Step* step)
{
  return RunManager::Instance().addRelation(hit, step);
}
