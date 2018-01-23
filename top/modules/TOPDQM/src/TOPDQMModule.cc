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
#include <top/geometry/TOPGeometryPar.h>

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
#include <top/dataobjects/TOPLikelihood.h>
#include <mdst/dataobjects/Track.h>

// root
#include "TVector3.h"
#include "TDirectory.h"

// boost
#include <boost/format.hpp>

using namespace std;
using boost::format;

namespace Belle2 {

  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPDQM)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPDQMModule::TOPDQMModule() : HistoModule()
  {
    // set module description (e.g. insert text)
    setDescription("TOP DQM histogrammer");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("histogramDirectoryName", m_histogramDirectoryName,
             "histogram directory in ROOT file", string("TOP"));
    addParam("momentumCut", m_momentumCut,
             "momentum cut used to histogram pulls etc.", 2.0);
    addParam("pValueCut", m_pValueCut,
             "track p-value cut used to histogram pulls etc.", 0.001);
    addParam("usePionID", m_usePionID,
             "use pion ID from TOP to histogram pulls etc.", true);
    // FIXME
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
    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    m_numModules = geo->getNumModules();
    int numTDCbins = geo->getNominalTDC().getNumWindows() * 64;

    // book histograms
    m_barHits = new TH1F("barHits", "Number of hits per bar",
                         m_numModules, 0.5, m_numModules + 0.5);
    m_barHits->GetXaxis()->SetTitle("bar ID");
    m_barHits->GetYaxis()->SetTitle("hits per bar");

    for (int i = 0; i < m_numModules; i++) {
      int moduleID = i + 1;
      string name = str(format("hitsBar%1%") % (moduleID));
      string title = str(format("Number of hits per pixel, bar#%1%") % (moduleID));
      int numPixels = geo->getModule(moduleID).getPMTArray().getNumPixels();
      TH1F* h1 = new TH1F(name.c_str(), title.c_str(),
                          numPixels, 0.5, numPixels + 0.5);
      h1->GetXaxis()->SetTitle("pixel ID");
      h1->GetYaxis()->SetTitle("hits per pixel");
      m_pixelHits.push_back(h1);
    }

    for (int i = 0; i < m_numModules; i++) {
      int moduleID = i + 1;
      string name = str(format("timeBar%1%") % (moduleID));
      string title = str(format("Time distribution, bar#%1%") % (moduleID));
      TH1F* h1 = new TH1F(name.c_str(), title.c_str(),
                          numTDCbins, 0, numTDCbins);
      h1->GetXaxis()->SetTitle("time [samples]");
      h1->GetYaxis()->SetTitle("hits per sample");
      m_hitTimes.push_back(h1);
    }


    m_recoTime = new TH1F("recoTime", "reco: time distribution",
                          500, 0, 50);
    m_recoTime->GetXaxis()->SetTitle("time [ns]");

    m_recoTimeBg = new TH1F("recoTimeBg", "reco: time distribution (bkg)",
                            500, 0, 50);
    m_recoTimeBg->GetXaxis()->SetTitle("time [ns]");

    m_recoTimeMinT0 = new TH1F("recoTimeMinT0", "reco: time in respect to first PDF peak",
                               200, -10, 10);
    m_recoTimeMinT0->GetXaxis()->SetTitle("time [ns]");

    m_recoTimeDiff = new TH1F("recoTimeDiff", "reco: time resolution",
                              100, -1.0, 1.0);
    m_recoTimeDiff->GetXaxis()->SetTitle("time residual [ns]");

    m_recoTimeDiff_Phic = new TH2F("recoTimeDiff_Phic",
                                   "reco: time resolution vs. phiCer",
                                   90, -180, 180, 100, -1.0, 1.0);
    m_recoTimeDiff_Phic->GetXaxis()->SetTitle("Cerenkov azimuthal angle [deg]");
    m_recoTimeDiff_Phic->GetYaxis()->SetTitle("time residuals [ns]");

    // cd back to root directory
    oldDir->cd();
  }

  void TOPDQMModule::initialize()
  {
    // Register histograms (calls back defineHisto)
    REG_HISTOGRAM;

    // register dataobjects
    m_digits.isRequired();
    m_tracks.isOptional();

  }

  void TOPDQMModule::beginRun()
  {
  }

  void TOPDQMModule::event()
  {

    for (const auto& digit : m_digits) {
      m_barHits->Fill(digit.getModuleID());
      int i = digit.getModuleID() - 1;
      if (i < 0 || i >= m_numModules) {
        B2ERROR("Invalid module ID found in TOPDigits: ID = " << i + 1);
        continue;
      }
      m_pixelHits[i]->Fill(digit.getPixelID());
      m_hitTimes[i]->Fill(digit.getRawTime());
    }

    for (const auto& track : m_tracks) {
      const auto* trackFit = track.getTrackFitResultWithClosestMass(Const::pion);
      if (!trackFit) continue;
      if (trackFit->getMomentum().Mag() < m_momentumCut) continue;
      if (trackFit->getPValue() < m_pValueCut) continue;
      if (m_usePionID) {
        const auto* top = track.getRelated<TOPLikelihood>();
        if (!top) continue;
        if (top->getLogL_pi() < top->getLogL_K()) continue;
        if (top->getLogL_pi() < top->getLogL_p()) continue;
      }

    }

  }


  void TOPDQMModule::endRun()
  {
  }

  void TOPDQMModule::terminate()
  {
  }


} // end Belle2 namespace

