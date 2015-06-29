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
#include <skim/L1Emulator/modules/L1Emulation/L1EmulationModule.h>
#include <skim/L1Emulator/dataobjects/L1EmulationInformation.h>
// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
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
#include <analysis/VariableManager/L1EmulatorVariables.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <framework/utilities/FileSystem.h>

#include <boost/foreach.hpp>
#include <fstream>
#include <iostream>
#include <iomanip>

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

  addParam("UserCustomOpen", m_userCustomOpen, "the switch of customing the L1 Trigger by user", 0);
  std::string emptyCut;
  for (int i = 1; i <= 50; i++) {
    char name[10];
    sprintf(name, "TRG%d", i);
    addParam(name, m_userCut[i - 1], name, emptyCut);
  }

}

L1EmulationModule::~L1EmulationModule()
{
}

void L1EmulationModule::initialize()
{
  B2INFO("L1EmulationModule processing");
  StoreArray<L1EmulationInformation>::registerPersistent();
  m_cut = Variable::Cut::Compile(m_userCut[0]);

  Total_Event = 0;
  for (int i = 0; i < 50; i++) {
    TRG_Event[i] = 0;
    TRG_Event_TRGOnly[i] = 0;
    for (int j = 0; j < 50; j++)
      TRG_Event_Matrix[i][j] = 0;
  }
}


void L1EmulationModule::event()
{
  eventSelect();
  setReturnValue(1);
}

void L1EmulationModule::terminate()
{
//std::cout<<"TRG "<<"\t"<<"Ratio"<<std::endl;
  int W = 7;

  std::cout << std::endl;
  std::cout << "========L1 Emulator TRG Matrix========" << std::endl;
  for (int i = 0; i < 50; i++)
    if (m_userCut[i].size())std::cout << std::setw(W) << "-------";
  std::cout << "-------" << std::endl;

  std::cout << std::left << std::setw(W) << "TRG";
  for (int i = 0; i < 50; i++)
    if (m_userCut[i].size())std::cout << std::setw(W) << i + 1;
  std::cout << std::endl;

  for (int i = 0; i < 50; i++) {
    if (m_userCut[i].size()) {
//std::cout<<i+1<<"\t"<<TRG_Event[i]/(float)Total_Event<<std::endl;
      std::cout << std::left << std::setw(W) << i + 1;
      for (int j = 0; j < 50; j++) {
        if (j < i)std::cout << std::setw(W) << " ";
        if (j >= i && m_userCut[j].size())
          std::cout << std::setw(W) << std::setprecision(2) << TRG_Event_Matrix[i][j] / (float)Total_Event;
      }
      std::cout << std::endl;

    }


  }

  for (int i = 0; i < 50; i++)
    if (m_userCut[i].size())std::cout << std::setw(W) << "-------";
  std::cout << "-------" << std::endl;
  std::cout << std::left << std::setw(W) << "TRG";
  for (int i = 0; i < 50; i++)
    if (m_userCut[i].size())std::cout << std::setw(W) << i + 1;
  std::cout << std::endl;

  std::cout << std::left << std::setw(W) << "R";
  for (int i = 0; i < 50; i++)
    if (m_userCut[i].size())std::cout << std::setw(W) << TRG_Event_TRGOnly[i] / (float)Total_Event;
  std::cout << std::endl;

  for (int i = 0; i < 50; i++)
    if (m_userCut[i].size())std::cout << std::setw(W) << "-------";
  std::cout << "-------" << std::endl;
  std::cout << std::endl;

}


void L1EmulationModule::eventSelect()
{
  Total_Event++;
  for (int i = 0; i < 50; i++)m_summary[i] = 0;

  const Particle* part = NULL;
  StoreArray<L1EmulationInformation> LEInfos;
  L1EmulationInformation* LEInfo = LEInfos.appendNew(L1EmulationInformation());

  int count = 0;
  int record = -1;
  for (int i = 0; i < 50; i++) {
    // TODO: This is extremely inefficient
    // The Cut objects should be created once in initialize!
    // Because parsing the cut string and generating the cut objects is (extremely) slow!
    m_cut = Variable::Cut::Compile(m_userCut[i]);
    if (m_userCut[i].size() && m_cut->check(part)) {
      m_summary[i] = 1;
      if (i == 0)LEInfo->setECLBhabha(1);
      else if (i == 1) LEInfo->setBhabhaVeto(1);
      else if (i == 2) LEInfo->setggVeto(1);
      TRG_Event[i]++;
      count++;
      record = i;
    }
    LEInfo->setSummary(m_summary[i]);
  }

  for (int i = 0; i < 50; i++) {
    if (m_summary[i] == 1) {
      for (int j = i; j < 50; j++) {
        if (m_summary[j] == 1)
          TRG_Event_Matrix[i][j]++;
      }
    }
  }
  if (count == 1)
    TRG_Event_TRGOnly[record]++;


}

