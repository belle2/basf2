/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua LI                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// own include
#include <hlt/L1Emulator/modules/L1Emulation/L1EmulationModule.h>
// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/dataobjects/EventMetaData.h>

//framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>

// dataobjects
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <analysis/dataobjects/ParticleList.h>

//utilities
#include <analysis/VariableManager/Variables.h>
#include <analysis/VariableManager/PhysicsTriggerVariables.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <framework/utilities/FileSystem.h>

#include <boost/foreach.hpp>
#include <fstream>
#include <iostream>
#include <iomanip>

#include <TRandom.h>

using namespace Belle2;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(L1Emulation)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

L1EmulationModule::L1EmulationModule() : Module()
{
  // Set module properties
  setDescription("L1 emulator for the study of L1 trigger");
  setPropertyFlags(c_ParallelProcessingCertified);
  std::string emptyCut;
  std::vector<int> emptyScale;
  addParam("TRG", m_userCut, "trigger", emptyCut);
  addParam("TRG_Scale", m_scalefactor, "the scale factor of trigger", emptyScale);
}

L1EmulationModule::~L1EmulationModule()
{
}

void L1EmulationModule::initialize()
{
  B2INFO("L1EmulationModule processing");
  m_l1EmulationInformation.registerInDataStore();
  m_cut = Variable::Cut::compile(m_userCut);
  m_ntrg = 0;
  trgname.clear();
  nEvent = 0;
}


void L1EmulationModule::event()
{
  setReturnValue(0);
  if (eventSelect())
    setReturnValue(1);
}

void L1EmulationModule::terminate()
{
}


bool L1EmulationModule::eventSelect()
{

//get weight of event
  nEvent++;
  double Weight = 1.0;
  StoreObjPtr<EventMetaData> eventmetadata;
  if (eventmetadata)
    Weight = eventmetadata->getGeneratedWeight();

  const Particle* part = NULL;
  StoreArray<L1EmulationInformation> LEInfos;
  if (!LEInfos.getEntries()) {
    LEInfos.appendNew(L1EmulationInformation());
    B2DEBUG(200, "No entry in L1EmulationInformation");
  }
  //L1EmulationInformation* LEInfo = LEInfos.appendNew(L1EmulationInformation());
  L1EmulationInformation& LEInfo = *LEInfos[0];
  LEInfo.setnTrg(1);
  m_ntrg = LEInfo.getnTrg();
  if (nEvent == 1)LEInfo.setTrgName(m_ntrg - 1, getName());

  double summary = 0.0;

  if ((m_userCut.size() && m_cut->check(part)) || (!m_userCut.size())) {
    if (makeScalefx(m_scalefactor)) {
      summary = Weight;
      if (m_ntrg == 1)LEInfo.setECLBhabha(1);
      else if (m_ntrg == 2)
        LEInfo.setBhabhaVeto(1);
      else if (m_ntrg == 3)
        LEInfo.setSBhabhaVeto(1);
      else if (m_ntrg == 4)
        LEInfo.setggVeto(1);
    }
  }

  LEInfo.setSummary(m_ntrg, summary);
  bool Val = false;
//The global trigger result
  if (summary > 0) {
    if (m_ntrg > 4)LEInfo.setSummary(0, summary);
    Val = true;
  }

  return Val;
}

bool L1EmulationModule::makeScale(int f)
{
  bool Val = false;
  double ran = gRandom->Uniform(f);
  if (ceil(ran) == f) Val = true;

  return Val;
}

bool L1EmulationModule::makeScalefx(std::vector<int> f)
{
  bool Val = true;

  int fsize = f.size();
  if (fsize < 1)
    Val = makeScale(1);
  else if (fsize == 1)
    Val = makeScale(f[0]);

  if (fsize <= 1)
    return Val;

  Particle* p = NULL;
  double theta = -1;
  double interval = 0.;
  theta = Variable::MinusThetaBhabhaLE(p); interval = (2.65 - 0.2) / (double)fsize;
//  theta = Variable::ThetaC1LE(p); interval = (2.7 - 0.2) / (double)fsize;

  for (int i = 0; i < fsize; i++) {
    if (theta >= (interval * i + 0.2) && theta < (interval * (i + 1) + 0.2)) {
      Val = makeScale(f[i]);
      break;
    }
  }

  return Val;
}


