/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/modules/collectors/TOPModuleT0DeltaTCollectorModule.h>
#include <string>
#include <TH1F.h>
#include <TH2F.h>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPModuleT0DeltaTCollector)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPModuleT0DeltaTCollectorModule::TOPModuleT0DeltaTCollectorModule()
  {
    // set module description and processing properties
    setDescription("A collector for module T0 calibration with chi2 minimization of "
                   "time differences between slots (method DeltaT). Useful for initial "
                   "(rough) calibration, since the results are found slightly biased. "
                   "For the final (precise) calibration one has to use LL method.");
    setPropertyFlags(c_ParallelProcessingCertified);

    // module parameters
    addParam("numBins", m_numBins,
             "number of bins of histograms of time difference", 400);
    addParam("timeRange", m_timeRange,
             "time range [ns] of histograms of time difference", 20.0);

  }


  void TOPModuleT0DeltaTCollectorModule::prepare()
  {

    m_timeZeros.isRequired();

    auto slotPairs = new TH2F("slots", "slot pairs: number of events",
                              16, 0.5, 16.5, 16, 0.5, 16.5);
    slotPairs->SetXTitle("first slot number");
    slotPairs->SetYTitle("second slot number");
    registerObject<TH2F>("slots", slotPairs);

    double xmin = -m_timeRange / 2;
    double xmax = m_timeRange / 2;
    for (int slot1 = 1; slot1 <= 9; slot1++) {
      for (int slot2 = slot1 + 7; slot2 <= slot1 + 9; slot2++) {
        if (slot2 > 16) continue;
        string name = "deltaT0_" + to_string(slot1) + "-" + to_string(slot2);
        string title = "time difference: slot " + to_string(slot1) + " - slot "
                       + to_string(slot2);
        auto h = new TH1F(name.c_str(), title.c_str(), m_numBins, xmin, xmax);
        h->SetXTitle("time difference [ns]");
        registerObject<TH1F>(name, h);
      }
    }

  }


  void TOPModuleT0DeltaTCollectorModule::collect()
  {

    if (m_timeZeros.getEntries() != 2) return;

    const auto* timeZero1 = m_timeZeros[0];
    const auto* timeZero2 = m_timeZeros[1];
    if (timeZero1->getModuleID() > timeZero2->getModuleID()) {
      timeZero1 = m_timeZeros[1];
      timeZero2 = m_timeZeros[0];
    }

    int slot1 = timeZero1->getModuleID();
    int slot2 = timeZero2->getModuleID();
    if (slot1 > 9 or slot2 - slot1 < 7 or slot2 - slot1 > 9) return;
    string name = "deltaT0_" + to_string(slot1) + "-" + to_string(slot2);
    auto h = getObjectPtr<TH1F>(name);
    if (not h) return;

    double t1 = timeZero1->getTime();
    if (m_moduleT0->isCalibrated(slot1)) t1 += m_moduleT0->getT0(slot1);
    double t2 = timeZero2->getTime();
    if (m_moduleT0->isCalibrated(slot2)) t2 += m_moduleT0->getT0(slot2);
    h->Fill(t1 - t2);

    auto slotPairs = getObjectPtr<TH2F>("slots");
    slotPairs->Fill(slot1, slot2);
  }

}
