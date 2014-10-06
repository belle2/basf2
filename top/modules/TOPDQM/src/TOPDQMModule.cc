/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric,  Dan Santel                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Module manager
#include <framework/core/HistoModule.h>

// Own include
#include <top/modules/TOPDQM/TOPDQMModule.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// dataobject classes
#include <top/dataobjects/TOPDigit.h>

// root
#include "TH1F.h"
#include "TH2F.h"
#include "TVector3.h"
#include "TDirectory.h"

// boost
#include <boost/format.hpp>

using namespace std;
using boost::format;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPDQM)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPDQMModule::TOPDQMModule() : HistoModule(),
    m_topgp(TOP::TOPGeometryPar::Instance()),
    m_barHits(NULL)

  {
    // set module description (e.g. insert text)
    setDescription("TOP DQM histogrammer");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("histogramDirectoryName", m_histogramDirectoryName,
             "histogram directory in ROOT file", string("top"));

  }

  TOPDQMModule::~TOPDQMModule()
  {
  }

  void TOPDQMModule::defineHisto()
  {
    // Create a separate histogram directory and cd into it.
    TDirectory* oldDir = gDirectory;
    oldDir->mkdir(m_histogramDirectoryName.c_str())->cd();

    // variables needed for booking
    int NumBars = m_topgp->getNbars();
    int NumChannels = m_topgp->getNpmtx() * m_topgp->getNpmty() * m_topgp->getNpadx() *
                      m_topgp->getNpady();
    int NumTDCbins = 1 << m_topgp->getTDCbits();

    // book histograms
    m_barHits = new TH1F("barHits", "Number of hits per bar",
                         NumBars, 0.5, NumBars + 0.5);
    m_barHits->GetXaxis()->SetTitle("bar ID");
    m_barHits->GetYaxis()->SetTitle("hits per bar");

    for (int i = 0; i < NumBars; i++) {
      string name = str(format("hitsBar%1%") % (i + 1));
      string title = str(format("Number of hits per channel, bar#%1%") % (i + 1));
      TH1F* h1 = new TH1F(name.c_str(), title.c_str(),
                          NumChannels, 0.5, NumChannels + 0.5);
      h1->GetXaxis()->SetTitle("channel ID");
      h1->GetYaxis()->SetTitle("hits per channel");
      m_channelHits.push_back(h1);
    }

    for (int i = 0; i < NumBars; i++) {
      string name = str(format("timeBar%1%") % (i + 1));
      string title = str(format("Time distribution, bar#%1%") % (i + 1));
      TH1F* h1 = new TH1F(name.c_str(), title.c_str(),
                          NumTDCbins, 0, NumTDCbins);
      h1->GetXaxis()->SetTitle("time [TDC bins]");
      h1->GetYaxis()->SetTitle("hits per TDC bin");
      m_hitTimes.push_back(h1);
    }

    // cd back to root directory
    oldDir->cd();
  }

  void TOPDQMModule::initialize()
  {
    if (m_topgp->getNbars() == 0) {
      B2ERROR("TOP geometry not loaded");
      return;
    }

    // Register histograms (calls back defineHisto)
    REG_HISTOGRAM;

    StoreArray<TOPDigit>::required();

  }

  void TOPDQMModule::beginRun()
  {
  }

  void TOPDQMModule::event()
  {

    StoreArray<TOPDigit> digits;

    for (TOPDigit & digit : digits) {
      m_barHits->Fill(digit.getBarID());
      int i = digit.getBarID() - 1;
      if (i < 0 || i >= m_topgp->getNbars()) {
        B2ERROR("Invalid bar ID found in TOPDigit");
        continue;
      }
      m_channelHits[i]->Fill(digit.getChannelID());
      m_hitTimes[i]->Fill(digit.getTDC());
    }

  }


  void TOPDQMModule::endRun()
  {
  }

  void TOPDQMModule::terminate()
  {
  }


} // end Belle2 namespace

