/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <analysis/modules/MVClassifier/CustomTrainingDataModule.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <analysis/dataobjects/EventExtraInfo.h>
#include <analysis/dataobjects/RestOfEvent.h>

#include <analysis/VariableManager/Variables.h>
#include <analysis/VariableManager/FlavorTaggingVariables.h>
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


#include <cstdint>

namespace Belle2 {


  REG_MODULE(CustomTrainingData)

  CustomTrainingDataModule::CustomTrainingDataModule() : Module()
  {
    setDescription("Writes out information about the current ROE object in the datastore which can than later be used in a deep neural network to train the deepFEI algorithm.");
    addParam("filename", m_filename, "Name of output file.");
  }

  void CustomTrainingDataModule::initialize()
  {

    m_file.open(m_filename, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
    if (not m_file.is_open() or not m_file.good()) {
      B2FATAL("Couldn't open file " << m_filename);
    }

  }

  void CustomTrainingDataModule::terminate()
  {
    m_file.close();
  }

  template<class T>
  void binaryWrite(std::ofstream& stream, T x)
  {
    stream.write(reinterpret_cast<char*>(&x), sizeof(T));
  }

  void CustomTrainingDataModule::event()
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

        binaryWrite<float>(m_file, position.X());
        binaryWrite<float>(m_file, position.Y());
        binaryWrite<float>(m_file, position.Z());

        binaryWrite<float>(m_file, momentum.Px());
        binaryWrite<float>(m_file, momentum.Py());
        binaryWrite<float>(m_file, momentum.Pz());

        binaryWrite<float>(m_file, particle->getPValue());
        binaryWrite<int32_t>(m_file, Variable::particleMCErrors(particle));
        binaryWrite<int32_t>(m_file, Variable::isSignalAcceptMissingNeutrino(particle));
        binaryWrite<int32_t>(m_file, Variable::isRestOfEventB0Flavor(particle));

        binaryWrite<int32_t>(m_file, roe->getNTracks());
        binaryWrite<int32_t>(m_file, roe->getNECLClusters());
        binaryWrite<int32_t>(m_file, roe->getNKLMClusters());

      }

      for (const auto& track : roe->getTracks()) {
        if (track == nullptr) {
          B2WARNING("Encountered nullptr as track in ROE.");
          binaryWrite<int32_t>(m_file, 0);
          continue;
        }
        PIDLikelihood* pid = track->getRelated<PIDLikelihood>();
        if (pid == nullptr) {
          B2WARNING("Encountered nullptr as pid in ROE.");
          binaryWrite<int32_t>(m_file, 0);
          continue;
        }

        const Const::ChargedStable mostLikely = track->getRelated<PIDLikelihood>()->getMostLikely();
        const TrackFitResult* trackFitResult = track->getTrackFitResult(mostLikely);
        if (trackFitResult == nullptr) {
          B2WARNING("Encountered nullptr as trackFitResult in ROE.");
          binaryWrite<int32_t>(m_file, 0);
          continue;
        }

        TLorentzVector momentum = PCmsLabTransform::labToCms(trackFitResult->get4Momentum());
        TVector3 position = trackFitResult->getPosition();

        if (not std::isfinite(momentum.Px()) or not std::isfinite(momentum.Py()) or not std::isfinite(momentum.Pz())) {
          B2WARNING("Encountered nan value in momentum.");
          binaryWrite<int32_t>(m_file, 0);
          continue;
        }

        if (not std::isfinite(position.X()) or not std::isfinite(position.Y()) or not std::isfinite(position.Z())) {
          B2WARNING("Encountered nan value in position.");
          binaryWrite<int32_t>(m_file, 0);
          continue;
        }

        binaryWrite<int32_t>(m_file, 1);

        binaryWrite<int32_t>(m_file, trackFitResult->getChargeSign());

        binaryWrite<float>(m_file, position.X());
        binaryWrite<float>(m_file, position.Y());
        binaryWrite<float>(m_file, position.Z());

        binaryWrite<float>(m_file, momentum.Px());
        binaryWrite<float>(m_file, momentum.Py());
        binaryWrite<float>(m_file, momentum.Pz());

        binaryWrite<float>(m_file, trackFitResult->getPValue());

        binaryWrite<float>(m_file, pid->getProbability(Const::electron, Const::pion));
        binaryWrite<float>(m_file, pid->getProbability(Const::muon, Const::pion));
        binaryWrite<float>(m_file, pid->getProbability(Const::kaon, Const::pion));
        binaryWrite<float>(m_file, pid->getProbability(Const::proton, Const::pion));

      }

