/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/modules/collectors/TOPPulseHeightCollectorModule.h>
#include <TH2F.h>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPPulseHeightCollector)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPPulseHeightCollectorModule::TOPPulseHeightCollectorModule()
  {
    // set module description and processing properties
    setDescription("A collector for channel pulse-height distributions");
    setPropertyFlags(c_ParallelProcessingCertified);

    // module parameters
    addParam("nx", m_nx, "number of histogram bins", 200);
    addParam("xmax", m_xmax, "histogram upper bound [ADC counts]", 2000.0);
    m_widthWindow.push_back(1.3);
    m_widthWindow.push_back(3.0);
    addParam("pulseWidthWindow", m_widthWindow,
             "lower and upper bound of pulse-width window [ns]. "
             "Empty list means no selection on the pulse width.", m_widthWindow);
    addParam("timeWindow", m_timeWindow,
             "lower and upper bound of time window [ns]. "
             "Empty list means no selection on photon time.", m_timeWindow);

  }


  void TOPPulseHeightCollectorModule::prepare()
  {

    m_digits.isRequired();

    for (int slot = 1; slot <= 16; slot++) {
      string name = "ph_slot_" + to_string(slot);
      string title = "pulse-height vs. channel for slot " + to_string(slot);
      auto h = new TH2F(name.c_str(), title.c_str(), 512, 0, 512, m_nx, 0, m_xmax);
      h->SetXTitle("channel number");
      h->SetYTitle("pulse height [ADC counts]");
      registerObject<TH2F>(name, h);
      m_names.push_back(name);
    }

  }


  void TOPPulseHeightCollectorModule::collect()
  {

    std::vector<TH2F*> slots;
    for (const auto& name : m_names) {
      auto h = getObjectPtr<TH2F>(name);
      slots.push_back(h);
    }

    for (const auto& digit : m_digits) {
      if (digit.getHitQuality() != TOPDigit::c_Good) continue;
      if (m_widthWindow.size() == 2) {
        auto w = digit.getPulseWidth();
        if (w < m_widthWindow[0] or w > m_widthWindow[1]) continue;
      }
      if (m_timeWindow.size() == 2) {
        auto t = digit.getTime();
        if (t < m_timeWindow[0] or t > m_timeWindow[1]) continue;
      }
      unsigned m = digit.getModuleID() - 1;
      if (m >= slots.size()) continue;
      slots[m]->Fill(digit.getChannel(), digit.getPulseHeight());
    }

  }

}
