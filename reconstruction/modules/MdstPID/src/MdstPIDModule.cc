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
#include <framework/datastore/StoreArray.h>

// data objects:
#include <mdst/dataobjects/PIDLikelihood.h>
#include <top/dataobjects/TOPLikelihood.h>
#include <arich/dataobjects/ARICHLikelihood.h>
#include <reconstruction/dataobjects/DedxLikelihood.h>
#include <ecl/dataobjects/ECLPidLikelihood.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/Muid.h>

// framework aux
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>

#include <string>


using namespace std;

namespace Belle2 {

  REG_MODULE(MdstPID)

  MdstPIDModule::MdstPIDModule() : Module(),
    m_pid(NULL)
  {
    setDescription("Create MDST PID format (PIDLikelihood objects) from subdetector PID info.");
    setPropertyFlags(c_ParallelProcessingCertified);
  }

  MdstPIDModule::~MdstPIDModule()
  {
  }

  void MdstPIDModule::initialize()
  {
    StoreArray<Track> tracks;
    StoreArray<PIDLikelihood> pidLikelihoods;
    // data store registration
    pidLikelihoods.registerInDataStore();
    tracks.registerRelationTo(pidLikelihoods);

    tracks.isRequired();
    StoreArray<TOPLikelihood>::optional();
    StoreArray<ARICHLikelihood>::optional();
    StoreArray<DedxLikelihood>::optional();
    StoreArray<ECLPidLikelihood>::optional();
    StoreArray<Muid>::optional();
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

    // loop over reconstructed tracks and collect likelihoods
    for (int itra = 0; itra < tracks.getEntries(); ++itra) {

      // reconstructed track
      const Track* track = tracks[itra];

      // append new and set relation
      m_pid = pidLikelihoods.appendNew();
      track->addRelationTo(m_pid);

      // set top likelihoods
      const TOPLikelihood* top = track->getRelated<TOPLikelihood>();
      if (top) setLikelihoods(top);

      // set arich likelihoods
      const ARICHLikelihood* arich = track->getRelated<ARICHLikelihood>();
      if (arich) setLikelihoods(arich);

      // set dedx likelihoods
      const DedxLikelihood* dedx = track->getRelatedTo<DedxLikelihood>();
      if (dedx) setLikelihoods(dedx);

      // set ecl likelihoods
      const ECLPidLikelihood* ecl = track->getRelatedTo<ECLPidLikelihood>();
      if (ecl) setLikelihoods(ecl);

      // set klm likelihoods
      const Muid* muid = track->getRelatedTo<Muid>();
      if (muid) setLikelihoods(muid);

    }

  }


  void MdstPIDModule::setLikelihoods(const TOPLikelihood* logl)
  {
    if (logl->getFlag() != 1) return;

    for (const auto & chargedStable : Const::chargedStableSet) {
      m_pid->setLogLikelihood(Const::TOP, chargedStable, logl->getLogL(chargedStable));
    }

  }


  void MdstPIDModule::setLikelihoods(const ARICHLikelihood* logl)
  {
    if (logl->getFlag() != 1) return;

    for (const auto & chargedStable : Const::chargedStableSet) {
      m_pid->setLogLikelihood(Const::ARICH, chargedStable, logl->getLogL(chargedStable));
    }

  }


  void MdstPIDModule::setLikelihoods(const DedxLikelihood* logl)
  {

    for (const auto & chargedStable : Const::chargedStableSet) {
      m_pid->setLogLikelihood(Const::SVD, chargedStable, logl->getSVDLogLikelihood(chargedStable));
      m_pid->setLogLikelihood(Const::CDC, chargedStable, logl->getCDCLogLikelihood(chargedStable));
    }

  }


  void MdstPIDModule::setLikelihoods(const ECLPidLikelihood* logl)
  {

    for (const auto & chargedStable : Const::chargedStableSet) {
      m_pid->setLogLikelihood(Const::ECL, chargedStable, logl->getLogLikelihood(chargedStable));
    }

  }


  void MdstPIDModule::setLikelihoods(const Muid* muid)
  {

    if (abs(muid->getPDGCode()) != abs(Const::muon.getPDGCode())) {
      B2WARNING("MdstPID, Muid: extrapolation with other than muon hypothesis ignored");
      return;
    }

    if (muid->getOutcome() == 0) return; // muon can't reach KLM

    if (muid->getJunkPDFValue() != 0) return; // unclassifiable track (all likelihoods were zero)

    m_pid->setLogLikelihood(Const::KLM, Const::electron, muid->getLogL_e());
    m_pid->setLogLikelihood(Const::KLM, Const::muon, muid->getLogL_mu());
    m_pid->setLogLikelihood(Const::KLM, Const::pion, muid->getLogL_pi());
    m_pid->setLogLikelihood(Const::KLM, Const::kaon, muid->getLogL_K());
    m_pid->setLogLikelihood(Const::KLM, Const::proton, muid->getLogL_p());

  }


} // Belle2 namespace

