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
#include <hlt/L1Emulator/modules/L1Emulation/L1EmulationPrintModule.h>
#include <hlt/L1Emulator/dataobjects/L1EmulationInformation.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
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
  m_ntrg = 0;
  m_nEvent = 0;
  for (int i = 0; i < 50; i++) {
    m_trgname[i] = "NULL";
    m_summary[i] = 0.0;
    m_summaryonly[i] = 0.0;
  }
}


void L1EmulationPrintModule::event()
{
  m_nEvent++;
  StoreArray<L1EmulationInformation> LEInfos;
  if (!LEInfos.getEntries()) {
    LEInfos.appendNew(L1EmulationInformation());
    B2DEBUG(200, "No entry in L1EmulationInformation");
  }

  StoreObjPtr<EventMetaData> eventmetadata;
  double Weight = 0.;
  if (eventmetadata)
    Weight = eventmetadata->getGeneratedWeight();
  m_weightcount += Weight;

  LEInfos[0]->settotWeight(m_weightcount);
  m_ntrg = LEInfos[0]->getnTrg();
  int trgn = 0;
  int remInd = 0;
  double weight = 0.0;
  for (int i = 0; i <= m_ntrg; i++) {
    double trgw = LEInfos[0]->getSummary(i);
    m_summary[i] += trgw;
    if (trgw > 0 && i > 4) {
      trgn++;
      remInd = i;
      weight = trgw;
    }
  }
  if (trgn == 1) m_summaryonly[remInd] += weight;


  if (m_nEvent == 1) {
    for (int i = 0; i < 50; i++)
      m_trgname[i] = LEInfos[0]->getTrgName(i);
  }
}


void L1EmulationPrintModule::terminate()
{

  std::cout << "\n" << "\n";
  std::cout << Form("===============================L1 Emulator Output Begin==============================") << "\n";
  std::cout << Form("Total trigger pathes: %2d", m_ntrg) << "\n";
  std::cout << Form("Ws:         The sum of the weight of all processed events") << "\n";
  std::cout << Form("Wst:        The sum of the weight of the events triggered by at least one path") << "\n";
  std::cout << Form("Wsubt:      The sum of the weight of the events triggered by a path") << "\n";
  std::cout << Form("Wosubt:      The sum of the weight of the events triggered by a unique path") << "\n";
  std::cout <<
            Form("Rs=Wst/Ws:  The ratio of the events triggered by at least one path to Ws. The pathes here don't include the veto logics") <<
            "\n";
  std::cout << Form("R=Wsubt/Ws: The ratio of the event triggered by a path to Ws.") << "\n";
  std::cout <<
            Form("Ro=Wosubt/Ws: The ratio of the event triggered by a unique path to Ws, this value reflects how many events this path could rescue")
            << "\n";
  std::cout << Form("-------------------------------------------------------------------------------------") << "\n";
  double ra = 0.0;
  if (m_weightcount > 0) ra = m_summary[0] / m_weightcount * 100.0;
  std::cout << Form("Ws=%-15.2f Wst=%-15.2f Rs=%5.2f%-1s", m_weightcount, m_summary[0], ra, "%") << "\n";
  std::cout << Form("-------------------------------------------------------------------------------------") << "\n";

  std::cout << Form("%-40s%-20s%-20s%-20s", "Trigger Name", "Wsubt", "R(%)", "Ro(%)") << "\n";
  for (int i = 0; i < 4; i++)
    std::cout << Form("%-40s%-20.2f%-20.2f", m_trgname[i].c_str(), m_summary[i + 1], m_summary[i + 1] / m_weightcount * 100.) << "\n";
  std::cout << Form("-------------------------------------------------------------------------------------") << "\n";

  for (int i = 4; i < m_ntrg; i++)
    std::cout << Form("%-40s%-20.2f%-20.2f%-20.2f", m_trgname[i].c_str(), m_summary[i + 1], m_summary[i + 1] / m_weightcount * 100.,
                      m_summaryonly[i + 1] / m_weightcount * 100.) << "\n";
  std::cout << Form("=================================L1 Emulator Output End==============================") << "\n" << "\n" << "\n";

}


