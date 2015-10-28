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
#include <skim/L1Emulator/modules/L1Emulation/L1EmulationPrintModule.h>
#include <skim/L1Emulator/dataobjects/L1EmulationInformation.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <boost/foreach.hpp>
#include <fstream>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace Belle2;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(L1EmulationPrint)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

L1EmulationPrintModule::L1EmulationPrintModule() : Module()
{
  // Set module properties
  setDescription("L1 emulator for printing the result of L1 trigger");
  setPropertyFlags(c_ParallelProcessingCertified);
}

L1EmulationPrintModule::~L1EmulationPrintModule()
{
}

void L1EmulationPrintModule::initialize()
{
  B2INFO("L1EmulationPrintModule processing");
  StoreArray<L1EmulationInformation>::registerPersistent();
  m_weightcount = 0;
  m_weightcounttrigger = 0;
  m_ntrg = 0;
}


void L1EmulationPrintModule::event()
{
  StoreArray<L1EmulationInformation> LEInfos;
  if (!LEInfos.getEntries()) {
    LEInfos.appendNew(L1EmulationInformation());
    B2DEBUG(200, "No entry in L1EmulationInformation");
  }

  m_weightcount = LEInfos[0]->gettotWeight();
  m_ntrg = LEInfos[0]->getnTrg();
  double trgweight = 0.0;
  for (int i = 5; i <= m_ntrg; i++) {
    double trgw = LEInfos[0]->getSummary(i);
    if (trgw > 0)
      trgweight = trgw;
  }
  m_weightcounttrigger += trgweight;
}


void L1EmulationPrintModule::terminate()
{

  std::cout << Form("============================L1 Emulator Output Begin===========================") << "\n";
  std::cout << Form("Total trigger pathes: %2d", m_ntrg) << "\n";
  std::cout << Form("Ws:         The sum of the weight of all processed events") << "\n";
  std::cout << Form("Wst:        The sum of the weight of the events triggered by at least one path") << "\n";
  std::cout << Form("Wsubt:      The sum of the weight of the events triggered by a path") << "\n";
  std::cout << Form("Rs=Wst/Ws:  The ratio of the events triggered by at least one path to Ws") << "\n";
  std::cout << Form("R=Wsubt/Ws: The ratio of the event triggered by a path to Ws") << "\n";
  std::cout << Form("-------------------------------------------------------------------------------") << "\n";
  double ra = 0.0;
  if (m_weightcount > 0) ra = m_weightcounttrigger / m_weightcount * 100.0;
  std::cout << Form("Ws=%-15.2f Wst=%-15.2f Rs=%5.2f%-1s", m_weightcount, m_weightcounttrigger, ra, "%") << "\n";
  std::cout << Form("-------------------------------------------------------------------------------") << "\n";

  std::cout << Form("%-40s%-20s%-10s", "Trigger Name", "Wsubt", "R(%)") << "\n";

}


