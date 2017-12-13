/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua LI                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// own include
#include <hlt/hlt/modules/physicstrigger/FillHLTTagModule.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>

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



using namespace Belle2;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(FillHLTTag)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

FillHLTTagModule::FillHLTTagModule() : Module()
{
  // Set module properties
  setDescription("skim physics processes");
  setPropertyFlags(c_ParallelProcessingCertified);

}

FillHLTTagModule::~FillHLTTagModule()
{
}

void FillHLTTagModule::initialize()
{
  B2INFO("FillHLTTagModule processing");
  m_hltTag.registerInDataStore();
  m_physicsTriggerInformation.registerInDataStore();
}

void FillHLTTagModule::beginRun()
{
}

void FillHLTTagModule::event()
{

  setReturnValue(0);
  if (eventSelect()) setReturnValue(1);
}

void FillHLTTagModule::endRun()
{
}

void FillHLTTagModule::terminate()
{
}


bool  FillHLTTagModule::eventSelect()
{
  B2DEBUG(200, "User custom selection is open");
  StoreArray<PhysicsTriggerInformation> phyTriInfos;
  if (!phyTriInfos.getEntries()) {
    B2DEBUG(200, "No entries in PhysicsTriggerInformation");
  }
  int summary = 0;

  PhysicsTriggerInformation& phyTriInfo = *phyTriInfos[0];
  summary = phyTriInfo.getPTSummary();

  StoreArray<HLTTag> hltTags;
  if (!hltTags.getEntries()) {
    hltTags.appendNew(HLTTag());
    B2DEBUG(200, "No entry in hltTags");
  }

  HLTTag& hltTag = *hltTags[0];
  int subsum[16] = {0};
  for (int i = 0; i < 16; i++)
    subsum[i] = phyTriInfo.getsubTrgResult(i);

  if (subsum[0])
    hltTag.Accept(HLTTag::Hadronic);
  else
    hltTag.Discard(HLTTag::Hadronic);
  hltTag.SetAlgoInfo(HLTTag::Hadronic, subsum[0]);

  if (subsum[3])
    hltTag.Accept(HLTTag::Bhabha);
  else
    hltTag.Discard(HLTTag::Bhabha);
  hltTag.SetAlgoInfo(HLTTag::Bhabha, subsum[3]);


  if (subsum[1])
    hltTag.Accept(HLTTag::Tautau);
  else
    hltTag.Discard(HLTTag::Tautau);
  hltTag.SetAlgoInfo(HLTTag::Tautau, subsum[1]);

  if (subsum[2])
    hltTag.Accept(HLTTag::LowMulti);
  else
    hltTag.Discard(HLTTag::LowMulti);
  hltTag.SetAlgoInfo(HLTTag::LowMulti, subsum[2]);


  if (subsum[4])
    hltTag.Accept(HLTTag::Mumu);
  else
    hltTag.Discard(HLTTag::Mumu);
  hltTag.SetAlgoInfo(HLTTag::Mumu, subsum[4]);

//tempory: 6:GG 7:Other
  if (subsum[6])
    hltTag.Accept(HLTTag::Calib1);
  else
    hltTag.Discard(HLTTag::Calib1);
  hltTag.SetAlgoInfo(HLTTag::Calib1, subsum[6]);

  if (subsum[7])
    hltTag.Accept(HLTTag::Calib2);
  else
    hltTag.Discard(HLTTag::Calib2);
  hltTag.SetAlgoInfo(HLTTag::Calib2, subsum[7]);

  bool Fval = false;
  if (summary)
    Fval = true;
  else
    Fval = false;

  return Fval;

}




