
/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua LI                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/VariableManager/L1EmulatorVariables.h>

#include <analysis/VariableManager/Manager.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

//#include <hlt/L1Emulator/modules/L1Emulation/L1EmulationModule.h>
#include <hlt/L1Emulator/dataobjects/L1EmulationInformation.h>
// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <TLorentzVector.h>
#include <TVectorF.h>
#include <TVector3.h>

#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace Variable {


    double eclBhabha(const Particle*)
    {
      double result = 0;
      StoreArray<L1EmulationInformation> LEins;
      if (LEins.getEntries())result = LEins[0]->getECLBhabha();
      return result;
    }

    double BhabhaVeto(const Particle*)
    {
      double result = 0;
      StoreArray<L1EmulationInformation> LEinsb;
      if (LEinsb.getEntries())result = LEinsb[0]->getBhabhaVeto();
      return result;
    }

    double SBhabhaVeto(const Particle*)
    {
      double result = 0;
      StoreArray<L1EmulationInformation> LEinsb;
      if (LEinsb.getEntries())result = LEinsb[0]->getSBhabhaVeto();
      return result;
    }

    double ggVeto(const Particle*)
    {
      double result = 0;
      StoreArray<L1EmulationInformation> LEinsb;
      if (LEinsb.getEntries())result = LEinsb[0]->getggVeto();
      return result;
    }

    //Variables for LE
    VARIABLE_GROUP("L1Emulator");
    REGISTER_VARIABLE("eclBhabhaVetoHLT", eclBhabha, "[Eventbased] the eclBhabha veto");
    REGISTER_VARIABLE("BhabhaVetoHLT", BhabhaVeto, "[Eventbased] the Bhabha veto");
    REGISTER_VARIABLE("SBhabhaVetoHLT", SBhabhaVeto, "[Eventbased] the Bhabha veto with single track");
    REGISTER_VARIABLE("ggVetoHLT", ggVeto, "[Eventbased] the gg Veto");

  }
}

