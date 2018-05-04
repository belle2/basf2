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

    // required input
    m_tracks.isRequired();
    m_pidLikelihoods.registerInDataStore();
    m_tracks.registerRelationTo(m_pidLikelihoods);

    // optional input
    m_topLikelihoods.isOptional();
    m_arichLikelihoods.isOptional();
    m_cdcDedxLikelihoods.isOptional();
    m_vxdDedxLikelihoods.isOptional();
    m_eclLikelihoods.isOptional();
    m_muid.isOptional();
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
    // loop over reconstructed tracks and collect likelihoods
    for (int itra = 0; itra < m_tracks.getEntries(); ++itra) {

      // reconstructed track
      const Track* track = m_tracks[itra];

      const auto charge = track->getTrackFitResultWithClosestMass(Const::pion)->getChargeSign();

      // append new and set relation
      m_pid = m_pidLikelihoods.appendNew();
      track->addRelationTo(m_pid);

      // set top likelihoods
      const TOPLikelihood* top = track->getRelated<TOPLikelihood>();
      if (top) setLikelihoods(top);

      // set arich likelihoods
      const ARICHLikelihood* arich = track->getRelated<ARICHLikelihood>();
      if (arich) setLikelihoods(arich);

      // set CDC dE/dx likelihoods
      const CDCDedxLikelihood* cdcdedx = track->getRelatedTo<CDCDedxLikelihood>();
      if (cdcdedx) setLikelihoods(cdcdedx);

      // set VXD dE/dx likelihoods
      const VXDDedxLikelihood* vxddedx = track->getRelatedTo<VXDDedxLikelihood>();
      if (vxddedx) setLikelihoods(vxddedx);

      // set ecl likelihoods
      const ECLPidLikelihood* ecl = track->getRelatedTo<ECLPidLikelihood>();
      if (ecl) setLikelihoods(ecl, charge);

      // set klm likelihoods
      const Muid* muid = track->getRelatedTo<Muid>();
      if (muid) setLikelihoods(muid);

    }

  }


  void MdstPIDModule::setLikelihoods(const TOPLikelihood* logl)
  {
    if (logl->getFlag() != 1) return;

    for (const auto& chargedStable : Const::chargedStableSet) {
      m_pid->setLogLikelihood(Const::TOP, chargedStable, logl->getLogL(chargedStable));
    }

  }


  void MdstPIDModule::setLikelihoods(const ARICHLikelihood* logl)
  {
    if (logl->getFlag() != 1) return;

    for (const auto& chargedStable : Const::chargedStableSet) {
      m_pid->setLogLikelihood(Const::ARICH, chargedStable, logl->getLogL(chargedStable));
    }

  }


  void MdstPIDModule::setLikelihoods(const CDCDedxLikelihood* logl)
  {

    for (const auto& chargedStable : Const::chargedStableSet) {
      m_pid->setLogLikelihood(Const::CDC, chargedStable, logl->getLogL(chargedStable));
    }

  }


  void MdstPIDModule::setLikelihoods(const VXDDedxLikelihood* logl)
  {

    for (const auto& chargedStable : Const::chargedStableSet) {
      m_pid->setLogLikelihood(Const::SVD, chargedStable, logl->getLogL(chargedStable));
    }

  }


  void MdstPIDModule::setLikelihoods(const ECLPidLikelihood* logl, const short& charge)
  {
    if (!charge) {
      B2WARNING("MdstPID, ECLPid: track has " << charge << "charge. Will not attempt at setting the logL...");
      return;
    }

    for (const auto& chargedStable : Const::chargedStableSet) {
      m_pid->setLogLikelihood(Const::ECL, chargedStable, logl->getLogLikelihood(chargedStable,
                              charge)); // ECLPidLikelihood::getLogLikelihood() method must retrieve the correct likelihood according to the reconstructed charge.
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
    m_pid->setLogLikelihood(Const::KLM, Const::deuteron, muid->getLogL_d());

  }


} // Belle2 namespace

