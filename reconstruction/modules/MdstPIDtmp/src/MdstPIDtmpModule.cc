/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/modules/MdstPIDtmp/MdstPIDtmpModule.h>
#include <framework/core/ModuleManager.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

// data objects:
#include <reconstruction/dataobjects/PIDLikelihood.h>
#include <top/dataobjects/TOPLikelihood.h>
#include <arich/dataobjects/ARICHLikelihoods.h>
#include <reconstruction/dataobjects/DedxLikelihood.h>
#include <tracking/dataobjects/Track.h>
#include <GFTrack.h>
#include <arich/dataobjects/ARICHAeroHit.h>
#include <generators/dataobjects/MCParticle.h>

// framework aux
#include <framework/logging/Logger.h>

#include <string>


using namespace std;

namespace Belle2 {

  REG_MODULE(MdstPIDtmp)

  MdstPIDtmpModule::MdstPIDtmpModule() : Module()
  {
    setDescription("Create MDST PID format (PIDLikelihood objects) from subdetector PID info.");
    setPropertyFlags(c_ParallelProcessingCertified);
  }

  MdstPIDtmpModule::~MdstPIDtmpModule()
  {
  }

  void MdstPIDtmpModule::initialize()
  {
    // data store registration
    StoreArray<PIDLikelihood>::registerPersistent();
    RelationArray::registerPersistent<GFTrack, PIDLikelihood>();
    RelationArray::registerPersistent<Track, PIDLikelihood>();

  }


  void MdstPIDtmpModule::beginRun()
  {
  }

  void MdstPIDtmpModule::endRun()
  {
  }

  void MdstPIDtmpModule::terminate()
  {
  }

  void MdstPIDtmpModule::event()
  {
    // reconstructed tracks
    StoreArray<GFTrack> tracks;

    // output
    StoreArray<PIDLikelihood> pidLikelihoods;
    pidLikelihoods.create();

    // loop over reconstructed tracks and collect likelihoods
    for (int itra = 0; itra < tracks.getEntries(); ++itra) {

      // append new and set relation
      PIDLikelihood* pid = pidLikelihoods.appendNew();
      DataStore::addRelationFromTo(tracks[itra], pid);

      // reconstructed track
      const GFTrack* gfTrack = tracks[itra];

      // set top likelihoods
      const TOPLikelihood* top = DataStore::getRelated<TOPLikelihood>(gfTrack);
      if (top) pid->setLikelihoods(top);

      // set arich likelihoods
      const MCParticle* part = DataStore::getRelated<MCParticle>(gfTrack);
      if (part) {
        const ARICHAeroHit* aero = part->getRelated<ARICHAeroHit>();
        const ARICHLikelihoods* arich = DataStore::getRelated<ARICHLikelihoods>(aero);
        if (arich) pid->setLikelihoods(arich);
      }

      // set dedx likelihoods
      const DedxLikelihood* dedx = DataStore::getRelated<DedxLikelihood>(gfTrack);
      if (dedx) pid->setLikelihoods(dedx);

      // relation between mdst track and likelihoods
      const Track* track = DataStore::getRelated<Track>(gfTrack);
      if (track) {
        DataStore::addRelationFromTo(track, pid);
      }
    }

  }


  void MdstPIDtmpModule::printModuleParams() const
  {
  }


} // Belle2 namespace

