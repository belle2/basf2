/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <analysis/modules/DeepFEIExtractor/DeepFEIExtractorModule.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <analysis/dataobjects/EventExtraInfo.h>
#include <analysis/dataobjects/RestOfEvent.h>

#include <analysis/VariableManager/Variables.h>
#include <analysis/utility/PCmsLabTransform.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/PIDLikelihood.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>


namespace Belle2 {


  REG_MODULE(DeepFEIExtractor)

  DeepFEIExtractorModule::DeepFEIExtractorModule() : Module()
  {
    setDescription("Writes out information about the current ROE object in the datastore which can than later be used in a deep neural network to train the deepFEI algorithm.");
    addParam("filename", m_filename, "Name of output file.");
  }

  void DeepFEIExtractorModule::initialize()
  {

    m_file.open(m_filename, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
    if (not m_file.is_open() or not m_file.good()) {
      B2FATAL("Couldn't open file " << m_filename);
    }

  }

  void DeepFEIExtractorModule::terminate()
  {
    m_file.close();
  }

  void DeepFEIExtractorModule::event()
  {
    StoreObjPtr<RestOfEvent> roe("RestOfEvent");

    if (roe.isValid()) {

      {
        const Particle* particle = roe->getRelated<Particle>();
        if (particle == nullptr) {
          B2ERROR("Encountered nullptr as particle related to ROE. This should never happen.");
          return;
        }

        TLorentzVector momentum = PCmsLabTransform::labToCms(particle->get4Vector());
        TVector3 position = particle->getVertex();

        m_file << static_cast<float>(position.X());
        m_file << static_cast<float>(position.Y());
        m_file << static_cast<float>(position.Z());

        m_file << static_cast<float>(momentum.Px());
        m_file << static_cast<float>(momentum.Py());
        m_file << static_cast<float>(momentum.Pz());

        m_file << static_cast<float>(particle->getPValue());
        m_file << static_cast<int>(Variable::particleMCErrors(particle));
        m_file << static_cast<int>(Variable::isSignalAcceptMissingNeutrino(particle));

      }

      m_file << static_cast<int>(roe->getNTracks());

      for (const auto& track : roe->getTracks()) {
        if (track == nullptr) {
          B2WARNING("Encountered nullptr as track in ROE.");
          m_file << static_cast<int>(0);
          continue;
        }
        PIDLikelihood* pid = track->getRelated<PIDLikelihood>();
        if (pid == nullptr) {
          B2WARNING("Encountered nullptr as pid in ROE.");
          m_file << static_cast<int>(0);
          continue;
        }

        const Const::ChargedStable mostLikely = track->getRelated<PIDLikelihood>()->getMostLikely();
        const TrackFitResult* trackFitResult = track->getTrackFitResult(mostLikely);
        if (trackFitResult == nullptr) {
          B2WARNING("Encountered nullptr as trackFitResult in ROE.");
          m_file << static_cast<int>(0);
          continue;
        }
        m_file << static_cast<int>(1);

        TLorentzVector momentum = PCmsLabTransform::labToCms(trackFitResult->get4Momentum());
        TVector3 position = trackFitResult->getPosition();

        m_file << static_cast<float>(position.X());
        m_file << static_cast<float>(position.Y());
        m_file << static_cast<float>(position.Z());

        m_file << static_cast<float>(momentum.Px());
        m_file << static_cast<float>(momentum.Py());
        m_file << static_cast<float>(momentum.Pz());

        m_file << static_cast<float>(pid->getProbability(Const::electron, Const::pion));
        m_file << static_cast<float>(pid->getProbability(Const::muon, Const::pion));
        m_file << static_cast<float>(pid->getProbability(Const::kaon, Const::pion));
        m_file << static_cast<float>(pid->getProbability(Const::proton, Const::pion));

      }

      m_file << static_cast<int>(roe->getNECLClusters());

      for (const auto& ecl_cluster : roe->getECLClusters()) {
        if (ecl_cluster == nullptr) {
          B2WARNING("Encountered nullptr as ecl_cluster in ROE.");
          m_file << static_cast<int>(0);
          continue;
        }
        m_file << static_cast<int>(1);

        TLorentzVector momentum = PCmsLabTransform::labToCms(ecl_cluster->get4Vector());
        TVector3 position = ecl_cluster->getclusterPosition();
        m_file << static_cast<float>(position.X());
        m_file << static_cast<float>(position.Y());
        m_file << static_cast<float>(position.Z());

        m_file << static_cast<float>(momentum.E());
        m_file << static_cast<float>(ecl_cluster->getTiming());
        m_file << static_cast<float>(ecl_cluster->getE9oE25());
        m_file << static_cast<float>(ecl_cluster->getLAT());

        m_file << static_cast<bool>(ecl_cluster->isNeutral());

      }

      m_file << static_cast<int>(roe->getNKLMClusters());

      for (const auto& klm_cluster : roe->getKLMClusters()) {
        if (klm_cluster == nullptr) {
          B2WARNING("Encountered nullptr as klm_cluster in ROE.");
          m_file << static_cast<int>(0);
          continue;
        }
        m_file << static_cast<int>(1);

        TLorentzVector momentum = PCmsLabTransform::labToCms(klm_cluster->getMomentum());
        TVector3 position = klm_cluster->getClusterPosition();
        m_file << static_cast<float>(position.X());

        m_file << static_cast<float>(position.X());
        m_file << static_cast<float>(position.Y());
        m_file << static_cast<float>(position.Z());

        m_file << static_cast<float>(momentum.E());
        m_file << static_cast<float>(klm_cluster->getTime());
        m_file << static_cast<float>(klm_cluster->getLayers());

        m_file << static_cast<bool>(klm_cluster->getAssociatedEclClusterFlag());
        m_file << static_cast<bool>(klm_cluster->getAssociatedTrackFlag());

      }
    } else {
      B2ERROR("Encountered invalid RestOfEvent. This module has to run inside an foreach loop over RestOfEvents!")
    }

  }

} // Belle2 namespace

