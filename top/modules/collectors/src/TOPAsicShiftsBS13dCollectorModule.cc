/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/modules/collectors/TOPAsicShiftsBS13dCollectorModule.h>
#include <top/geometry/TOPGeometryPar.h>
#include <string>

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
    addParam("nx", m_nx, "number of histogram bins", 50);

  }


  void TOPAsicShiftsBS13dCollectorModule::prepare()
  {

    m_topDigits.isRequired();

    registerObject<TH2F>(m_time_vs_BS->GetName(), m_time_vs_BS);
    registerObject<TH1F>(m_timeReference->GetName(), m_timeReference);
    for (const auto& h : m_timeCarriers) {
      registerObject<TH1F>(h->GetName(), h);
    }

  }


  void TOPAsicShiftsBS13dCollectorModule::collect()
  {

    for (const auto& digit : m_topDigits) {
      if (digit.getModuleID() != 13) continue;
      if (digit.getHitQuality() != TOPDigit::c_Good) continue;
      double time = digit.getTime() - m_timeOffset;
      m_time_vs_BS->Fill(digit.getChannel(), time);
      if (digit.getBoardstackNumber() < 3) {
        m_timeReference->Fill(time);
      } else {
        auto c = digit.getCarrierNumber();
        m_timeCarriers[c]->Fill(time);
      }
    }

  }

  void TOPAsicShiftsBS13dCollectorModule::inDefineHisto()
  {
    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    double timeStep = geo->getNominalTDC().getSyncTimeBase() / 6;
    double xmi = - m_nx * timeStep / 2;
    double xma = m_nx * timeStep / 2;

    m_time_vs_BS = new TH2F("time_vs_BS", "time vs BS, slot 13",
                            16, 0.0, 512.0, m_nx, xmi, xma);
    m_time_vs_BS->SetXTitle("channel number");
    m_time_vs_BS->SetYTitle("time [ns]");

    m_timeReference = new TH1F("time_reference", "time, slot 13(a, b, c)",
                               m_nx, xmi, xma);
    m_timeReference->SetXTitle("time [ns]");
    m_timeReference->SetYTitle("entries per bin [arbitrary]");

    for (unsigned i = 0; i < 4; i++) {
      string name = "time_carr_" + to_string(i);
      string title = "time, slot 13d, carrier " + to_string(i);
      auto h = new TH1F(name.c_str(), title.c_str(), m_nx, xmi, xma);
      h->SetXTitle("time [ns]");
      h->SetYTitle("entries per bin [arbitrary]");
      m_timeCarriers.push_back(h);
    }
  }


} // end namespace Belle2
