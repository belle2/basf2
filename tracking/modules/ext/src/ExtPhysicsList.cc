/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <globals.hh>

#include <tracking/modules/ext/ExtPhysicsList.h>
#include <simulation/kernel/ExtPhysicsConstructor.h>

using namespace std;
using namespace Belle2;

ExtPhysicsList::ExtPhysicsList() : G4VModularPhysicsList()
{
  defaultCutValue = 1.0E+9 * cm; // set big step so that AlongStep computes all the energy
  RegisterPhysics(new Simulation::ExtPhysicsConstructor);
}

ExtPhysicsList::~ExtPhysicsList()
{
}

void ExtPhysicsList::SetCuts()
{
  SetCutsWithDefault();
}

