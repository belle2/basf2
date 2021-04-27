/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/modules/collectors/TOPChannelMaskCollectorModule.h>
#include <TH1F.h>
#include <TH2F.h>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPChannelMaskCollector)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPChannelMaskCollectorModule::TOPChannelMaskCollectorModule()
  {
    // set module description and processing properties
    setDescription("A collector for preparing masks of dead and hot channels");
    setPropertyFlags(c_ParallelProcessingCertified);
  }


  void TOPChannelMaskCollectorModule::prepare()
  {

    m_digits.isRequired();

    auto nhits = new TH1F("nhits", "Number of good hits per event; hits per event; entries per bin", 200, 0, 2000);
    registerObject<TH1F>("nhits", nhits);

    for (int slot = 1; slot <= 16; slot++) {
      string name = "hits_" + to_string(slot);
      string title = "Channel occupancies for slot " + to_string(slot);
      auto h = new TH1F(name.c_str(), title.c_str(), 512, 0, 512);
      h->SetXTitle("channel number");
      h->SetYTitle("number of hits per channel");
      registerObject<TH1F>(name, h);
      m_names.push_back(name);
    }

    for (int slot = 1; slot <= 16; slot++) {
      string name = "window_vs_asic_" + to_string(slot);
      string title = "Window vs. asic for slot " + to_string(slot);
      auto h = new TH2F(name.c_str(), title.c_str(), 64, 0, 64, 512, 0, 512);
      h->SetXTitle("ASIC number");
      h->SetYTitle("window number w.r.t reference window");
      registerObject<TH2F>(name, h);
      m_asicNames.push_back(name);
    }

  }


  void TOPChannelMaskCollectorModule::collect()
  {

    std::vector<TH1F*> slots;
    for (const auto& name : m_names) {
      auto h = getObjectPtr<TH1F>(name);
      slots.push_back(h);
    }

    std::vector<TH2F*> asics;
    for (const auto& name : m_asicNames) {
      auto h = getObjectPtr<TH2F>(name);
      asics.push_back(h);
    }

    int NGood = 0;
    for (const auto& digit : m_digits) {
      if (digit.getHitQuality() == TOPDigit::c_Junk) continue;
      NGood++;
      unsigned m = digit.getModuleID() - 1;
      if (m < slots.size()) slots[m]->Fill(digit.getChannel());
      if (m < asics.size()) asics[m]->Fill(digit.getChannel() / 8, digit.getRawTime() / 64 + 220);
    }

    auto h = getObjectPtr<TH1F>("nhits");
    h->Fill(NGood);

  }

}

