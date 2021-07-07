/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/modules/collectors/TOPPulseHeightCollectorModule.h>
#include <TH1F.h>
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
    addParam("xmax", m_xmax, "histogram upper limit [ADC counts]", 2000.0);
    addParam("pulseWidthWindow", m_widthWindow,
             "lower and upper bound of pulse-width selection window [ns]. "
             "Empty list means no selection on the pulse width. "
             "Note: selection on pulse width will influence pulse-height distribution.",
             m_widthWindow);
    addParam("timeWindow", m_timeWindow,
             "lower and upper bound of time selection window [ns]. "
             "Empty list means no selection on photon time.", m_timeWindow);

  }


  void TOPPulseHeightCollectorModule::prepare()
  {

    m_digits.isRequired();

    auto h1a = new TH1F("time", "time distribution (all hits)", 1000, -100, 250);
    h1a->SetXTitle("time [ns]");
    registerObject<TH1F>("time", h1a);

    auto h1b = new TH1F("time_sel", "time distribution (selected hits)", 1000, -100, 250);
    h1b->SetXTitle("time [ns]");
    registerObject<TH1F>("time_sel", h1b);

    auto h2a = new TH2F("ph_vs_width", "pulse height vs. width (all hits)",
                        200, 0, 10, 200, 0, 2000);
    h2a->SetXTitle("pulse width [ns]");
    h2a->SetYTitle("pulse height [ADC counts]");
    registerObject<TH2F>("ph_vs_width", h2a);

    auto h2b = new TH2F("ph_vs_width_sel", "pulse height vs. width (selected hits)",
                        200, 0, 10, 200, 0, 2000);
    h2b->SetXTitle("pulse width [ns]");
    h2b->SetYTitle("pulse height [ADC counts]");
    registerObject<TH2F>("ph_vs_width_sel", h2b);

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

    auto h1a = getObjectPtr<TH1F>("time");
    auto h1b = getObjectPtr<TH1F>("time_sel");
    auto h2a = getObjectPtr<TH2F>("ph_vs_width");
    auto h2b = getObjectPtr<TH2F>("ph_vs_width_sel");

    for (const auto& digit : m_digits) {
      if (digit.getHitQuality() != TOPDigit::c_Good) continue;
      auto t = digit.getTime();
      auto w = digit.getPulseWidth();
      auto ph = digit.getPulseHeight();
      h1a->Fill(t);
      h2a->Fill(w, ph);
      if (m_widthWindow.size() == 2) {
        if (w < m_widthWindow[0] or w > m_widthWindow[1]) continue;
      }
      if (m_timeWindow.size() == 2) {
        if (t < m_timeWindow[0] or t > m_timeWindow[1]) continue;
      }
      h1b->Fill(t);
      h2b->Fill(w, ph);
      unsigned m = digit.getModuleID() - 1;
      if (m < slots.size()) slots[m]->Fill(digit.getChannel(), ph);
    }

  }

}
