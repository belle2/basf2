/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/modules/MdstPID/MdstPIDModule.h>

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

// framework aux
#include <framework/logging/Logger.h>

#include <string>


using namespace std;

namespace Belle2 {

  REG_MODULE(MdstPID)

  MdstPIDModule::MdstPIDModule() : Module()
  {
    setDescription("Create MDST PID format (PIDLikelihood objects) from subdetector PID info.");
    setPropertyFlags(c_ParallelProcessingCertified);
  }

  MdstPIDModule::~MdstPIDModule()
  {
  }

  void MdstPIDModule::initialize()
  {
    // data store registration
    StoreArray<PIDLikelihood>::registerPersistent();
    RelationArray::registerPersistent<Track, PIDLikelihood>();

  }


  void MdstPIDModule::beginRun()
  {
  }

  void MdstPIDModule::endRun()
  {
  }

  void MdstPIDModule::terminate()
  {
  }

  void MdstPIDModule::event()
  {
    // input: reconstructed tracks
    StoreArray<Track> tracks;

    // output
    StoreArray<PIDLikelihood> pidLikelihoods;
    pidLikelihoods.create();

    // loop over reconstructed tracks and collect likelihoods
    for (int itra = 0; itra < tracks.getEntries(); ++itra) {

      // reconstructed track
      const Track* track = tracks[itra];

      // append new and set relation
      PIDLikelihood* pid = pidLikelihoods.appendNew();
      DataStore::addRelationFromTo(track, pid);

      // set top likelihoods
      const TOPLikelihood* top = DataStore::getRelated<TOPLikelihood>(track);
      if (top) pid->setLikelihoods(top);

      // set arich likelihoods
      const ARICHLikelihoods* arich = DataStore::getRelated<ARICHLikelihoods>(track);
      if (arich) pid->setLikelihoods(arich);

      // set dedx likelihoods
      const DedxLikelihood* dedx = track->getRelatedTo<DedxLikelihood>();
      if (dedx) pid->setLikelihoods(dedx);

    }

  }


  void MdstPIDModule::printModuleParams() const
  {
  }


} // Belle2 namespace

