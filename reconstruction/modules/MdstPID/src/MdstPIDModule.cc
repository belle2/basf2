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
#include <framework/datastore/RelationArray.h>

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
    // data store registration
    StoreArray<PIDLikelihood>::registerPersistent();
    RelationArray::registerPersistent<Track, PIDLikelihood>();

    StoreArray<Track>::required();
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
      const TOPLikelihood* top = DataStore::getRelated<TOPLikelihood>(track);
      if (top) setLikelihoods(top);

      // set arich likelihoods
      const ARICHLikelihood* arich = DataStore::getRelated<ARICHLikelihood>(track);
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

    m_pid->setLogLikelihood(Const::TOP, Const::electron, (float) logl->getLogL_e());
    m_pid->setLogLikelihood(Const::TOP, Const::muon, (float) logl->getLogL_mu());
    m_pid->setLogLikelihood(Const::TOP, Const::pion, (float) logl->getLogL_pi());
    m_pid->setLogLikelihood(Const::TOP, Const::kaon, (float) logl->getLogL_K());
    m_pid->setLogLikelihood(Const::TOP, Const::proton, (float) logl->getLogL_p());

  }


  void MdstPIDModule::setLikelihoods(const ARICHLikelihood* logl)
  {
    if (logl->getFlag() != 1) return;

    m_pid->setLogLikelihood(Const::ARICH, Const::electron, (float) logl->getLogL_e());
    m_pid->setLogLikelihood(Const::ARICH, Const::muon, (float) logl->getLogL_mu());
    m_pid->setLogLikelihood(Const::ARICH, Const::pion, (float) logl->getLogL_pi());
    m_pid->setLogLikelihood(Const::ARICH, Const::kaon, (float) logl->getLogL_K());
    m_pid->setLogLikelihood(Const::ARICH, Const::proton, (float) logl->getLogL_p());

  }


  void MdstPIDModule::setLikelihoods(const DedxLikelihood* logl)
  {
    for (Const::ParticleType k = Const::chargedStableSet.begin(); k != Const::chargedStableSet.end(); ++k) {
      m_pid->setLogLikelihood(Const::SVD, k, logl->getSVDLogLikelihood(k));
      m_pid->setLogLikelihood(Const::CDC, k, logl->getCDCLogLikelihood(k));
    }
  }


  void MdstPIDModule::setLikelihoods(const ECLPidLikelihood* logl)
  {
    for (Const::ParticleType k = Const::chargedStableSet.begin(); k != Const::chargedStableSet.end(); ++k)
      m_pid->setLogLikelihood(Const::ECL, k, logl->getLogLikelihood(k));
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

