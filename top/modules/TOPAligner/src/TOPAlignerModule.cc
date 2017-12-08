/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <top/modules/TOPAligner/TOPAlignerModule.h>
#include <top/geometry/TOPGeometryPar.h>
#include <top/reconstruction/TOPalign.h>
#include <top/reconstruction/TOPtrack.h>
#include <top/reconstruction/TOPconfigure.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// datastore classes
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <top/dataobjects/TOPDigit.h>

using namespace std;

namespace Belle2 {
  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPAligner)


  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPAlignerModule::TOPAlignerModule() : Module()

  {
    // set module description
    setDescription("Alignment of TOP");
    //    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("minBkgPerBar", m_minBkgPerBar,
             "Minimal number of background photons per module", 0.0);
    addParam("scaleN0", m_scaleN0,
             "Scale factor for figure-of-merit N0", 1.0);
    addParam("targetModule", m_targetMid,
             "Module to be aligned. Must be 1 <= Mid <= 16.", 1);
    addParam("maxFails", m_maxFails,
             "Maximum number of consecutive failed iterations before resetting the procedure", 100);
    addParam("outFileName", m_outFileName,
             "Root output file name containing alignment results", std::string("TopAlignPars.root"));

  }

  TOPAlignerModule::~TOPAlignerModule()
  {
  }

  void TOPAlignerModule::initialize()
  {

    // check if the target module is correctly set

    if (m_targetMid < 1 || m_targetMid > 16)
      B2FATAL("Target ModuleId is not correctly set, use targetModule in [1, 16]. Exiting...");


    // check if target module ID is valid

    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    if (!geo->isModuleIDValid(m_targetMid))
      B2FATAL("Target module " << m_targetMid << " is invalid. Exiting...");

    m_align = TOPalign(m_targetMid);

    // configure detector in reconstruction code

    TOPconfigure config;

    // input

    StoreArray<TOPDigit> digits;
    digits.isRequired();

    StoreArray<Track> tracks;
    tracks.isRequired();

    StoreArray<ExtHit> extHits;
    extHits.isRequired();


    // set counter for failed iterations:

    m_countFails = 0;


    // output file

    m_file = new TFile(m_outFileName.c_str(), "RECREATE");
    if (m_file->IsZombie()) {
      B2FATAL("Couldn't open file '" << m_outFileName << "' for writing!");
      return;
    }


    m_alignTree = new TTree("alignTree", "TOP alignment results");

    m_alignTree->Branch("ModuleId", &m_targetMid);
    m_alignTree->Branch("ntrk", &m_ntrk);
    m_alignTree->Branch("errorCode", &m_errorCode);
    m_alignTree->Branch("iterPars", &m_vAlignPars);

  }

  void TOPAlignerModule::beginRun()
  {
  }

  void TOPAlignerModule::event()
  {

    // add photons

    TOPalign::clearData();

    StoreArray<TOPDigit> digits;
    for (const auto& digit : digits) {
      if (digit.getHitQuality() == TOPDigit::EHitQuality::c_Good)
        TOPalign::addData(digit.getModuleID(), digit.getPixelID(), digit.getTime(),
                          digit.getTimeError());
    }

    TOPalign::setPhotonYields(m_minBkgPerBar, m_scaleN0);

    // track-by-track iterations

    StoreArray<Track> tracks;
    for (const auto& track : tracks) {

      // construct TOPtrack from mdst track
      TOPtrack trk(&track);
      if (!trk.isValid()) continue;

      // do iteration
      int i = trk.getModuleID() - 1;

      // skip if moduleID != target module
      if (m_targetMid != (i + 1)) continue;

      auto& align = m_align;
      int err = align.iterate(trk, Const::muon);

      // check number of consecutive failures, and in case reset

      if (err == 0) m_countFails = 0;
      else if (m_countFails <= m_maxFails) m_countFails++;
      else {
        B2INFO("Reached maximum allowed number of failed iterations. Resetting TOPalign object(s)");
        m_align = TOPalign(m_targetMid);
        m_countFails = 0;
      }

      const std::vector<float>& curPars = align.getParameters();

      m_ntrk = align.getNumTracks();
      m_errorCode = err;
      m_vAlignPars = curPars;

      B2INFO("M=" << align.getModuleID() << " ntr=" << m_ntrk << " err=" << m_errorCode << " v=" << align.isValid()
             << " " << curPars.at(0)
             << " " << curPars.at(1)
             << " " << curPars.at(2)
             << " " << curPars.at(3)
             << " " << curPars.at(4)
             << " " << curPars.at(5)
             << " " << curPars.at(6));

      // fill output tree
      m_alignTree->Fill();

    }

  }


  void TOPAlignerModule::endRun()
  {
  }

  void TOPAlignerModule::terminate()
  {

    m_file->cd();
    m_alignTree->Write();
    m_file->Close();

  }


} // end Belle2 namespace

