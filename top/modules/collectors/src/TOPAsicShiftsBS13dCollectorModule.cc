/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/modules/collectors/TOPAsicShiftsBS13dCollectorModule.h>
#include <top/geometry/TOPGeometryPar.h>
#include <string>
#include <vector>
#include <TH1F.h>
#include <TH2F.h>

using namespace std;

namespace Belle2 {

  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPAsicShiftsBS13dCollector)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPAsicShiftsBS13dCollectorModule::TOPAsicShiftsBS13dCollectorModule()
  {
    // set module description and processing properties
    setDescription("A collector for calibration of carrier shifts of BS13d.");
    setPropertyFlags(c_ParallelProcessingCertified);

    // module parameters
    addParam("timeOffset", m_timeOffset, "time offset", 0.0);
    addParam("nx", m_nx, "number of histogram bins (bin size ~8 ns)", 50);
    addParam("requireRecBunch", m_requireRecBunch,
             "if True, require reconstructed bunch (to be used on cdst files only!)",
             false);

  }


  void TOPAsicShiftsBS13dCollectorModule::prepare()
  {

    m_topDigits.isRequired();
    if (m_requireRecBunch) {
      m_recBunch.isRequired();
    } else {
      m_recBunch.isOptional();
    }

    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    double timeStep = geo->getNominalTDC().getSyncTimeBase() / 6;
    double xmi = - m_nx * timeStep / 2;
    double xma = m_nx * timeStep / 2;

    auto time_vs_BS = new TH2F("time_vs_BS", "time vs BS, slot 13",
                               16, 0.0, 512.0, m_nx, xmi, xma);
    time_vs_BS->SetXTitle("channel number");
    time_vs_BS->SetYTitle("time [ns]");
    registerObject<TH2F>("time_vs_BS", time_vs_BS);

    auto timeReference = new TH1F("time_reference", "time, slot 13(a, b, c)",
                                  m_nx, xmi, xma);
    timeReference->SetXTitle("time [ns]");
    timeReference->SetYTitle("entries per bin [arbitrary]");
    registerObject<TH1F>("time_reference", timeReference);

    for (unsigned i = 0; i < 4; i++) {
      string name = "time_carr_" + to_string(i);
      string title = "time, slot 13d, carrier " + to_string(i);
      auto h = new TH1F(name.c_str(), title.c_str(), m_nx, xmi, xma);
      h->SetXTitle("time [ns]");
      h->SetYTitle("entries per bin [arbitrary]");
      registerObject<TH1F>(name, h);
    }

  }


  void TOPAsicShiftsBS13dCollectorModule::collect()
  {

    if (m_requireRecBunch) {
      if (not m_recBunch.isValid()) return;
      if (not m_recBunch->isReconstructed()) return;
    }

    auto time_vs_BS = getObjectPtr<TH2F>("time_vs_BS");
    auto timeReference = getObjectPtr<TH1F>("time_reference");
    std::vector<TH1F*> timeCarriers;
    for (unsigned i = 0; i < 4; i++) {
      string name = "time_carr_" + to_string(i);
      auto h = getObjectPtr<TH1F>(name);
      timeCarriers.push_back(h);
    }

    for (const auto& digit : m_topDigits) {
      if (digit.getModuleID() != 13) continue;
      if (digit.getHitQuality() != TOPDigit::c_Good) continue;
      double time = digit.getTime() - m_timeOffset;
      time_vs_BS->Fill(digit.getChannel(), time);
      if (digit.getBoardstackNumber() < 3) {
        timeReference->Fill(time);
      } else {
        auto c = digit.getCarrierNumber();
        timeCarriers[c]->Fill(time);
      }
    }

  }


} // end namespace Belle2
