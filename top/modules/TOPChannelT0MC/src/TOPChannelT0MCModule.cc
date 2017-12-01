/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Wenlong Yuan                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <top/modules/TOPChannelT0MC/TOPChannelT0MCModule.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework - Database
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/Database.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// DataStore classes
#include <framework/dataobjects/EventMetaData.h>

// database classes
#include <framework/database/DBStore.h>
#include <top/dbobjects/TOPASICChannel.h>

#include <TFile.h>
#include <TH1F.h>
#include <TF1.h>
#include <TTree.h>
#include <sstream>

using namespace std;

namespace Belle2 {
  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPChannelT0MC)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPChannelT0MCModule::TOPChannelT0MCModule() : Module()
  {
    std::vector<double> frange = {100, 0., 1.};
    // set module description (e.g. insert text)
    setDescription("TOP channel T0 Calibration of MC extraction");
    // Add parameters
    addParam("outputFile", m_outputFile, "Output root file name",
             string(""));
    addParam("fitRange", m_fitRange, "fit range[nbins, xmin, xmax]", frange);

  }

  TOPChannelT0MCModule::~TOPChannelT0MCModule()
  {
  }

  void TOPChannelT0MCModule::initialize()
  {

    m_digits.isRequired();

  }

  void TOPChannelT0MCModule::beginRun()
  {
  }

  void TOPChannelT0MCModule::event()
  {

    for (auto& digit : m_digits) {
      unsigned channel = digit.getChannel();
      if (channel < c_NumChannels) {
        auto histo = m_histo[channel];
        if (!histo) {
          stringstream ss;
          ss << "chan" << channel ;
          string name;
          ss >> name;
          string title = "Times " + name;
          histo = new TH1F(name.c_str(), title.c_str(), (int)m_fitRange[0], m_fitRange[1], m_fitRange[2]);
          m_histo[channel] = histo;
        }
        if (digit.getTime() < m_fitRange[1] || digit.getTime() > m_fitRange[2]) continue;
        histo->Fill(digit.getTime());
      }
    }
  }

  void TOPChannelT0MCModule::endRun()
  {
  }

  void TOPChannelT0MCModule::terminate()
  {
    auto file = new TFile(m_outputFile.c_str(), "RECREATE");
    auto otree = new TTree("t0MC", "extract channel t0 info. from MC");

    unsigned channel = 0;
    double maxpos = 0;

    otree->Branch("maxpos", &maxpos, "maxpos/D");
    otree->Branch("channel", &channel, "channel/I");

    for (int i = 0; i < c_NumChannels; i++) {
      channel = i;
      maxpos = m_histo[i]->GetXaxis()->GetBinCenter(m_histo[i]->GetMaximumBin());
      otree->Fill();
    }
    otree->Write();
    delete otree;
    file->Close();
    delete file;
  }
} // end Belle2 namespace

