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


  }

  TOPAlignerModule::~TOPAlignerModule()
  {
  }

  void TOPAlignerModule::initialize()
  {

    // input

    StoreArray<TOPDigit> digits;
    digits.isRequired();

    StoreArray<Track> tracks;
    tracks.isRequired();

    StoreArray<ExtHit> extHits;
    extHits.isRequired();

    // Configure TOP detector

    TOPconfigure config;

    // Construct alignment objects (vector index == moduleID - 1)

    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    for (unsigned i = 0; i < geo->getNumModules(); i++) {
      m_align.push_back(TOPalign(i + 1));
    }

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
        TOPalign::addData(digit.getModuleID(), digit.getPixelID(), digit.getTime());
    }

    TOPalign::setPhotonYields(m_minBkgPerBar, m_scaleN0);

    // track-by-track iterations

    StoreArray<Track> tracks;
    for (const auto& track : tracks) {

      // construct TOPtrack from mdst track
      TOPtrack trk(&track);
      if (!trk.isValid()) continue;

      // do iteration
      unsigned i = trk.getModuleID() - 1;
      if (i < m_align.size()) {
        auto& align = m_align[i];
        int err = align.iterate(trk, Const::muon);

        cout << "M=" << align.getModuleID() << " ";
        cout << "ntr=" << align.getNumTracks() << " ";
        cout << "err=" << err << " ";
        cout << "v=" << align.isValid() << " ";
        for (const auto& par : align.getParameters()) cout << " " << par;
        cout << endl;

      }

    }

  }


  void TOPAlignerModule::endRun()
  {
  }

  void TOPAlignerModule::terminate()
  {
  }


} // end Belle2 namespace