      for (const auto& ecl_cluster : roe->getECLClusters()) {
        if (ecl_cluster == nullptr) {
          B2WARNING("Encountered nullptr as ecl_cluster in ROE.");
          binaryWrite<int32_t>(m_file, 0);
          continue;
        }

        TLorentzVector momentum = PCmsLabTransform::labToCms(ecl_cluster->get4Vector());
        TVector3 position = ecl_cluster->getclusterPosition();

        if (not std::isfinite(momentum.Px()) or not std::isfinite(momentum.Py()) or not std::isfinite(momentum.Pz())) {
          B2WARNING("Encountered nan value in momentum.");
          binaryWrite<int32_t>(m_file, 0);
          continue;
        }

        if (not std::isfinite(position.X()) or not std::isfinite(position.Y()) or not std::isfinite(position.Z())) {
          B2WARNING("Encountered nan value in position.");
          binaryWrite<int32_t>(m_file, 0);
          continue;
        }

        binaryWrite<int32_t>(m_file, 1);
        binaryWrite<float>(m_file, position.X());
        binaryWrite<float>(m_file, position.Y());
        binaryWrite<float>(m_file, position.Z());

        binaryWrite<float>(m_file, momentum.E());
        binaryWrite<float>(m_file, ecl_cluster->getTiming());
        binaryWrite<float>(m_file, ecl_cluster->getE9oE25());
        binaryWrite<float>(m_file, ecl_cluster->getLAT());

        binaryWrite<bool>(m_file, ecl_cluster->isNeutral());

      }

      for (const auto& klm_cluster : roe->getKLMClusters()) {
        if (klm_cluster == nullptr) {
          B2WARNING("Encountered nullptr as klm_cluster in ROE.");
          binaryWrite<int32_t>(m_file, 0);
          continue;
        }

        TLorentzVector momentum = PCmsLabTransform::labToCms(klm_cluster->getMomentum());
        TVector3 position = klm_cluster->getClusterPosition();

        if (not std::isfinite(momentum.Px()) or not std::isfinite(momentum.Py()) or not std::isfinite(momentum.Pz())) {
          B2WARNING("Encountered nan value in momentum.");
          binaryWrite<int32_t>(m_file, 0);
          continue;
        }

        if (not std::isfinite(position.X()) or not std::isfinite(position.Y()) or not std::isfinite(position.Z())) {
          B2WARNING("Encountered nan value in position.");
          binaryWrite<int32_t>(m_file, 0);
          continue;
        }

        binaryWrite<int32_t>(m_file, 1);

        binaryWrite<float>(m_file, position.X());
        binaryWrite<float>(m_file, position.Y());
        binaryWrite<float>(m_file, position.Z());

        binaryWrite<float>(m_file, momentum.E());
        binaryWrite<float>(m_file, klm_cluster->getTime());
        binaryWrite<float>(m_file, klm_cluster->getLayers());

        binaryWrite<bool>(m_file, klm_cluster->getAssociatedEclClusterFlag());
        binaryWrite<bool>(m_file, klm_cluster->getAssociatedTrackFlag());

      }
    } else {
      B2ERROR("Encountered invalid RestOfEvent. This module has to run inside an foreach loop over RestOfEvents!")
    }

  }

} // Belle2 namespace

