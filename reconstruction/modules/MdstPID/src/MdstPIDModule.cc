/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <reconstruction/modules/MdstPID/MdstPIDModule.h>

/* Belle2 headers. */
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <klm/muid/MuidElementNumbers.h>

/* C++ headers. */
#include <string>

using namespace std;

namespace Belle2 {

  REG_MODULE(MdstPID)

  MdstPIDModule::MdstPIDModule() : Module(),
    m_pid(nullptr)
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
      if (ecl) setLikelihoods(ecl);

      // set klm likelihoods
      const KLMMuidLikelihood* muid = track->getRelatedTo<KLMMuidLikelihood>();
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


  void MdstPIDModule::setLikelihoods(const ECLPidLikelihood* logl)
  {

    for (const auto& chargedStable : Const::chargedStableSet) {
      m_pid->setLogLikelihood(Const::ECL, chargedStable, logl->getLogLikelihood(chargedStable));
    }

  }


  void MdstPIDModule::setLikelihoods(const KLMMuidLikelihood* muid)
  {
    if (abs(muid->getPDGCode()) != abs(Const::muon.getPDGCode())) {
      B2WARNING("MdstPID, KLMMuidLikelihood: extrapolation with other than muon hypothesis ignored");
      return;
    }

    if (muid->getOutcome() == MuidElementNumbers::c_NotReached)
      return; // track extrapolation didn't reach KLM

    if (muid->getJunkPDFValue())
      return; // unclassifiable track (all likelihoods were zero), extremely rare

    for (const auto& chargedStable : Const::chargedStableSet) {
      m_pid->setLogLikelihood(Const::KLM, chargedStable, muid->getLogL(chargedStable.getPDGCode()));
    }
  }


} // Belle2 namespace

